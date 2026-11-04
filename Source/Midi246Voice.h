// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Midi246Special.h"
namespace pocket {
// Authored per-voice scheduler around the recovered MIDI gain/pitch arithmetic.
// Initial zero display, ownership, 88.2 kHz/128-sample clock and resets are host
// policy. Blocked engines defer activation until their next pitch block.
struct Midi246Voice {
 std::array<int,4> routes{},blockRoutes{};
 std::array<uint32_t,4> cc{{64,64,64,64}},blockCC{{64,64,64,64}};
 std::array<int16_t,4> display{};
 unsigned frame=0;bool enabled=false,started=false,blockPitch=false,active=false;
 void configure(bool on,const std::array<int,4>& pendingRoutes,const std::array<uint32_t,4>& pendingCC,bool blocked) noexcept {
  const bool wanted=on&&std::any_of(pendingRoutes.begin(),pendingRoutes.end(),[](int route){return route==19||route==20;});
  if(!on&&active)display={};
  if(!wanted&&enabled){started=false;frame=0;}
  active=on;enabled=wanted;blockPitch=blocked;routes=pendingRoutes;cc=pendingCC;
 }
 void begin() noexcept {
  blockRoutes=routes;blockCC=cc;
  for(size_t i=0;i<4;++i)if(blockRoutes[i]==19)display[i]=reference::midi246VoiceGainCoefficient(display[i],blockCC[i]);
  frame=0;started=true;
 }
 uint32_t applyPitch(uint32_t input) const noexcept {
  for(size_t i=0;i<4;++i)if(blockRoutes[i]==20)input=reference::midi246VoicePitch(input,blockCC[i]);return input;
 }
 uint32_t pitch(uint32_t input) noexcept {
  if(!enabled)return input;if(!started)begin();return applyPitch(input);
 }
 void pitchBlock(std::array<uint32_t,128>& values) noexcept {
  if(!enabled)return;begin();for(auto& value:values)value=applyPitch(value);
 }
 int16_t amplitude(int16_t input) noexcept {
  if(!enabled||(!started&&blockPitch))return input;if(!started)begin();
  for(size_t i=0;i<4;++i)if(blockRoutes[i]==19)input=reference::midi246VoiceGain(input,display[i]);
  if(++frame==128)started=false;return input;
 }
};
}
