// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Element246.h"
#include <cmath>
namespace pocket {
inline constexpr std::array<const char*,4> element246Ids{{"element246Source","element246Amount","element246Destination","element246Colour"}};
// Authored frontend: one global 44100/64 window, virtual sensor, positive input
// peak, max voice ADSR and absolute pre-FX instrument peak. No original physical
// sensor, IPC producer, per-voice input/routing or level detector is implied.
struct Element246Control {
 reference::Element246Inputs inputs;
 double position=0;
 float externalPeak=0,instrumentPeak=0;
 bool active=false,drum=false;
 int source=0,amount=16000,destination=0,colour=0;
 static float finite(float v) noexcept {return std::isfinite(v)?v:0.f;}
 static int16_t word(float v) noexcept {return int16_t(std::lround(std::clamp(finite(v),-1.f,1.f)*32767.f));}
 bool willTick(double step) const noexcept {return position+step>=1;}
 void next(bool enabled,int selected,int depth,int dest,int target,int sensor,float external,float envelope,float instrument,double step,bool isDrum=false) noexcept {
  if(!enabled){if(active)*this={};return;}
  if(!active){*this={};active=true;}
  source=std::clamp(selected,0,3);amount=std::clamp(depth,-32767,32767);destination=std::clamp(dest,0,2);colour=std::clamp(target,0,7);drum=isDrum;
  externalPeak=std::max(externalPeak,finite(external));instrumentPeak=std::max(instrumentPeak,std::abs(finite(instrument)));
  position+=step;
  if(position>=1){
   inputs.sensor=int16_t(std::clamp(sensor,-32768,32767));inputs.externalPeak=word(externalPeak);
   // Quarter scale compensates the original synth/FX x4 route; drum's explicit
   // scalar route has no x4. All three are authored mappings of max host ADSR.
   const auto env=word(std::clamp(finite(envelope),0.f,1.f));inputs.synthGainPeak=inputs.synthEnvelope=int16_t(env/4);inputs.drumEnvelope=env;inputs.level=word(instrumentPeak);
   externalPeak=instrumentPeak=0;position-=1;
  }
 }
 float level() const noexcept {return active?reference::element246Signal(inputs,source,int16_t(amount),destination==2?2:drum?1:0,destination==2?2:0)/32768.f:0;}
 template<size_t N> std::array<int16_t,N> apply(const std::array<int16_t,N>& base,int kind,int type) const noexcept {
  static_assert(N==4||N==8);
  if(!active||colour>=(destination==0&&reference::midi246Extended(type)?8:4))return base;
  const bool extended=destination==0&&reference::midi246Extended(type);
  std::array<int16_t,8> c{{int16_t(source*2048),int16_t(amount),int16_t(destination*2048),int16_t(colour*(extended?2048:4096)),0,0,0,0}},targets{};
  std::copy(base.begin(),base.end(),targets.begin());reference::element246Apply(inputs,c,kind==2?2:(type==8||type==9)?1:0,kind,type,targets);
  std::array<int16_t,N> result{};std::copy_n(targets.begin(),N,result.begin());return result;
 }
};
}
