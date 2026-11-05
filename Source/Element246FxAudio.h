// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Element246.h"
namespace pocket::reference {
inline bool element246FxAudioRoute(const std::array<int16_t,8>& c,int kind) noexcept {
 const int colour=int(floorDivide(c[3],4096));return floorDivide(c[2],2048)==3&&(kind==2||kind==5)&&(colour==2||colour==3);
}
// Original FX arithmetic differs from parameter modulation: raw sensor sign,
// sample-wise external input and self-product for source 3 (not its level word).
inline int16_t element246FxSample(int16_t input,int16_t external,const Element246Inputs& in,int source,int16_t amount,int colour,int kind=2) noexcept {
 int16_t product;
 if(source==2){const auto factor=kind==5?multiplyQ15(in.drumEnvelope,amount):saturate16(int(multiplyQ15(in.synthEnvelope,amount))*4);product=multiplyQ15(input,factor);}
 else product=multiplyQ15(multiplyQ15(input,source==1?external:source==3?input:in.sensor),amount);
 return colour==3?saturate16(int(input)-product):product;
}
// Return word is separate from processing: source 3 mutates audio but returns 0.
inline uint32_t element246FxAudioBlock(const Element246Inputs& in,const std::array<int16_t,8>& c,int kind,const std::array<int16_t,64>& external,std::array<int16_t,128>& audio) noexcept {
 if(!element246FxAudioRoute(c,kind))return 0;
 const int source=int(floorDivide(c[0],2048)),colour=int(floorDivide(c[3],4096));
 for(size_t i=0;i<64;++i)audio[i]=element246FxSample(audio[i],external[i],in,source,c[1],colour,kind);
 return source==3?0u:1u;
}
}
namespace pocket {
// Authored snapshot; source 2 uses the host's quarter-scale ADSR mapping and
// kind 2. The mono bus independently resamples the external sidechain.
struct Element246FxAudio {
 reference::Element246Inputs inputs{};int source=0;int16_t amount=0;int colour=-1;
 int16_t apply(int16_t sample,int16_t external) const noexcept {return colour==2||colour==3?reference::element246FxSample(sample,external,inputs,source,amount,colour):sample;}
};
}
