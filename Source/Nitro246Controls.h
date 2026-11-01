// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace pocket {
inline constexpr std::array<const char*,4> nitro246Ids{{"nitro246Blue","nitro246Green","nitro246White","nitro246Orange"}};
inline constexpr std::array<int,4> nitro246Min{{64,-32768,0,64}},nitro246Max{{16448,32767,20643,16448}};
inline constexpr std::array<int16_t,4> nitro246Defaults{{512,0,8000,14000}};
}
