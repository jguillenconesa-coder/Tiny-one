// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(){try{
 pocket::Phase246Config rate;rate.prepare(88200);pocket::DrWave246Config config;config.prepare(rate);
 const std::array<int16_t,4> controls{},adsr{{4096,8192,24000,16383}};
 uint32_t random=0x554e4953u;std::array<uint32_t,6> pitches{};
 for(unsigned i=0;i<6;++i){auto v=std::make_unique<pocket::DrWave246Voice>();v->setDynamics(60,1);v->pitchDeviation=pocket::reference::voice246UnisonDeviation(random,i,false);
  for(int j=0;j<256;++j)v->source(440,controls,adsr,config,rate,false);
  const auto previous=v->renderedPitch;pitches[i]=previous;
  v->pitchDeviation=pocket::reference::voice246UnisonDeviation(random,i,true);v->requestRestart(72,.5f,true);
  for(int j=0;j<128;++j){v->source(880,controls,adsr,config,rate,false);check(v->renderedPitch==previous,"Pending detune changed old pitch");}
  const auto target=uint32_t(pocket::reference::voice246ApplyUnison(int32_t(pocket::DrWave246Voice::hostPitch(880)),v->pitchDeviation));
  check(v->output.envelope.field12==target,"Pending register lost Q31 detune");v->source(880,controls,adsr,config,rate,false);check(v->renderedPitch==target,"Unison pending pitch not delivered");
 }
 check(pitches[0]==pocket::DrWave246Voice::hostPitch(440)&&std::any_of(pitches.begin()+1,pitches.end(),[&](auto x){return x!=pitches[0];}),"No detuned voices");
 for(double sr:{8000.,44100.,48000.,88200.,96000.,192000.}){
  auto e=std::make_unique<pocket::Engine>();pocket::Parameters p;p.engine=6;p.drWave246Unison=true;p.hybridGlideMs=30;p.drWave246Adsr[3]=16383;p.effect=0;e->prepare(sr);e->selectEngine(6);e->configureHybridPerformance(0,30,true);
  auto render=[&](double sec){double energy=0;for(int i=0;i<int(sr*sec);++i){const auto s=e->next(p);check(std::isfinite(s)&&std::abs(s)<1,"Unison mix unstable");energy+=s*s;}return energy;};
  auto note=[&](int n,int ch){const auto s=e->visualState(6);check(s.active==6&&s.note==n&&s.channel==ch,"Six-voice priority failed");};
  e->noteOn(60,1,2);check(render(.05)>1e-5,"Unison silent");note(60,2);
  e->noteOn(67,.7f,1);e->noteOn(72,.5f,3);render(.01);const auto trigger=e->visualState(6).trigger;e->noteOff(67,1);note(72,3);check(e->visualState(6).trigger==trigger,"Older note retriggered group");
  e->noteOff(72,3);render(.03);note(60,2);e->sustainPedal(2,true);e->noteOff(60,2);e->noteOn(84,.8f,17);render(.01);e->noteOff(84,17);render(.03);note(60,2);
  e->sustainPedal(2,false);render(.2);check(e->visualState(6).active==0,"Unison sustain/drain left voices");
  e->noteOn(60,1);render(.01);e->noteOn(72,1);e->reset();render(.03);check(e->visualState(6).active==0,"Panic revived unison");
  e->noteOn(60,1);render(.01);p.drWave246Unison=false;render(.001);check(e->visualState(6).active==0,"Disabling Unison retained group");
  e->noteOn(60,1);e->noteOn(64,1);render(.01);check(e->visualState(6).active==2,"Poly not restored");
  p.drWave246Unison=true;p.engine=0;e->selectEngine(0);e->noteOn(60,1);render(.01);check(e->visualState(0).active==1,"Unison leaked to other engines");
 }
 std::cout<<"PASS: six Q31 detuned pitch/restart paths; Unison priority, sustain, channel17, drain, panic and mode/engine transitions at six rates\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
