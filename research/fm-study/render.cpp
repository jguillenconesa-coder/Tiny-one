// SPDX-License-Identifier: AGPL-3.0-only
// Diagnostic render of the shipped authored FM, not a firmware audio oracle.
#include "Engine.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>

int main(int argc, char** argv) {
    if (argc != 6) {
        std::cerr << "usage: render sample-rate midi-note tone color output.csv\n";
        return 2;
    }
    try {
        const double rate = std::stod(argv[1]);
        const int note = std::stoi(argv[2]);
        const float tone = std::stof(argv[3]), color = std::stof(argv[4]);
        if (!(rate >= 8000 && rate <= 192000) || note < 0 || note > 127 ||
            !(tone >= 0 && tone <= 1) || !(color >= 0 && color <= 1))
            throw std::runtime_error("argument outside diagnostic range");
        auto engine = std::make_unique<pocket::Engine>();
        engine->prepare(rate);
        pocket::Parameters p;
        p.engine = 0; p.digital246 = false;
        p.tone = tone; p.color = color; p.envelope = .5f;
        p.attack = 0; p.sustain = 1; p.effect = 0; p.drive = 0;
        p.volume = .7f; p.lfoDepth = 0;
        // Settle controls in silence, then render a held note after its attack.
        for (int i = 0; i < int(rate); ++i) engine->next(p);
        engine->noteOn(note, 1);
        for (int i = 0; i < int(rate / 8); ++i) engine->next(p);
        std::ofstream out(argv[5]);
        out.exceptions(std::ios::badbit | std::ios::failbit);
        out << "sample,value\n" << std::setprecision(9);
        for (int i = 0; i < int(rate / 4); ++i) {
            const auto value = engine->next(p);
            if (!std::isfinite(value)) throw std::runtime_error("non-finite render");
            out << i << ',' << value << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
