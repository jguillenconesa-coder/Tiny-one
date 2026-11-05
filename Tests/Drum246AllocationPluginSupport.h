// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "PluginProcessor.h"
#include <memory>
#include <stdexcept>
inline void testDrum246AllocationPlugin(){
 for(double rate:{48000.,96000.})for(int quantum:{64,257}){
  auto baseline=std::make_unique<PocketProcessor>(),duplicates=std::make_unique<PocketProcessor>();
  for(auto* p:{baseline.get(),duplicates.get()}){
   p->applyDrum246Init();auto& raw=p->phaseParameters();for(int pad=0;pad<24;++pad)raw.getParameter(PocketProcessor::drum246ModeId(pad))->setValueNotifyingHost(1);
   p->state.getParameter("effect")->setValueNotifyingHost(0);p->prepareToPlay(rate,quantum);
  }
  double energy=0;float tail=0;
  for(int frame=0;frame<8192;frame+=quantum){const int count=std::min(quantum,8192-frame);juce::AudioBuffer<float> a(2,count),b(2,count);a.clear();b.clear();juce::MidiBuffer ma,mb;
   if(frame==0){ma.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);mb.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);}
   if(frame<=4096&&4096<frame+count){ma.addEvent(juce::MidiMessage::noteOff(1,60),4096-frame);mb.addEvent(juce::MidiMessage::noteOff(1,60),4096-frame);}
   for(int trigger:{64,128,512,1023,2048,4095})if(frame<=trigger&&trigger<frame+count)mb.addEvent(juce::MidiMessage::noteOn(1,60,.2f),trigger-frame);
   baseline->processBlock(a,ma);duplicates->processBlock(b,mb);
   for(int ch=0;ch<2;++ch)for(int i=0;i<count;++i){const float x=a.getSample(ch,i),y=b.getSample(ch,i);if(x!=y||!std::isfinite(y))throw std::runtime_error("Plugin held duplicate changed native drum audio");energy+=double(y)*y;if(frame+i>7500)tail=std::max(tail,std::abs(y));}
  }
  if(energy<=0||tail>1e-5f)throw std::runtime_error("Plugin allocation/release is silent or retains audio");
 }
}
