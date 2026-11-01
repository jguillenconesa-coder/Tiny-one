// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace pocket {
inline constexpr std::array<const char*,4> grid246Ids{{"grid246Blue","grid246Green","grid246White","grid246Orange"}};
inline constexpr std::array<int,4> grid246Min{{1344,1344,0,0}},grid246Max{{16704,16704,32767,32767}};
inline constexpr std::array<int16_t,4> grid246Defaults{{6144,8192,27000,16000}};
}
