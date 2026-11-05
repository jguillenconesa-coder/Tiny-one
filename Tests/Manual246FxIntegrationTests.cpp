// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include "Filter246Effect.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char**){try{const bool quick=argc>1;std::array<int16_t,8192> sine{};for(size_t i=0;i<sine.size();++i)sine[i]=int16_t(std::lround(32767*std::sin(i*6.283185307179586/sine.size())));
 auto render=[&](double rate,int fx,bool crank,int colour,int partition,bool motion,bool enabled){
  auto engine=std::make_unique<Engine>();engine->prepare(rate);auto effect=std::make_unique<Filter246Effect>();effect->prepare(rate);Parameters p;p.engine=11;p.crank246=crank;p.bend246=!crank;p.crank246Destination=p.bend246Destination=3;p.crank246Colour=p.bend246Colour=colour;p.crank246Amount=p.bend246Amount=24000;p.bend246Angle=8192;
  for(int n=0;n<int(rate*.2);++n)engine->next(p);std::vector<float> result;const int count=int(rate*.35),event=int(rate*.02);result.reserve(size_t(count));
  const std::array<int16_t,4> controls=fx==0?std::array<int16_t,4>{10000,4000,0,0}:fx==1?std::array<int16_t,4>{4000,10000,0,0}:fx==2?std::array<int16_t,4>{12000,7000,8000,0}:fx==3?std::array<int16_t,4>{12000,6000,8000,0}:fx==4?std::array<int16_t,4>{8000,8000,8000,0}:fx==5?std::array<int16_t,4>{8000,8000,8000,0}:std::array<int16_t,4>{8000,8000,8000,0};
  for(int start=0;start<count;start+=partition)for(int n=start;n<std::min(start+partition,count);++n){if(motion&&n>=event){p.crank246Angle=4096;p.bend246Angle=4096;if(n>=int(rate*.18))p.crank246Amount=p.bend246Amount=-24000;}engine->next(p);const auto snapshot=engine->manualFxAudio();const float input=float(.15*std::sin(n*.071)+.12*std::cos(n*.031));const float v=effect->next(input,enabled,controls,sine,fx==1,fx==2,fx==3,fx==4,fx==5,fx==6,nullptr,&snapshot);check(std::isfinite(v)&&std::abs(v)<8,"Manual FX output invalid");result.push_back(v);}
  check(effect->latency==Filter246Effect::latencyFor(rate)&&effect->underruns==0,"Manual FX changed PDC or underruns");engine->reset();check(engine->manualFxAudio().colour==-1,"Manual FX panic retains route/source");effect->reset();check(effect->cursor==0&&effect->completed==-1,"Manual FX reset retains buffers");return result;
 };
 size_t cases=0;for(double rate:{32000.,44100.,48000.,88200.,96000.,176400.,192000.})if(!quick||rate==48000.)for(int fx=0;fx<7;++fx)for(bool crank:{false,true})for(int colour:{2,3})for(bool enabled:{false,true}){const auto a=render(rate,fx,crank,colour,64,true,enabled),b=render(rate,fx,crank,colour,257,true,enabled),neutral=render(rate,fx,crank,colour,64,false,enabled);check(a==b,"Manual FX audio depends on host partition");check(std::equal(a.begin(),a.begin()+int(rate*.02),neutral.begin()),"Manual FX motion affects earlier audio");check(enabled?a!=neutral:a==neutral,"Manual FX routing/bypass mismatch");++cases;}
 // Pure input transforms preserve zero and zero amount; signed extremes saturate.
 for(int source:{-32768,-1,0,1,32767})for(int amount:{-32768,-1,0,1,32767})for(int colour:{2,3}){check(reference::manual246FxSample(0,int16_t(source),int16_t(amount),colour)==0,"Manual FX creates audio from silence");for(int sample:{-32768,-32767,-1,0,1,32767})if(amount==0)check(reference::manual246FxSample(int16_t(sample),int16_t(source),0,colour)==sample,"Manual FX zero amount changes input");}
 std::cout<<"PASS: "<<cases<<" Bend/Crank FX rate/effect/curve/bypass cases; exact 64/257 audio, signed amount, timestamps, PDC, reset and silence\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
