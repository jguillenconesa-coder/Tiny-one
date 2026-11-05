// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Element246.h"
namespace pocket::reference {
inline int16_t element246GainWord(int16_t gain,int16_t external,const Element246Inputs& inputs,int source,int16_t amount) noexcept {
 const auto delta=multiplyQ15(source==1?external:source==2?gain:source==3?inputs.level:saturate16(-int(inputs.sensor)),amount);
 return saturate16(int(gain)+delta);
}
inline bool element246GainRoute(const std::array<int16_t,8>& c,int family,int kind) noexcept {return family==0&&kind==0&&floorDivide(c[2],2048)==3&&floorDivide(c[3],4096)==0;}
inline uint32_t element246GainBlock(const Element246Inputs& inputs,const std::array<int16_t,8>& c,int family,int kind,const std::array<int16_t,64>& external,std::array<int16_t,128>& gain) noexcept {
 if(!element246GainRoute(c,family,kind))return 0;const int source=int(floorDivide(c[0],2048));
 for(size_t i=0;i<128;++i)gain[i]=element246GainWord(gain[i],external[i/2],inputs,source,c[1]);
 return source==3?0u:1u;
}
// Scalar offset, self amount or external amount for the authored host capture.
inline int16_t element246GainScalar(const Element246Inputs& inputs,int source,int16_t amount) noexcept {return source==1||source==2?amount:multiplyQ15(source==3?inputs.level:saturate16(-int(inputs.sensor)),amount);}
}
