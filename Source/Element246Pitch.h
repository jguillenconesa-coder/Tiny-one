// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Element246.h"
namespace pocket::reference {
inline uint32_t element246PitchRelative(uint32_t input,int16_t coefficient) noexcept {return uint32_t(saturate32(int64_t(signedPhase(input))+multiplyQ31(signedPhase(input),int32_t(coefficient)*65536)));}
inline uint32_t element246PitchEnvelope(uint32_t input,int16_t coefficient) noexcept {return uint32_t(saturate32(int64_t(signedPhase(input))+int32_t(coefficient)*16384));}
inline uint32_t element246PitchExternal(uint32_t input,int16_t external,int16_t amount) noexcept {
 // Original product -> double -> float -> Q31 clips every nonzero integral
 // product to its signed endpoint. This is not a replacement smoothing curve.
 const auto product=int64_t(external)*amount;
 const int32_t delta=product>0?2147483647:product<0?signedPhase(0x80000000u):0;
 return uint32_t(saturate32(int64_t(signedPhase(input))+delta));
}
inline uint32_t element246PitchWord(uint32_t input,int16_t gain,int16_t external,const Element246Inputs& in,int source,int16_t amount) noexcept {
 if(source==1)return element246PitchExternal(input,external,amount);
 if(source==2)return element246PitchEnvelope(input,multiplyQ15(gain,amount));
 return element246PitchRelative(input,multiplyQ15(source==3?in.level:saturate16(-int(in.sensor)),amount));
}
inline bool element246PitchRoute(const std::array<int16_t,8>& c,int family,int kind) noexcept {return family==0&&kind==0&&floorDivide(c[2],2048)==3&&floorDivide(c[3],4096)==1;}
inline uint32_t element246PitchBlock(const Element246Inputs& in,const std::array<int16_t,8>& c,int family,int kind,const std::array<int16_t,64>& external,const std::array<int16_t,128>& gain,std::array<uint32_t,128>& pitch) noexcept {
 if(!element246PitchRoute(c,family,kind))return 0;
 const int source=int(floorDivide(c[0],2048));for(size_t i=0;i<128;++i)pitch[i]=element246PitchWord(pitch[i],gain[i],external[i/2],in,source,c[1]);return source==3?0u:1u;
}
}
