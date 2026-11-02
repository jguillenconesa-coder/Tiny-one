// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace pocket {
inline constexpr std::array<const char*,4> spring246Ids{{"spring246Blue","spring246Green","spring246White","spring246Orange"}};
inline constexpr std::array<int,4> spring246Min{{1344,7744,0,0}},spring246Max{{16448,16448,16384,32767}};
inline constexpr std::array<int16_t,4> spring246Defaults{{9651,12966,8192,13000}};
}
