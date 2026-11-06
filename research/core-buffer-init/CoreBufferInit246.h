// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
namespace pocket::research {
// Observed windows only. No active producer or whole-machine reset contract.
struct CoreBufferInit246 {
    static void apply(std::array<uint8_t,24>& defaults,std::array<uint8_t,1552>& buffers) noexcept {
        constexpr std::array<uint8_t,6> values{{0x00,0xd7,0xa3,0x00,0x00,0x40}};
        std::copy(values.begin(),values.end(),defaults.begin()+8);
        std::fill(buffers.begin()+8,buffers.begin()+1544,uint8_t(0));
    }
};
}
