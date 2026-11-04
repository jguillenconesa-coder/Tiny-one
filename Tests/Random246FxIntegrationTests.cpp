// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket;
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){try{
 // Subsequent original callbacks: retained state, RNG, meter and held modulations.
 for(int fx=0;fx<8;++fx)for(int amount:{-32768,0,32767}){
  Random246FxControl host;reference::Random246State original;uint32_t a=123,b=123,meter=0;host.noteOn();original.noteOn(24000);
  for(int n=0;n<512;++n){const std::array<int16_t,4> raw{{int16_t(amount),32767,4096,24000}};std::array<int16_t,8> c{{raw[0],raw[1],4096,raw[3],0,0,0,0}},base{{int16_t(n*60),-1000,16000,32000,100,200,300,400}};auto expected=base;
   reference::random246FxStep(original,b,meter,c,2,fx,expected);
   const auto actual=host.next(true,raw,fx,1,a,{base[0],base[1],base[2],base[3]});
   check(std::equal(actual.begin(),actual.end(),expected.begin())&&std::memcmp(&host.state,&original,44)==0&&a==b&&host.meter==meter,"Random FX retained adapter diverged");
  }
  const auto retained=host.state;host.noteOn();check(host.state.targets==retained.targets&&host.state.counter==retained.counter,"FX note discarded its sequence");
  const std::array<int16_t,4> base{{100,200,300,400}};check(host.next(false,{24000,32767,4096,24000},fx,1,a,base)==base&&!host.active&&host.ticks==0,"FX disable did not reset/bypass");
 }
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  Random246FxControl host;uint32_t rng=123;host.noteOn();
  for(int n=0;n<int(rate);++n)host.next(true,{24000,32767,4096,24000},n<int(rate)/2?-1:5,689.0625/rate,rng,{8192,8192,8192,8192});
  check(host.ticks==uint64_t(1+std::floor((rate-1)*689.0625/rate)),"Random FX clock depends on rate or bypass");
 }
 // Bus routing must work on a legacy voice too and remove per-voice Random.
 for(int engine:{0,4,5,11})for(int fx=0;fx<8;++fx){
  auto run=[&](bool enabled,bool competing){auto e=std::make_unique<Engine>();Parameters p;p.engine=engine;p.drum246=engine==5;p.effect=0;p.volume=1;p.random246=enabled;p.random246Controls={30000,32767,4096,16000};p.value246=p.valueVoice246=p.tremolo246=competing;p.lfoDepth=competing?1:0;
   p.punch246=fx==2;p.delay246=fx==0;p.grid246=fx==1;p.phone246=fx==3;p.spring246=fx==4;p.filter246=fx==5;p.nitro246=fx==6;p.cwo246=fx==7;
   e->prepare(48000);e->selectEngine(engine,false,p.drum246);e->noteOn(60,.8f);std::vector<float> out;
   for(int n=0;n<8192;++n){const auto sample=e->next(p);check(std::isfinite(sample),"Random FX engine output is not finite");if(fx==2)out.push_back(sample);else for(auto word:e->filterControls())out.push_back(word);}
   if(enabled){const auto vis=e->visualState(engine);bool nonzero=false;for(auto x:vis.random)nonzero|=x!=0;check(nonzero,"Random FX visualization missing");}return out;
  };
  const auto a=run(true,false),b=run(true,true),neutral=run(false,false);check(a==b,"Random FX leaks competing LFO route");if(fx!=2||engine==0)if(a==neutral)throw std::runtime_error("Random FX does not reach effect controls/audio: engine="+std::to_string(engine)+" fx="+std::to_string(fx));
 }
 std::cout<<"PASS: Random FX retained kernel/bridge, seven rates, bypass clock, note/reset, four engine families x eight FX, modulation exclusivity and display\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
