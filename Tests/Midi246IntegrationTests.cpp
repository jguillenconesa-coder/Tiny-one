// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{using namespace pocket;using namespace pocket::reference;
 const std::array<uint32_t,4> cc{{80,48,70,58}};
 for(int type:{0,1,2,3,4,6,7,11,12,13,8,9})for(int route=0;route<17;++route){
  const std::array<int,4> routes{{route,route,route,route}};const std::array<int16_t,8> base{{12000,1000,16000,22000,8192,12345,23456,16384}};
  auto expected=base;const auto c=midi246Controls(routes,type);std::array<int16_t,4> display{};std::array<uint32_t,128> pitch{};std::array<int16_t,128> gain{};
  if(type==8||type==9)midi246DrumStep(c,cc,0,type,expected);else midi246SynthStep(c,cc,display,0,type,expected,pitch,gain);
  check(midi246Apply(base,0,type,routes,cc)==expected,"Host Instrument route mapping differs from verified callback");
  if(type==8||type==9){Drum246ModulationControl control;Drum246ModulationConfig config;config.midi=true;config.midiRoutes=routes;config.midiCC=cc;std::array<int16_t,8192> sine{};check(control.next(base,type,config,sine)==expected,"Per-hit MIDI adapter differs from original callback");}
 }
 for(int fx=0;fx<8;++fx)for(int route=0;route<17;++route){std::array<int16_t,4> base{{12000,9000,14000,16000}};std::array<int16_t,8> expected{};std::copy(base.begin(),base.end(),expected.begin());std::array<int16_t,128> audio{};const std::array<int,4> routes{{route,route,route,route}};midi246FxStep(midi246Controls(routes,fx),cc,2,fx,expected,audio);const auto actual=midi246Apply(base,2,fx,routes,cc);check(std::equal(actual.begin(),actual.end(),expected.begin()),"Host FX route differs from original callback");}
 Sample sample;sample.rate=44100;sample.frames.resize(12000);for(size_t i=0;i<sample.frames.size();++i)sample.frames[i]=float(.3*std::sin(i*.075)+.1*std::sin(i*.113));sample.prepareDrumPcm();Kit kit;for(auto& s:kit)s=sample;
 auto render=[&](double rate,int mode,int destination,bool enabled,int block){auto e=std::make_unique<Engine>();e->prepare(rate);e->setSample(sample);e->setKit(kit,24);Parameters p;p.engine=mode==12?0:mode;p.digital246=mode==12;p.phase246=p.phase246Envelope=p.digital246Envelope=p.sampler246=p.sampler246Envelope=true;p.drum246=true;p.effect=0;p.drive=0;p.tone=.5f;p.midi246=enabled;p.sampleLoop=true;p.sampler246Controls=Sampler246Voice::clipControls(sample);p.midi246Routes=destination==0?std::array<int,4>{{1,2,3,4}}:destination==1?std::array<int,4>{{9,10,11,12}}:std::array<int,4>{{13,14,15,16}};if(mode==3&&destination==0)p.midi246Routes={{8,5,6,7}};if(destination==2)p.punch246=true;for(auto& pad:p.pads)pad.nativeMode=2;e->next(p);e->noteOn(60,.8f,1);std::vector<float> result;
  for(int start=0;start<8192;start+=block)for(int i=start;i<std::min(start+block,8192);++i){if(i==1537)for(int j=0;j<4;++j)e->midiController(j+1,int(cc[size_t(j)]));if(i==6145)e->noteOff(60,1);const float v=e->next(p);check(std::isfinite(v)&&std::abs(v)<8,"MIDI audio is unstable");result.push_back(v);}e->reset();check(e->visualState(p.engine).active==0,"MIDI panic retains voices");return result;};
 size_t cases=0;for(double rate:{32000.,44100.,48000.,88200.,96000.,176400.,192000.})for(int mode:{0,1,2,3,5,6,7,8,9,10,11,12})for(int destination=0;destination<3;++destination){
  const auto active=render(rate,mode,destination,true,64),other=render(rate,mode,destination,true,257),base=render(rate,mode,destination,false,64);check(active==other,"MIDI output depends on host buffer partition");
  if(destination==1&&(mode==5||mode==11))check(active==base,"Drum MIDI ENV should be rejected");
  else {bool changed=false;for(size_t i=1537;i<active.size();++i)changed|=active[i]!=base[i];if(!changed){std::cerr<<"Silent route rate "<<rate<<" mode "<<mode<<" destination "<<destination<<"\n";}check(changed,"Supported MIDI destination does not affect audio");}
  ++cases;
 }
 std::cout<<"PASS: "<<cases<<" MIDI engine/destination/rate cases, offsets, 64/257 partition, rejected drum ENV and panic; four CC routes and eight FX adapters\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
