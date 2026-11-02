// SPDX-License-Identifier: AGPL-3.0-only
#include "PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void set(PocketProcessor& p,const char* id,float value){auto* parameter=p.state.getParameter(id);check(parameter!=nullptr,"Missing output parameter");parameter->setValueNotifyingHost(parameter->convertTo0to1(value));}
static const char* modes[]={nullptr,"punch246Enabled","filter246Enabled","nitro246Enabled","delay246Enabled","grid246Enabled","phone246Enabled"};
static std::vector<float> render(int mode,double rate,int block,float volume,bool automate){
 auto p=std::make_unique<PocketProcessor>();p->applyDrWave246Init();
 if(modes[mode])p->phaseParameters().getParameter(modes[mode])->setValueNotifyingHost(1);
 set(*p,"volume",volume);set(*p,"inputMonitor",1);set(*p,"masterEcho",.7f);set(*p,"masterDelayTime",.05f);set(*p,"masterFeedback",.8f);set(*p,"masterDrive",.4f);
 p->prepareToPlay(rate,block);check(p->getLatencySamples()==pocket::Filter246Effect::latencyFor(rate),"Master routing changed PDC");
 const int mute=int(rate*.12),unmute=int(rate*.28),frames=int(rate*.48);std::vector<float> result;result.reserve(size_t(frames));
 for(int start=0;start<frames;){
  if(automate&&start==mute)set(*p,"volume",0);
  if(automate&&start==unmute)set(*p,"volume",volume);
  int n=std::min(block,frames-start);for(int boundary:{mute,unmute})if(start<boundary)n=std::min(n,boundary-start);
  juce::AudioBuffer<float> audio(2,n);audio.clear();
  for(int i=0;i<n;++i){const int t=start+i;const float x=t<int(rate*.06)?float(.35*std::sin(t*.071)+.1*std::sin(t*.137)):0;audio.setSample(0,i,x);audio.setSample(1,i,x);}
  juce::MidiBuffer midi;const int on=37,off=int(rate*.05);if(on>=start&&on<start+n)midi.addEvent(juce::MidiMessage::noteOn(1,60,.8f),on-start);if(off>=start&&off<start+n)midi.addEvent(juce::MidiMessage::noteOff(1,60),off-start);
  p->processBlock(audio,midi);for(int i=0;i<n;++i){check(audio.getSample(0,i)==audio.getSample(1,i),"Output channels disagree");check(std::isfinite(audio.getSample(0,i)),"Non-finite output");result.push_back(audio.getSample(0,i));}start+=n;
 }
 return result;
}
int main(){try{
 juce::ScopedJuceInitialiser_GUI init;
 for(double rate:{44100.,48000.,96000.})for(int mode=0;mode<7;++mode){
  const auto unity=render(mode,rate,257,1,false),quarter=render(mode,rate,257,.25f,false),automated=render(mode,rate,257,.25f,true),partition=render(mode,rate,64,.25f,true);
  double energy=0,tail=0;
  for(size_t i=0;i<unity.size();++i){
   check(std::abs(quarter[i]-.25f*unity[i])<2.e-7f,"Master gain changes FX processing instead of scaling its complete output");
   check(std::abs(automated[i]-partition[i])<1.e-6f,"Master volume automation depends on host block size");
   if(i>=size_t(rate*.22)&&i<size_t(rate*.28))check(automated[i]==0,"Master mute leaves an audible effect tail");
   if(i>=size_t(rate*.39)){if(std::abs(automated[i]-quarter[i])>=2.e-7f){std::cerr<<"mode="<<mode<<" rate="<<rate<<" frame="<<i<<" actual="<<automated[i]<<" expected="<<quarter[i]<<" diff="<<automated[i]-quarter[i]<<"\n";throw std::runtime_error("Muting destroyed retained FX state");}tail+=double(quarter[i])*quarter[i];}
   energy+=double(unity[i])*unity[i];
  }
  if(energy<.001||tail<1.e-12){std::cerr<<"mode="<<mode<<" rate="<<rate<<" energy="<<energy<<" tail="<<tail<<'\n';throw std::runtime_error("Output regression did not exercise signal and retained tails");}
 }
 std::cout<<"PASS: master gain scales complete output across seven FX selections and three rates; exact settled mute, retained tails, stereo and 64/257 automation\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
