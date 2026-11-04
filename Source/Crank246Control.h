// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Crank246.h"
namespace pocket {
inline constexpr std::array<const char*,4> crank246Ids{{"crank246Angle","crank246Amount","crank246Destination","crank246Colour"}};
// Authored host tick and activation policy. One inertial source feeds every
// voice/FX; notes do not reset it. Controls, not its live trajectory, persist.
struct Crank246Control {
 reference::Crank246State state;
 std::array<int16_t,8> controls{};
 double position=1;
 bool active=false;
 int colour=0,destination=0;
 void next(bool enabled,int angle,int amount,int dest,int target,double step) noexcept {
  if(!enabled){if(active)*this={};return;}
  if(!active){*this={};active=true;}
  colour=std::clamp(target,0,7);destination=std::clamp(dest,0,2);
  controls[0]=int16_t(std::clamp(angle,-8192,8191));controls[1]=int16_t(std::clamp(amount,-32767,32767));controls[2]=int16_t(destination*2048);
  if(position>=1){reference::crank246Prepare(state);reference::crank246Advance(state,controls);position-=1;}
  position+=step;
 }
 float phase() const noexcept {return active?float(state.words[2])/256.f:0;}
 template<size_t N> std::array<int16_t,N> apply(const std::array<int16_t,N>& base,int kind,int type) const noexcept {
  static_assert(N==4||N==8);
  if(!active||colour>=(destination==0&&reference::midi246Extended(type)?8:4))return base;
  auto c=controls;c[3]=int16_t(colour*(destination==0&&reference::midi246Extended(type)?2048:4096));
  std::array<int16_t,8> targets{};std::copy(base.begin(),base.end(),targets.begin());
  reference::crank246Apply(state,c,kind==2?2:(type==8||type==9)?1:0,kind,type,targets);
  std::array<int16_t,N> result{};std::copy_n(targets.begin(),N,result.begin());return result;
 }
};
}
