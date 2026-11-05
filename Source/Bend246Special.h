// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Bend246.h"
namespace pocket::reference {
inline int16_t bend246VoiceCoefficient(int16_t previous,int level,int16_t amount) noexcept {
 const auto delta=multiplyQ15(saturate16(level*128),amount);
 return saturate16(int(multiplyQ15(saturate16(int(delta)+16384),4096))+multiplyQ15(previous,28672));
}
inline int16_t bend246PitchCoefficient(int level,int16_t amount) noexcept {return multiplyQ15(saturate16(level*(amount<0?32:256)),amount);}
inline uint32_t bend246VoicePitch(uint32_t input,int16_t coefficient) noexcept {
 const auto product=multiplyQ31(signedPhase(input),int32_t(coefficient)*65536);auto delta=saturate32(int64_t(product)*2);
 for(int n=0;n<5;++n)delta=saturate32(int64_t(delta)+product);
 return uint32_t(saturate32(int64_t(signedPhase(input))+delta));
}
inline bool bend246VoiceStep(Bend246State& state,std::array<int16_t,8>& c,int family,int kind,std::array<uint32_t,128>& pitch,std::array<int16_t,128>& gain) noexcept {
 bend246Advance(state,c);if(family!=0||kind!=0||floorDivide(c[1],2048)!=3)return false;
 const int target=int(floorDivide(c[0],4096));const auto level=int(int16_t(state.words[0]));
 if(target==0){const auto coefficient=bend246VoiceCoefficient(int16_t(state.display&65535u),level,c[2]);state.display=(state.display&0xffff0000u)|uint16_t(coefficient);for(auto& v:gain)v=midi246VoiceGain(v,coefficient);return true;}
 if(target==1){const auto coefficient=bend246PitchCoefficient(level,c[2]);for(auto& v:pitch)v=bend246VoicePitch(v,coefficient);return true;}return false;
}
}
