// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
 auto engine=std::make_unique<pocket::Engine>();pocket::Parameters p;p.engine=7;p.volume=1;p.effect=0;p.cluster246Controls={3072,0,512,3};
 for(double rate:{44100.,48000.,88200.,96000.}){
  engine->prepare(rate);engine->selectEngine(7);engine->noteOn(69,1);
  double energy=0;int crossings=0;float previous=0;
  for(int i=0;i<int(rate);++i){float s=engine->next(p);check(std::isfinite(s)&&std::abs(s)<1,"Invalid output");if(i>=int(rate*.1)){energy+=s*s;if(previous<=0&&s>0)++crossings;}previous=s;}
  check(energy>1,"Silent default patch");check(std::abs(crossings/.9-440)<3,"Host-rate tuning drift");
  engine->pitchBend(1,2);crossings=0;
  for(int i=0;i<int(rate);++i){float s=engine->next(p);if(i>=int(rate*.1)&&previous<=0&&s>0)++crossings;previous=s;}
  check(std::abs(crossings/.9-440*std::exp2(2./12))<3,"Pitch bend failed");
  engine->sustainPedal(1,true);engine->noteOff(69);for(int i=0;i<1000;++i)engine->next(p);check(engine->visualState(7).active==1,"Sustain released key early");
  p.cluster246Adsr[3]=16383;engine->sustainPedal(1,false);for(int i=0;i<int(rate*2);++i)engine->next(p);check(engine->visualState(7).active==0,"Release did not finish");p.cluster246Adsr[3]=8192;
 }
 engine->prepare(48000);engine->selectEngine(7);
 for(int note=40;note<52;++note){engine->noteOn(note,.8f);for(int i=0;i<300;++i)engine->next(p);check(engine->visualState(7).active<=6,"Exceeded six voice limit");}
 check(engine->visualState(7).active==6,"Missing voices after stealing");
 // Exercise all legal corner combinations and MIDI extremes with held voices.
 for(int corner=0;corner<16;++corner){for(size_t k=0;k<4;++k)p.cluster246Controls[k]=int16_t(corner&(1<<k)?pocket::reference::cluster246ControlMax[k]:pocket::reference::cluster246ControlMin[k]);engine->noteOn(corner&1?127:0,1);for(int i=0;i<8192;++i){float s=engine->next(p);check(std::isfinite(s)&&std::abs(s)<=1,"Unstable control corner");}}
 engine->reset();check(engine->visualState(7).active==0,"Panic retained voices");engine->selectEngine(7);engine->noteOn(69,1);engine->next(p);engine->selectEngine(0);check(engine->visualState(0).active==0,"Engine change retained incompatible voices");
 // Compare each colour using an identical independent RNG seed/voice lifetime.
 std::vector<float> baseline;
 for(int control=-1;control<4;++control){auto fresh=std::make_unique<pocket::Engine>();fresh->prepare(48000);fresh->selectEngine(7);p.cluster246Controls={8192,4000,12000,30};if(control>=0)p.cluster246Controls[size_t(control)]=std::array<int16_t,4>{{16384,20000,22000,1200}}[size_t(control)];fresh->noteOn(57,1);double difference=0;for(size_t i=0;i<16000;++i){float s=fresh->next(p);if(control<0)baseline.push_back(s);else difference+=std::abs(s-baseline[i]);}if(control>=0)check(difference>1,"Colour control is inaudible");}
 pocket::Cluster246Config config;
 for(int count=1;count<=8;++count){auto observed=pocket::reference::cluster246ObservedNormalization(count);check(config.tables.normalization[size_t(count-1)]==(observed?*observed:int16_t(16384/count)),"Hybrid normalization altered observed entries");}
 // The wrapper must hand the unchanged kernel its declared controls/pitch/table.
 pocket::Phase246Config rate;rate.prepare(88200);
 for(int count=1;count<=8;++count){pocket::Cluster246Voice adapter;pocket::reference::Cluster246State raw;pocket::reference::Cluster246Context context;adapter.context.random=context.random=uint32_t(count)*2654435761u;std::array<int16_t,4> controls{{int16_t(count*2048+(count==1?1024:0)),7000,18000,200}};std::array<uint32_t,128> increments;increments.fill(uint32_t(440./88200*4294967296.+.5));std::array<int16_t,128> expected{};check(pocket::reference::cluster246Reset(raw,context,controls,config.tables),"Reference reset failed");for(int block=0;block<3;++block){const bool release=block==2;if(release)pocket::reference::cluster246Release(raw);check(pocket::reference::cluster246Block(raw,context,controls,config.tables,increments,rate.table,expected),"Reference render failed");pocket::reference::cluster246DisplayDecay(context);for(int i=0;i<128;++i)adapter.source(440,controls,p.cluster246Adsr,config,rate,release);check(adapter.block==expected,"Hybrid changed core samples");check(std::memcmp(&raw,&adapter.state,sizeof raw)==0&&std::memcmp(&context,&adapter.context,sizeof context)==0,"Hybrid changed core state");}}
 // Crossing directly between hybrid engines must retire incompatible voices.
 engine->selectEngine(6);engine->noteOn(60,1);engine->selectEngine(7);check(engine->visualState(7).active==0,"Cross-hybrid switch retained voices");
 std::cout<<"PASS: Cluster hybrid tuning at four rates, bend, sustain/release, six voices/steal, control corners and audible controls\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
