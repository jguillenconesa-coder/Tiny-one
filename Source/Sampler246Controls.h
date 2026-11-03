// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Sampler246.h"
namespace pocket {
inline constexpr std::array<const char*,8> sampler246Ids{{"sampler246Start","sampler246LoopStart","sampler246LoopEnd","sampler246End","sampler246Reverse","sampler246LoopStartFine","sampler246LoopEndFine","sampler246Volume"}};
inline constexpr std::array<const char*,8> sampler246Names{{"Start","Loop In","Loop Out","End","Reverse","Loop In Fine","Loop Out Fine","Volume"}};
inline constexpr std::array<int16_t,8> sampler246Defaults{{0,0,32767,32767,0,0,0,16384}};
inline constexpr std::array<const char*,4> sampler246EnvelopeIds{{"sampler246Attack","sampler246Decay","sampler246Sustain","sampler246Release"}};
inline constexpr std::array<int16_t,4> sampler246EnvelopeDefaults{{64,8064,32767,8192}};
inline double sampler246Seconds(int16_t coarse,int16_t fine=0) noexcept {return reference::sampler246Position(coarse,fine)/(4096.*44100.);}
}
