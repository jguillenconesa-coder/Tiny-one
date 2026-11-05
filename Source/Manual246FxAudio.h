// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Bend246.h"
#include "Crank246.h"
namespace pocket::reference {
// Independently reconstructed FX input transforms. XOR acts on the signed
// halfword's bits before Q15 multiplication; both branches add to the dry word.
inline int16_t manual246FxSample(int16_t input,int16_t source,int16_t amount,int colour) noexcept {
 if(colour==2)return saturate16(int(input)+multiplyQ15(input,multiplyQ15(source,amount)));
 if(colour==3){const auto bits=uint16_t(uint16_t(input)^uint16_t(source));const auto shaped=int16_t(bits<32768?int(bits):int(bits)-65536);return saturate16(int(input)+multiplyQ15(multiplyQ15(input,shaped),amount));}
 return input;
}
inline bool manual246FxBlock(int16_t source,int16_t amount,int destination,int colour,int kind,std::array<int16_t,128>& audio) noexcept {
 if(destination!=3||(kind!=2&&kind!=5)||(colour!=2&&colour!=3))return false;
 for(size_t i=0;i<64;++i)audio[i]=manual246FxSample(audio[i],source,amount,colour);
 return true;
}
inline bool bend246FxAudioStep(Bend246State& state,std::array<int16_t,8>& controls,int kind,std::array<int16_t,128>& audio) noexcept {
 bend246Advance(state,controls);return manual246FxBlock(saturate16(int(int16_t(state.words[0]))*256),controls[2],int(floorDivide(controls[1],2048)),int(floorDivide(controls[0],4096)),kind,audio);
}
inline bool crank246FxAudioStep(Crank246State& state,std::array<int16_t,8>& controls,int kind,std::array<int16_t,128>& audio) noexcept {
 const auto level=crank246Advance(state,controls);return manual246FxBlock(saturate16((level-64)*512),controls[1],int(floorDivide(controls[2],2048)),int(floorDivide(controls[3],4096)),kind,audio);
}
}
namespace pocket {
// Authored host snapshot. Shared source advances in Engine at its existing tick;
// this adapter samples it at the mono FX input and never advances the source.
struct Manual246FxAudio {
 int16_t source=0,amount=0;int colour=-1;
 int16_t apply(int16_t sample) const noexcept {return reference::manual246FxSample(sample,source,amount,colour);}
};
}
