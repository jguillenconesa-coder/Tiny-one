// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
 using namespace pocket;Sample sample;sample.rate=44100;sample.frames.resize(4410);for(size_t i=0;i<sample.frames.size();++i)sample.frames[i]=float(std::sin(i*.05)*.5);sample.prepareDrumPcm();
 // Flat-rate adapter versus an independent raw-control invocation of the reader.
 Phase246Config rate;rate.prepare(88200);rate.filters={};Sampler246Voice voice;reference::Sampler246State state;state.construct();state.bind(1);const std::array<int16_t,8> c{{0,0,0,546,0,0,0,16384}};state.reset(c);std::array<uint32_t,128> pitch{};pitch.fill(262145);std::array<int16_t,128> audio{};reference::sampler246Block(state,c,pitch,0x7fffffff,{sample.drumPcm.data(),sample.drumPcm.size(),-8},reference::drum246FirCoefficients,audio);
 for(size_t block=0;block<80;++block){if(block)reference::sampler246Block(state,c,pitch,0x7fffffff,{sample.drumPcm.data(),sample.drumPcm.size(),-8},reference::drum246FirCoefficients,audio);for(size_t i=0;i<128;++i)check(voice.next(1,sample,Sampler246Voice::clipControls(sample),false,rate,false)==audio[i]/32768.f,"Sampler adapter/core composition mismatch");}

 // Host voice/event contracts, not an assertion of original allocation or ADSR.
 for(double sr:{8000.,11025.,22050.,44100.,48000.,88200.,96000.})for(bool reverse:{false,true})for(bool loop:{false,true}){
  auto engine=std::make_unique<Engine>();engine->prepare(sr);engine->setSample(sample);Parameters p;p.engine=3;p.sampler246=true;p.sampler246Controls=Sampler246Voice::clipControls(sample);p.sampler246Controls[4]=reverse?16384:0;p.sampler246Envelope=false;p.sampleLoop=loop;p.tone=0;p.color=1;p.effect=0;p.drive=0;p.lfoDepth=0;p.attack=0;p.release=0;p.envelope=0;
  engine->next(p);for(int note=60;note<68;++note)engine->noteOn(note,.8f);check(engine->visualState(3).active==6,"Native sampler does not use six voices");double energy=0;for(int i=0;i<int(sr*.2);++i){const auto x=engine->next(p);check(std::isfinite(x),"Nonfinite sampler output");energy+=x*x;}check(energy>0,"Native sampler silent");
  engine->sustainPedal(1,true);engine->allNotesOff();for(int i=0;i<int(sr*.7);++i)engine->next(p);check(engine->visualState(3).active==0,"Sampler voices not retired after panic");
 }
 std::cout<<"PASS: sampler reader composition, seven rates, six voices/stealing, reverse, loop, release and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
