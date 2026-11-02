// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char**){try{
 // Host adapter must replay both original instrument passes on fresh bases.
 for(int type:{0,1,7,2,3,4})for(bool envelope:{false,true}){
  pocket::Random246Control host;pocket::reference::Random246State original;uint32_t hostRng=123,originalRng=123,meter=0;
  const std::array<int16_t,4> raw{{24000,32767,int16_t(envelope?2048:0),24000}};host.noteOn();original.noteOn(raw[3]);
  for(int n=0;n<1024;++n){std::array<int16_t,8> c{{raw[0],raw[1],raw[2],raw[3],0,0,0,0}},targets{{8192,8192,8192,8192,100,200,300,400}};
   pocket::reference::random246VoiceStep(original,originalRng,meter,c,envelope?1:0,type,targets);
   const auto actual=host.next(true,raw,type,1,hostRng,{8192,8192,8192,8192});
   check(std::equal(actual.begin(),actual.end(),targets.begin())&&hostRng==originalRng&&std::memcmp(&host.state,&original,44)==0,"Host adapter diverged from original kernel");
  }
  const auto before=host.state;host.noteOn();original.noteOn(raw[3]);
  check(host.state.targets==before.targets&&host.state.counter==before.counter,"Note reset discarded retained Random stream");
 }
 // Voices consume one shared RNG rather than replaying cloned local streams.
 {pocket::Random246Control a,b;uint32_t rng=123;a.noteOn();b.noteOn();a.next(true,{24000,32767,0,24000},1,1,rng,{8192,8192,8192,8192});b.next(true,{24000,32767,0,24000},1,1,rng,{8192,8192,8192,8192});check(a.state.targets!=b.state.targets,"Voices cloned their Random streams");}
 // Callback count is independent of host sampling rate (first callback is immediate).
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  pocket::Random246Control host;uint32_t rng=123;for(int n=0;n<int(rate);++n)host.next(true,{20000,30000,0,24000},1,689.0625/rate,rng,{8192,8192,8192,8192});
  check(host.ticks==uint64_t(1+std::floor((rate-1)*689.0625/rate)),"Wrong host control cadence");
 }
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.})for(int type:{-1,0,1,2,6,7})for(bool envelope:{false,true}){
  if(argc>1&&rate!=48000&&rate!=96000)continue;
  auto run=[&](bool enabled,int amount,bool lowerPriority){auto e=std::make_unique<pocket::Engine>();pocket::Parameters p;p.engine=type<0?0:type;p.phase246=true;p.digital246=type!=-1;p.phase246Envelope=p.digital246Envelope=true;p.effect=0;p.volume=1;p.cluster246Controls={8192,0,12000,30};p.random246=enabled;p.random246Controls={int16_t(amount),32767,int16_t(envelope?2048:0),24000};p.tremolo246=p.valueVoice246=p.value246=lowerPriority;p.lfoDepth=lowerPriority?1:0;
   e->prepare(rate);e->selectEngine(p.engine,p.digital246);e->noteOn(57,.8f);std::vector<float> samples;
   for(int i=0;i<int(rate*.35);++i){const auto s=e->next(p);check(std::isfinite(s)&&std::abs(s)<3,"Unstable Random render");samples.push_back(s);}
   // Stop release modulation before checking finite release timing.
   p.random246=false;p.string246Adsr[3]=p.fm246Adsr[3]=p.drWave246Adsr[3]=p.cluster246Adsr[3]=p.phase246Adsr[3]=p.digital246Adsr[3]=16383;e->sustainPedal(1,true);e->noteOff(57);for(int i=0;i<128;++i)e->next(p);check(e->visualState(type).active==1,"Random changed sustain behavior");e->sustainPedal(1,false);for(int i=0;i<int(rate*2);++i)e->next(p);check(e->visualState(type).active==0,"Random left hanging release");e->reset();for(int i=0;i<128;++i)check(e->next(p)==0,"Random generated sound after panic");return samples;};
  const auto baseline=run(false,0,false),neutral=run(true,0,false),modulated=run(true,24000,false),priority=run(true,24000,true);
  if(type==2)check(baseline!=neutral,"String Random must share and advance the excitation RNG even at zero depth");else check(baseline==neutral,"Zero-amount Random changed valid native controls");check(modulated==priority,"Random priority differs from host policy");const auto& reference=type==2?neutral:baseline;double difference=0;for(size_t i=0;i<reference.size();++i)difference+=std::abs(reference[i]-modulated[i]);check(difference>.01,"Random instrument/envelope modulation inaudible");
 }
 std::cout<<"PASS: Random kernel/adapter replay, retained stream, host cadence; six engines, both destinations, seven rates (two in sanitizer mode), neutral identity, modulation priority, sustain/release and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
