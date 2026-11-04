// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Midi246.h"
namespace pocket::reference {
// Explicit observed inputs, not implementations of their physical producers.
// Positive peaks include zero; they are not absolute-value envelope followers.
struct Element246Inputs {
 int16_t sensor=0,externalPeak=0,synthGainPeak=0,synthEnvelope=0,level=0,drumEnvelope=0;
};
template<size_t N> inline int16_t element246PositivePeak(const std::array<int16_t,N>& samples) noexcept {
 int16_t peak=0;for(auto v:samples)peak=std::max(peak,v);return peak;
}
inline int16_t element246Signal(const Element246Inputs& in,int source,int16_t amount,int family,int kind) noexcept {
 if(source==1)return multiplyQ15(in.externalPeak,amount);
 if(source==2){
  if(family==1||(family==2&&kind==5))return multiplyQ15(in.drumEnvelope,amount);
  if(family==2)return saturate16(int(multiplyQ15(in.synthEnvelope,amount))*4);
  return multiplyQ15(saturate16(int(in.synthGainPeak)*4),amount);
 }
 return multiplyQ15(source==3?in.level:saturate16(-int(in.sensor)),amount);
}
// Bounded Instrument/first-four ADSR/FX parameter dispatch. No special
// audio/gain/pitch routes, auxiliary FX ENV or physical input producer implied.
inline bool element246Apply(const Element246Inputs& in,const std::array<int16_t,8>& c,int family,int kind,int type,std::array<int16_t,8>& targets) noexcept {
 const int destination=int(floorDivide(c[2],2048));const auto mod=element246Signal(in,int(floorDivide(c[0],2048)),c[1],family,kind);
 if(family==0&&destination==1&&kind==1){const auto i=size_t(uint16_t(c[3])>>12);if(i>=4)return false;targets[i]=value246Adjust(targets[i],mod,valueVoice246EnvelopeRanges[i]);return true;}
 if(family==2&&destination==2&&(kind==2||kind==5)&&type>=0&&type<8){const auto i=size_t(uint16_t(c[3])>>12);if(i>=8)return false;const auto& range=value246FxRanges[size_t(type)][i];targets[i]=std::clamp(saturate16(int(targets[i])+mod),range[0],range[1]);return true;}
 if(destination!=0||(family==0&&kind!=0)||family==2)return false;
 const auto colour=size_t(uint16_t(c[3])>>(midi246Extended(type)?11:12));if(colour>=8)return false;
 if(family==1){if(type!=8&&type!=9)return false;drum246Modulate(targets,type,colour,mod);return true;}
 const auto index=valueVoice246RangeIndex(type);if(index<0)return false;
 if(type==6)sampler246Modulate(targets,colour,mod);else {const auto i=type==12?colour:colour%4;targets[i]=valueVoice246Adjust(targets[i],type==12||colour<4?mod:int16_t(floorDivide(mod,16)),valueVoice246Ranges[size_t(index)][i]);}return true;
}
}
