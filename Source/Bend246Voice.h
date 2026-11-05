// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Bend246Special.h"
#include "Element246Pitch.h"
namespace pocket {
// Reusable per-voice capture for Bend, Crank and Element gain/pitch. Each controller
// owns its shared coefficients; voices never advance that shared smoothing.
struct Bend246Voice {
 int owner=0,target=0,pendingTarget=0,gainMode=0,pendingGainMode=0;int16_t gain=0,pendingGain=0,pitchFactor=0,pendingPitch=0;
 std::array<int16_t,64> external{};const std::array<int16_t,64>* pendingExternal=nullptr;unsigned pendingHead=0;
 unsigned frame=0;bool enabled=false,started=false,blocked=false;
 void configure(bool on,int selected,int16_t sharedGain,int16_t sharedPitch,bool blockPitch,int sourceOwner=1,int operation=0,const std::array<int16_t,64>* stream=nullptr,unsigned head=0) noexcept {
  const bool wanted=on&&selected>=0&&selected<2;if(!wanted||!enabled||owner!=sourceOwner){started=false;frame=0;}
  owner=wanted?sourceOwner:0;enabled=wanted;blocked=blockPitch;pendingTarget=selected;pendingGain=sharedGain;pendingPitch=sharedPitch;pendingGainMode=operation;pendingExternal=stream;pendingHead=head%64;
 }
 void begin() noexcept {target=pendingTarget;gainMode=pendingGainMode;gain=pendingGain;pitchFactor=pendingPitch;frame=0;started=true;if(gainMode==3||gainMode==6){if(pendingExternal)for(size_t i=0;i<64;++i)external[i]=(*pendingExternal)[(pendingHead+i)%64];else external.fill(0);}}
 uint32_t applyPitch(uint32_t input,unsigned index) const noexcept {return gainMode==6?reference::element246PitchExternal(input,external[index/2],gain):gainMode==5?reference::element246PitchEnvelope(input,pitchFactor):gainMode==4?reference::element246PitchRelative(input,pitchFactor):reference::bend246VoicePitch(input,pitchFactor);}
 uint32_t pitch(uint32_t input) noexcept {if(!enabled)return input;if(!started)begin();return target==1?applyPitch(input,frame):input;}
 void pitchBlock(std::array<uint32_t,128>& values) noexcept {if(!enabled)return;begin();if(target==1)for(size_t i=0;i<128;++i)values[i]=applyPitch(values[i],unsigned(i));}
 int16_t amplitude(int16_t input) noexcept {
  if(!enabled||(!started&&blocked))return input;if(!started)begin();
  const auto value=target==0?(gainMode==1?reference::saturate16(int(input)+gain):gainMode==3?reference::saturate16(int(input)+reference::multiplyQ15(external[frame/2],gain)):gainMode==2?reference::saturate16(int(input)+reference::multiplyQ15(input,gain)):reference::midi246VoiceGain(input,gain)):input;if(++frame==128)started=false;return value;
 }
};
}
