// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace pocket {
inline constexpr std::array<const char*,4> filter246Ids{{"filter246Blue","filter246Green","filter246White","filter246Orange"}};
inline constexpr std::array<int,4> filter246Min{{64,0,0,8192}},filter246Max{{14528,24576,22528,32767}};
inline constexpr std::array<int16_t,4> filter246Defaults{{8000,0,0,24576}};
}
