// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace pocket {
inline constexpr std::array<const char*,4> cwo246Ids{{"cwo246Blue","cwo246Green","cwo246White","cwo246Orange"}};
inline constexpr std::array<int,4> cwo246Min{{0,0,0,0}},cwo246Max{{32767,32767,32767,32767}};
inline constexpr std::array<int16_t,4> cwo246Defaults{{16384,8192,8192,16384}};
}
