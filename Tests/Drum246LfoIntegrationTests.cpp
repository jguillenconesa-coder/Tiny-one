// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
 using namespace pocket;using namespace pocket::reference;
 Phase246Config output;output.prepare(48000);
 // Full callbacks and the held-modulation adapter agree on each own tick;
 // unsupported ENV calls preserve full DSP state, display, RNG and destinations.
 for(int type:{8,9})for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.})for(bool random:{false,true}){
  uint32_t rng=0x12345678,expectedRng=rng,meter=0;Drum246ModulationControl control;Drum246ModulationConfig config;config.random=random;config.value=!random;config.rng=&rng;config.step=44100./(64*rate);config.speed=13000;config.amount=24000;config.colour=2;config.secondary=true;config.randomControls={{24000,30000,0,24000}};config.tempo=value246Tempo(120);
  Value246State value;Random246State state;std::array<uint32_t,8> display{};std::array<int16_t,8> c{{13000,24000,0,12288,0,0,0,0}};
  if(random)c={{24000,30000,0,24000,0,0,0,0}};
  value.noteOn(c);state.noteOn(c[3]);const std::array<int16_t,8> base{{16000,1000,16000,12288,8192,12345,23456,16384}};double position=1;unsigned ticks=0;
  for(int frame=0;frame<int(rate*.15);++frame){auto targets=base;const bool tick=position>=1;if(tick){if(random)randomDrum246Step(state,expectedRng,meter,c,0,type,targets);else valueDrum246Step(value,display,c,config.tempo,0,type,targets,output.table);position-=1;++ticks;}const auto actual=control.next(base,type,config,output.table);if(tick){check(actual==targets,"Drum LFO hold differs from complete callback");check(control.value.phase==value.phase&&control.random.counter==state.counter&&rng==expectedRng,"Drum LFO state/RNG scheduling mismatch");}position+=config.step;}
  check(control.ticks==ticks,"Drum LFO clock depends on host rate");const auto beforeValue=control.value;const auto beforeRandom=control.random;const auto beforeDisplay=control.display;const auto beforeRng=rng;config.envelope=true;config.randomControls[2]=2048;
  for(int i=0;i<200;++i)check(control.next(base,type,config,output.table)==base,"Rejected drum ENV changes targets");
  check(std::memcmp(&beforeValue,&control.value,sizeof(value))==0&&std::memcmp(&beforeRandom,&control.random,sizeof(state))==0&&beforeDisplay==control.display&&beforeRng==rng,"Rejected drum ENV advances DSP state");
 }
 Kit kit;for(int pad=0;pad<24;++pad){auto& sample=kit[size_t(pad)];sample.rate=44100;sample.frames.resize(4410);for(size_t i=0;i<sample.frames.size();++i)sample.frames[i]=float(.35*std::sin(i*(.03+pad*.002)+i*i*.0000001));sample.prepareDrumPcm();}
 auto render=[&](double rate,bool dbox,int mode,bool bank,int colour){auto engine=std::make_unique<Engine>();engine->prepare(rate);engine->setKit(kit,24);Parameters p;p.engine=dbox?11:5;p.drum246=!dbox;p.effect=0;p.tone=.5;p.valueVoice246=mode==1||mode==3;p.random246=mode==2||mode==4;p.valueVoice246Envelope=mode==3;p.valueVoice246Colour=colour;p.drum246LfoBank=bank;p.valueVoice246Speed=13000;p.valueVoice246Amount=24000;p.random246Controls={{24000,32767,int16_t(mode==4?2048:0),24000}};for(auto& pad:p.pads)pad.nativeMode=2;
  engine->next(p);for(int n=60;n<72;++n)engine->noteOn(n,.7f,n%3+1);
  std::vector<float> audio;for(int i=0;i<16384;++i){const auto x=engine->next(p);check(std::isfinite(x)&&std::abs(x)<4,"Drum LFO audio unstable");audio.push_back(x);}
  check(engine->visualState(p.engine).active<=12,"Drum modulation exceeds twelve voices");engine->allNotesOff();for(int i=0;i<int(rate*.4);++i)engine->next(p);engine->reset();check(engine->visualState(p.engine).active==0,"Drum LFO panic retains voices");return audio;};
 for(double rate:{22050.,44100.,48000.,88200.,96000.})for(bool dbox:{false,true}){const auto baseline=render(rate,dbox,0,false,0);for(int mode:{1,2,3,4}){const auto audio=render(rate,dbox,mode,false,0);check((audio!=baseline)==(mode<3),"Drum modulation acceptance/ENV rejection not audible");}}
 for(bool dbox:{false,true})for(bool bank:{false,true})for(int colour=0;colour<4;++colour){const auto audio=render(48000,dbox,1,bank,colour);const auto different=audio!=render(48000,dbox,0,bank,colour);if(!different)std::cerr<<"engine "<<dbox<<" bank "<<bank<<" colour "<<colour<<"\n";check(different,"Drum Value target/bank has no audio effect");}
 std::cout<<"PASS: drum/D-box held callback composition at seven rates, rejected ENV full-state preservation, eight targets, twelve voices, five audio rates and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
