// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 check(argc==1||(argc==2&&std::string(argv[1])=="--sanitizer"),"Unknown test option");
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.})for(int type:{0,1,6,7}){
  if(argc>1&&rate!=48000&&rate!=96000)continue;
  auto run=[&](bool enabled,int shape,int pitch,int amplitude){auto e=std::make_unique<pocket::Engine>();pocket::Parameters p;p.engine=type;p.phase246=p.digital246=true;p.phase246Envelope=p.digital246Envelope=true;p.effect=0;p.volume=1;p.cluster246Controls={3072,0,512,3};p.tremolo246=enabled;p.tremolo246Shape=shape;p.tremolo246Controls={12000,int16_t(pitch),int16_t(amplitude),16000};p.value246Tempo=pocket::reference::value246Tempo(120);
   e->prepare(rate);e->selectEngine(type,p.digital246);e->noteOn(57,.8f);
   std::vector<float> samples;for(int i=0;i<int(rate*.35);++i){const auto s=e->next(p);check(std::isfinite(s)&&std::abs(s)<2,"Unstable Tremolo render");samples.push_back(s);}
   p.drWave246Adsr[3]=p.cluster246Adsr[3]=p.phase246Adsr[3]=p.digital246Adsr[3]=16383;e->sustainPedal(1,true);e->noteOff(57);for(int i=0;i<128;++i)e->next(p);check(e->visualState(type).active==1,"Tremolo changed sustain behavior");e->sustainPedal(1,false);for(int i=0;i<int(rate*2);++i)e->next(p);check(e->visualState(type).active==0,"Tremolo left hanging release");e->reset();for(int i=0;i<128;++i)check(e->next(p)==0,"Tremolo generated sound after panic");return samples;};
  const auto baseline=run(false,0,0,0),neutral=run(true,0,0,0);check(baseline==neutral,"Zero-depth Tremolo changed audio");
  for(int shape=0;shape<5;++shape)for(bool pitch:{false,true}){const auto samples=run(true,shape,pitch?-12000:0,pitch?0:18000);double difference=0;for(size_t i=0;i<samples.size();++i)difference+=std::abs(samples[i]-baseline[i]);check(difference>.1,"Tremolo pitch/amplitude shape is inaudible");}
 }
 // Extreme controls and MIDI endpoints with all native voices occupied.
 for(int type:{0,1,6,7}){auto e=std::make_unique<pocket::Engine>();pocket::Parameters p;p.engine=type;p.phase246=p.digital246=p.phase246Envelope=p.digital246Envelope=true;p.tremolo246=true;p.effect=0;e->prepare(48000);e->selectEngine(type,p.digital246);for(int note:{0,24,48,72,96,127,60,67})e->noteOn(note,1);
  for(int shape=0;shape<5;++shape)for(int corner=0;corner<16;++corner){p.tremolo246Shape=shape;p.tremolo246Controls={int16_t(corner&1?32767:0),int16_t(corner&2?32767:-32768),int16_t(corner&4?32767:0),int16_t(corner&8?32767:0)};for(int i=0;i<384;++i){const auto sample=e->next(p);check(std::isfinite(sample)&&std::abs(sample)<3,"Extreme Tremolo controls/voices unstable");}}
 }
 // For block engines, enabling halfway through a buffered render must wait for
 // its next block before starting the LFO's pitch/amplitude clocks together.
 for(int type:{6,7}){auto e=std::make_unique<pocket::Engine>();pocket::Parameters p;p.engine=type;p.effect=0;e->prepare(48000);e->selectEngine(type,p.digital246);e->noteOn(60,1);for(int i=0;i<37;++i)e->next(p);p.tremolo246=true;p.tremolo246Controls={20000,10000,20000,16000};for(int i=0;i<1000;++i){check(std::isfinite(e->next(p)),"Mid-block activation failed");if(i==333)p.tremolo246=false;if(i==370)p.tremolo246=true;}e->allNotesOff();}
 std::cout<<"PASS: Tremolo pitch and amplitude, five shapes on four native engines at "<<(argc>1?"two sanitizer":"seven")<<" rates, zero-depth identity, sustain/release, panic, control extremes and mid-block activation\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
