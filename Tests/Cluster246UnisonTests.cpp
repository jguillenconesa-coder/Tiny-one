// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(){try{
 pocket::Phase246Config rate;rate.prepare(88200);pocket::Cluster246Config config;uint32_t random=0x554e4953u;size_t blocks=0;
 for(int count=1;count<=8;++count)for(unsigned voice=0;voice<6;++voice){
  std::array<int16_t,4> controls{{int16_t(std::max(3072,count*2048+1024)),4000,12000,30}},adsr{{64,8064,32767,16383}};
  auto actual=std::make_unique<pocket::Cluster246Voice>();actual->context.random=0x24600000u+voice;actual->pitchDeviation=pocket::reference::voice246UnisonDeviation(random,voice,false);
  pocket::reference::Cluster246State state{};auto context=actual->context;check(pocket::reference::cluster246Reset(state,context,controls,config.tables),"Reference reset rejected");
  std::array<uint32_t,128> pitch{};const int32_t base=int32_t(440./88200.*4294967296.+.5);pitch.fill(uint32_t(pocket::reference::voice246ApplyUnison(base,actual->pitchDeviation)));
  for(bool release:{false,true}){
   if(release)pocket::reference::cluster246Release(state);
   std::array<int16_t,128> expected{};check(pocket::reference::cluster246Block(state,context,controls,config.tables,pitch,rate.table,expected),"Reference render rejected");pocket::reference::cluster246DisplayDecay(context);
   for(int frame=0;frame<128;++frame)actual->source(440,controls,adsr,config,rate,release);
   check(actual->block==expected&&actual->renderedPitch==pitch[0],"Six-voice adapter block differs from verified kernel composition");
   check(std::memcmp(&actual->state,&state,sizeof(state))==0&&actual->context.random==context.random,"Cluster state/RNG differs");++blocks;
  }
 }
 check(blocks==96,"Composition grid");
 for(double sr:{8000.,44100.,48000.,88200.,96000.,192000.}){
  auto e=std::make_unique<pocket::Engine>();pocket::Parameters p;p.engine=7;p.cluster246Unison=true;p.hybridGlideMs=30;p.cluster246Adsr[3]=16383;p.effect=0;e->prepare(sr);e->selectEngine(7);e->configureHybridPerformance(0,30,true);
  auto render=[&](double sec){double energy=0;for(int i=0;i<int(sr*sec);++i){const auto s=e->next(p);check(std::isfinite(s)&&std::abs(s)<1,"Unstable Cluster Unison");energy+=s*s;}return energy;};
  auto note=[&](int n,int ch){const auto s=e->visualState(7);check(s.active==6&&s.note==n&&s.channel==ch,"Cluster Unison priority");};
  e->noteOn(60,1,2);check(render(.05)>1e-5,"Cluster Unison silent");note(60,2);
  e->noteOn(67,.8f,1);e->noteOn(72,.5f,3);render(.01);const auto trigger=e->visualState(7).trigger;e->noteOff(67,1);note(72,3);check(e->visualState(7).trigger==trigger,"Old owner retriggered group");
  e->noteOff(72,3);render(.03);note(60,2);e->sustainPedal(2,true);e->noteOff(60,2);e->noteOn(84,.8f,17);render(.01);e->noteOff(84,17);render(.03);note(60,2);
  e->sustainPedal(2,false);render(.2);check(e->visualState(7).active==0,"Cluster group left notes");
  e->noteOn(60,1);render(.01);e->noteOn(72,1);e->reset();render(.03);check(e->visualState(7).active==0,"Cluster Panic revived group");
  e->noteOn(60,1);render(.01);p.cluster246Unison=false;render(.001);check(e->visualState(7).active==0,"Unison off retained group");e->noteOn(60,1);e->noteOn(64,1);render(.01);check(e->visualState(7).active==2,"Cluster Poly not restored");
 }
 std::cout<<"PASS: 96 six-voice/count/reset-release blocks (12288 samples) against verified kernel composition, plus six-rate priority/sustain/channel17/mode/Panic streams\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
