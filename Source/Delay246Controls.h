// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace pocket {
inline constexpr std::array<const char*,4> delay246Ids{{"delay246Blue","delay246Green","delay246White","delay246Orange"}};
inline constexpr std::array<int,4> delay246Min{{1024,3276,0,0}},delay246Max{{11264,32767,16384,32767}};
inline constexpr std::array<int16_t,4> delay246Defaults{{6144,24000,9000,16000}};
}
