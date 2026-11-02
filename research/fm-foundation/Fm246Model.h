// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace pocket::reference {
// Twenty recovered binary32 words; ten authored substitutions, explicitly indexed.
inline constexpr std::array<uint32_t,30> fm246ModelBits{{0x3ba3d70au,0x3b9ba5e3u,0x3baf1308u,0x3bcbd954u,0x3b90fdbbu,0x3bad4e50u,0x3c000000u,0x3c800000u,0x3d000000u,0x3da00000u,0x3e000000u,0x3e200000u,0x3e400000u,0x3e800000u,0x3f000000u,0x3ec00000u,0x3e800000u,0x3e000000u,0x3dc00000u,0x3d800000u,0x3d200000u,0x3c000000u,0x3c800000u,0x3de00000u,0x3e600000u,0x3ea80000u,0x3ee00000u,0x3f000000u,0x3f600000u,0x3f800000u}};
inline constexpr std::array<int,10> fm246AuthoredIndices{{0,1,6,7,12,13,18,19,24,25}};
}
