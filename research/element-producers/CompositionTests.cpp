// SPDX-License-Identifier: AGPL-3.0-only
#include "EnvelopePublisher246.h"
#include "Element246.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <tuple>
#include <vector>

static void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
static std::vector<unsigned char> read(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    check(bool(stream), "Missing composition fixture");
    return {std::istreambuf_iterator<char>(stream), {}};
}
static uint32_t word(const unsigned char* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
           (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
static int16_t half(const unsigned char* p) {
    const int value = p[0] | (unsigned(p[1]) << 8);
    return int16_t(value < 32768 ? value : value - 65536);
}

int main(int argc, char** argv) {
    try {
        using namespace pocket::reference;
        check(argc == 2, "Provide unpacked composition fixture directory");
        const auto input = read(std::filesystem::path(argv[1]) / "envelope-publisher246-element-input.bin");
        const auto output = read(std::filesystem::path(argv[1]) / "envelope-publisher246-element-output.bin");
        check(input.size() == 4096 * 328 && output.size() == 4096 * 704, "Incomplete composition fixtures");
        std::array<EnvelopePublisher246State, 4> states{};
        int32_t published = 0;
        size_t changed = 0, acceptedCalls = 0;
        std::map<std::tuple<int, int, int, int>, size_t> coverage;
        for (size_t n = 0; n < 4096; ++n) {
            const auto* x = input.data() + n * 328;
            const auto* expected = output.data() + n * 704;
            const auto action = word(x);
            auto& state = states[(action >> 2) & 3];
            if (action & 1) {
                state.first = signedPhase(word(x + 4));
                state.second = signedPhase(word(x + 8));
                published = signedPhase(word(x + 12));
            }
            if (action & 2) EnvelopePublisher246::resetPublished(published);
            std::array<int16_t, 4> publisherControls{};
            std::array<int16_t, 128> audio{};
            std::array<int16_t, 8> controls{}, initialTargets{};
            for (size_t i = 0; i < 4; ++i) publisherControls[i] = half(x + 16 + i * 2);
            for (size_t i = 0; i < 128; ++i) audio[i] = half(x + 24 + i * 2);
            for (size_t i = 0; i < 8; ++i) {
                controls[i] = half(x + 296 + i * 2);
                initialTargets[i] = half(x + 312 + i * 2);
            }
            const int family = int(word(x + 280)), type = int(word(x + 284)), kind = int(word(x + 288));
            const int destination = int(floorDivide(controls[2], 2048));
            std::array<unsigned char, 704> actual{};
            auto consume = [&](size_t offset) {
                auto targets = initialTargets;
                Element246Inputs sources;
                sources.drumEnvelope = EnvelopePublisher246::readPublished(published);
                const uint32_t accepted = element246Apply(sources, controls, family, kind, type, targets);
                std::memcpy(actual.data() + offset, x + 292, 4);
                std::memcpy(actual.data() + offset + 4, controls.data(), 16);
                std::memcpy(actual.data() + offset + 20, targets.data(), 16);
                std::memcpy(actual.data() + offset + 36, &accepted, 4);
                if (accepted) { ++coverage[{family, type, kind, destination}]; ++acceptedCalls; }
                return targets;
            };
            auto snapshot = [&](size_t offset) {
                static_assert(sizeof(states) == 32);
                std::memcpy(actual.data() + offset, states.data(), 32);
                std::memcpy(actual.data() + offset + 32, &published, 4);
                const uint32_t getter = 0xa5b60000u | uint16_t(EnvelopePublisher246::readPublished(published));
                std::memcpy(actual.data() + offset + 36, &getter, 4);
                std::memcpy(actual.data() + offset + 40, publisherControls.data(), 8);
                std::memcpy(actual.data() + offset + 48, audio.data(), 256);
                const uint32_t guard = 0x2468ace0u;
                std::memcpy(actual.data() + offset + 304, &guard, 4);
                std::memcpy(actual.data() + offset + 308, &action, 4);
            };
            const auto before = consume(0);
            EnvelopePublisher246::process(state, published, publisherControls, audio);
            snapshot(40);
            const auto after = consume(352);
            snapshot(392);
            if (before != after) ++changed;
            if (std::memcmp(actual.data(), expected, actual.size())) {
                for (size_t i = 0; i < actual.size(); ++i) if (actual[i] != expected[i]) {
                    std::cerr << "Composition block " << n << " family " << family << " type " << type
                              << " kind " << kind << " destination " << destination << " byte " << i
                              << " expected " << int(expected[i]) << " got " << int(actual[i]) << '\n';
                    break;
                }
                throw std::runtime_error("Original composed publisher/Element mismatch");
            }
        }
        for (int type : {8, 9}) for (int kind = 0; kind < 4; ++kind)
            check(coverage[{1, type, kind, 0}] > 0, "Missing accepted drum dispatch");
        for (int type = 0; type < 8; ++type)
            check(coverage[{2, type, 5, 2}] > 0, "Missing accepted FX dispatch");
        check(changed > 0, "Before/after scheduling never changed a target");
        std::cout << "PASS: 4096 original publisher blocks / 524288 samples, 8192 composed original Element calls; "
                  << acceptedCalls << " accepted dispatches, " << changed
                  << " blocks with different before/after targets; complete publisher state/audio preserved by consumers\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
