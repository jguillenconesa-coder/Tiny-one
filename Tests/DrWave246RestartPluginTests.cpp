// SPDX-License-Identifier: AGPL-3.0-only
#include "PluginProcessor.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
struct Event{int sample;juce::MidiMessage midi;};
static std::vector<float> render(double rate,int block,int policy,const juce::File& sounds,bool oldOff=true){
 auto p=std::make_unique<PocketProcessor>(sounds);
 auto parameter=[&](const char* id,float value){auto* a=p->state.getParameter(id);check(a!=nullptr,"Parameter missing");a->setValueNotifyingHost(a->convertTo0to1(value));};
 parameter("engine",6);parameter("effect",0);
 p->phaseParameters().getParameter("drWave246Release")->setValueNotifyingHost(1);
 p->prepareToPlay(rate,block);
 std::vector<Event> events;
 auto on=[&](int frame,int note,int channel){events.push_back({frame,juce::MidiMessage::noteOn(channel,note,.8f)});};
 auto off=[&](int frame,int note,int channel){events.push_back({frame,juce::MidiMessage::noteOff(channel,note)});};
 for(int n=48;n<54;++n)on(0,n,1);
 on(333,72,2);if(oldOff)off(334,48,1);
 if(policy==0)off(335,72,2);
 else{on(337,76,2);on(338,79,3);}
 if(policy==2){on(598,84,4);events.push_back({600,juce::MidiMessage::allSoundOff(1)});}
 for(int ch=1;ch<=4;++ch)events.push_back({1000,juce::MidiMessage::allNotesOff(ch)});
 std::stable_sort(events.begin(),events.end(),[](const auto& a,const auto& b){return a.sample<b.sample;});
 std::vector<float> out;const int total=int(rate*.3);double energy=0;
 for(int start=0;start<total;start+=block){const int frames=std::min(block,total-start);juce::AudioBuffer<float> audio(2,frames);audio.clear();juce::MidiBuffer midi;
  for(const auto& e:events)if(e.sample>=start&&e.sample<start+frames)midi.addEvent(e.midi,e.sample-start);
  p->processBlock(audio,midi);
  for(int i=0;i<frames;++i){const auto value=audio.getSample(0,i);check(std::isfinite(value)&&std::abs(value)<1,"Unstable processor restart stream");if(policy==2&&start+i>=600)check(value==0,"Panic revived pending note/output");out.push_back(value);energy+=value*value;}
 }
 check(energy>1e-6&&p->visualVoices.load()==0,"Processor restart silent or left notes");return out;
}
int main(){try{
 juce::ScopedJuceInitialiser_GUI init;
 const auto sounds=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("pocket-restart-test",{},false);sounds.createDirectory();
 struct Cleanup{juce::File file;~Cleanup(){file.deleteRecursively();}} cleanup{sounds};
 for(double rate:{44100.,48000.,96000.})for(int policy:{0,1,2}){
  auto baseline=render(rate,64,policy,sounds);
  check(baseline==render(rate,257,policy,sounds),"Processor restart audio depends on callback partition");
  check(baseline==render(rate,64,policy,sounds,false),"Old owner note-off affected its successor");
 }
 std::cout<<"PASS: nine processor streams bit-identical across 64/257 callbacks, early release, dense steals, cross-channel owner removal and panic without resurrection\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
