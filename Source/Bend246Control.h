// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Bend246Special.h"
namespace pocket {
inline constexpr std::array<const char*,4> bend246Ids{{"bend246Colour","bend246Destination","bend246Amount","bend246Angle"}};
// Own host clock and activation/panic policy. One source is shared by voices
// and FX; the original input arithmetic runs only at an explicit host tick.
struct Bend246Control {
 reference::Bend246State state;std::array<int16_t,8> controls{};double position=1;bool active=false;int colour=0,destination=0;
 void next(bool enabled,int target,int dest,int amount,int angle,bool reverse,double step) noexcept {
  if(!enabled){if(active)*this={};return;}if(!active){*this={};active=true;}
  colour=std::clamp(target,0,7);destination=std::clamp(dest,0,3);controls[1]=int16_t(destination*2048);controls[2]=int16_t(std::clamp(amount,-32767,32767));controls[3]=int16_t(std::clamp(angle,0,16383));controls[7]=reverse?2048:0;
  if(position>=1){reference::bend246Prepare(state);reference::bend246Advance(state,controls);if(destination==3&&colour==0){const auto gain=reference::bend246VoiceCoefficient(int16_t(state.display&65535u),int(int16_t(state.words[0])),int16_t(amount));state.display=(state.display&0xffff0000u)|uint16_t(gain);}position-=1;}position+=step;
 }
 float level() const noexcept {return active?float(state.words[0])/127.f:0;}
 template<size_t N> std::array<int16_t,N> apply(const std::array<int16_t,N>& base,int kind,int type) const noexcept {
  static_assert(N==4||N==8);if(!active||colour>=(destination==0&&reference::midi246Extended(type)?8:4))return base;
  auto c=controls;c[0]=int16_t(colour*(destination==0&&reference::midi246Extended(type)?2048:4096));std::array<int16_t,8> targets{};std::copy(base.begin(),base.end(),targets.begin());
  reference::bend246Apply(state,c,kind==2?2:(type==8||type==9)?1:0,kind,type,targets);std::array<int16_t,N> result{};std::copy_n(targets.begin(),N,result.begin());return result;
 }
};
}
