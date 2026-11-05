// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Crank246.h"
#include "Bend246Special.h"
namespace pocket::reference {
inline int16_t crank246VoiceCoefficient(int16_t previous,int level,int16_t amount) noexcept {
 const auto delta=multiplyQ15(saturate16((level-64)*256),amount);
 return saturate16(int(multiplyQ15(saturate16(int(delta)+16384),4096))+multiplyQ15(previous,28672));
}
inline int16_t crank246PitchCoefficient(int level,int16_t amount) noexcept {
 const int centred=level-64;return multiplyQ15(saturate16(centred*(centred<0?64:512)),amount);
}
// Same recovered signed-word multiply/seven saturating adds as Bend, with the
// independently verified Crank coefficient above. No Bend source curve is used.
inline bool crank246VoiceStep(Crank246State& state,uint32_t& gainState,std::array<int16_t,8>& c,int family,int kind,std::array<uint32_t,128>& pitch,std::array<int16_t,128>& gain) noexcept {
 const auto level=crank246Advance(state,c);if(family!=0||kind!=0||floorDivide(c[2],2048)!=3)return false;
 const int target=int(floorDivide(c[3],4096));
 if(target==0){const auto coefficient=crank246VoiceCoefficient(int16_t(gainState&65535u),level,c[1]);gainState=(gainState&0xffff0000u)|uint16_t(coefficient);for(auto& v:gain)v=midi246VoiceGain(v,coefficient);return true;}
 if(target==1){const auto coefficient=crank246PitchCoefficient(level,c[1]);for(auto& v:pitch)v=bend246VoicePitch(v,coefficient);return true;}return false;
}
}
