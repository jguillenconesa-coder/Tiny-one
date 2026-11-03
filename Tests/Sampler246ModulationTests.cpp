// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
 using namespace pocket;using namespace pocket::reference;
 Phase246Config config;config.prepare(48000);
 const std::array<int16_t,8> base{{1000,2000,20000,24000,16384,12345,23456,16384}};
 // Compare held modulation against a fresh complete callback on each own tick,
 // including sequential coupled edits. Bases may change between callbacks.
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  ValueVoice246Control value;Random246Control random;value.noteOn();random.noteOn();
  Value246State expectedValue;std::array<uint32_t,8> display{};Random246State expectedRandom;
  const std::array<int16_t,8> vc{{12288,24000,0,12288,0,0,0,0}},rc{{24000,30000,0,24000,0,0,0,0}};
  expectedValue.noteOn(vc);expectedRandom.noteOn(rc[3]);uint32_t rng=0x12345678,otherRng=rng,meter=0;
  std::array<int16_t,8> heldValue{},heldRandom{};double position=1;unsigned ticks=0;
  for(int frame=0;frame<int(rate*.3);++frame){auto current=base;current[7]=int16_t(12000+frame%4096);
   const bool tick=position>=1;
   if(tick){heldValue=current;heldRandom=current;valueVoice246Step(expectedValue,display,vc,value246Tempo(120),0,6,heldValue,config.table);random246VoiceStep(expectedRandom,otherRng,meter,rc,0,6,heldRandom);position-=1;++ticks;}
   const auto a=value.next(true,false,false,12288,24000,6,6,value246Tempo(120),44100./(64*rate),current,config.table);
   const auto b=random.next(true,{{24000,30000,0,24000}},6,44100./(64*rate),rng,current);
   if(tick){check(a==heldValue&&b==heldRandom,"Sampler held modulation differs from complete callbacks");check(value.state.phase==expectedValue.phase&&random.state.counter==expectedRandom.counter&&rng==otherRng&&random.meter==meter,"Sampler control state/RNG scheduling differs");}
   position+=44100./(64*rate);
  }
  check(value.ticks==ticks&&random.ticks==ticks&&std::abs(double(ticks)-rate*.3*44100./(64*rate))<=1,"Sampler LFO clock depends on host rate");
  check(value.next(false,false,false,0,0,0,6,0,.1,base,config.table)==base,"Disabled sampler Value changes controls");check(random.next(false,{{0,0,0,0}},6,.1,rng,base)==base,"Disabled sampler Random changes controls");
 }
 Sample sample;sample.rate=44100;sample.frames.resize(44100);for(size_t i=0;i<sample.frames.size();++i)sample.frames[i]=float(.35*std::sin(i*2*pi*(330.+double(i)/250)/44100));sample.prepareDrumPcm();
 auto render=[&](double rate,int mode,bool secondary,int colour,bool envelope){auto engine=std::make_unique<Engine>();engine->prepare(rate);engine->setSample(sample);Parameters p;p.engine=3;p.sampler246=true;p.sampler246Secondary=secondary;p.sampler246Controls=Sampler246Voice::clipControls(sample);p.sampleLoop=true;p.sampleRoot=69;p.effect=0;p.sampler246Envelope=envelope;p.sampler246Adsr[3]=12000;p.sampler246Controls[2]=Sampler246Voice::encode(1000);p.sampler246Controls[3]=Sampler246Voice::encode(5000);
  p.valueVoice246=mode==1||mode==4;p.valueVoice246Envelope=mode==4;p.valueVoice246Colour=colour;p.valueVoice246Speed=13000;p.valueVoice246Amount=24000;
  p.random246=mode==2||mode==5;p.random246Controls={{24000,32767,int16_t(mode==5?2048:0),24000}};
  p.tremolo246=mode==3;p.tremolo246Controls={{13000,8000,20000,20000}};
  engine->next(p);for(int n:{57,60,64,67,69,72})engine->noteOn(n,.7f);
  std::vector<float> audio;for(int i=0;i<32768;++i){if(i==24576)for(int note:{57,60,64,67,69,72})engine->noteOff(note);const auto x=engine->next(p);check(std::isfinite(x)&&std::abs(x)<4,"Sampler modulation output unstable");audio.push_back(x);if(mode==3&&i==1000)check(engine->visualState(3).lfo!=.5f,"Sampler Tremolo display reads another voice route");}
  engine->allNotesOff();for(int i=0;i<int(rate*2);++i)engine->next(p);check(engine->visualState(3).active==0,"Sampler LFO prevents panic release");return audio;};
 for(double rate:{22050.,44100.,48000.,88200.,96000.})for(bool envelope:{false,true}){
  const auto baseline=render(rate,0,false,0,envelope);
  for(int mode:{1,2,3,4,5}){const auto audio=render(rate,mode,true,3,envelope);if(envelope||mode<4){if(audio==baseline)std::cerr<<"rate "<<rate<<" mode "<<mode<<" env "<<envelope<<"\n";check(audio!=baseline,"Sampler LFO has no audible effect");}else check(audio==baseline,"Sampler ADSR LFO affects disabled native ADSR");}
 }
 for(bool secondary:{false,true})for(int colour=0;colour<4;++colour){const auto different=render(48000,1,secondary,colour,true)!=render(48000,0,secondary,colour,true);if(!different)std::cerr<<"target bank "<<secondary<<" colour "<<colour<<"\n";check(different,"Sampler Value target/bank has no audible effect");}
 std::cout<<"PASS: sampler complete/held Value and Random composition at seven rates, all eight Value targets, five LFO routes, native/own ADSR, six voices and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
