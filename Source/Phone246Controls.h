// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace pocket {
inline constexpr std::array<const char*,4> phone246Ids{{"phone246Blue","phone246Green","phone246White","phone246Orange"}};
inline constexpr std::array<int,4> phone246Min{{204,3072,1536,0}},phone246Max{{20480,17408,16896,32767}};
inline constexpr std::array<int16_t,4> phone246Defaults{{12000,8192,6144,20000}};
}
