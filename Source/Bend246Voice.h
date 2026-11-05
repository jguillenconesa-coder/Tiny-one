// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Bend246Special.h"
namespace pocket {
// Own per-voice 128-sample capture of the shared host Bend coefficients.
// The gain coefficient is owned/advanced by Bend246Control, not by each voice.
struct Bend246Voice {
 int target=0,pendingTarget=0;int16_t gain=0,pendingGain=0,pitchFactor=0,pendingPitch=0;
 unsigned frame=0;bool enabled=false,started=false,blocked=false;
 void configure(bool on,int selected,int16_t sharedGain,int16_t sharedPitch,bool blockPitch) noexcept {
  const bool wanted=on&&selected>=0&&selected<2;if(!wanted||!enabled){started=false;frame=0;}
  enabled=wanted;blocked=blockPitch;pendingTarget=selected;pendingGain=sharedGain;pendingPitch=sharedPitch;
 }
 void begin() noexcept {target=pendingTarget;gain=pendingGain;pitchFactor=pendingPitch;frame=0;started=true;}
 uint32_t pitch(uint32_t input) noexcept {if(!enabled)return input;if(!started)begin();return target==1?reference::bend246VoicePitch(input,pitchFactor):input;}
 void pitchBlock(std::array<uint32_t,128>& values) noexcept {if(!enabled)return;begin();if(target==1)for(auto& value:values)value=reference::bend246VoicePitch(value,pitchFactor);}
 int16_t amplitude(int16_t input) noexcept {
  if(!enabled||(!started&&blocked))return input;if(!started)begin();
  const auto value=target==0?reference::midi246VoiceGain(input,gain):input;if(++frame==128)started=false;return value;
 }
};
}
