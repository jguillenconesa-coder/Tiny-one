// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void sameEnvelope(const reference::Adsr246State& a,const reference::Adsr246State& b){
 check(a.stage==b.stage&&a.blocks==b.blocks&&a.level==b.level&&a.field12==b.field12&&a.field16==b.field16&&a.gain==b.gain&&a.frame==b.frame&&a.finished==b.finished,"Restart envelope differs from verified block model");
}
static std::vector<float> stream(double rate,bool earlyRelease){
 auto engine=std::make_unique<Engine>();Parameters p;p.engine=6;p.effect=0;p.drWave246Adsr={1024,8192,24000,16383};engine->prepare(rate);engine->selectEngine(6);
 std::vector<float> out;const int frames=int(rate*.3);
 for(int i=0;i<frames;++i){
  if(i==0)for(int n=48;n<54;++n)engine->noteOn(n,.8f,1);
  if(i==333)engine->noteOn(72,.5f,2);
  if(i==334)engine->noteOff(48,1); // The stolen owner cannot release its successor.
  if(i==335&&earlyRelease)engine->noteOff(72,2);
  if(i==337&&!earlyRelease)engine->noteOn(76,.6f,2);
  if(i==338&&!earlyRelease)engine->noteOn(79,.7f,3);
  if(i==450)engine->sustainPedal(2,true);
  if(i==500)engine->noteOff(72,2);
  if(i==600)engine->sustainPedal(2,false);
  if(i==1000)engine->allNotesOff();
  const auto sample=engine->next(p);check(std::isfinite(sample)&&std::abs(sample)<1,"Unstable restart stream");out.push_back(sample);
 }
 check(engine->visualState(6).active==0,"Restart stream left active voices");
 for(int n=48;n<54;++n)engine->noteOn(n,1);engine->next(p);engine->noteOn(84,1);engine->reset();engine->next(p);check(engine->visualState(6).active==0,"Panic resurrected pending voice");return out;
}
int main(){try{
 check(reference::voice246SelectBusy({0,0,0,0,0,0})==0,"First tie not preserved");
 check(reference::voice246SelectBusy({0xffffffffu,5,0x80000000u,5,4,3})==1,"Signed counter scan differs");
 check(reference::voice246SelectBusy({0xffffffffu,0x80000000u,0xffffffffu,0x80000000u,0xffffffffu,0x80000000u})==-1,"Excluded all-negative domain accepted");
 Phase246Config rate;rate.prepare(88200);DrWave246Config bank;bank.prepare(rate);
 const std::array<int16_t,4> controls{{0,0,0,0}},adsr{{4096,8192,24000,16383}};
 auto voice=std::make_unique<DrWave246Voice>();voice->setDynamics(60,1);
 auto sample=[&](double hz,bool release=false){return voice->source(hz,controls,adsr,bank,rate,release);};
 for(int i=0;i<273;++i)sample(261.6255653005986);
 const auto gain=voice->output.envelope.gain,oldPitch=voice->renderedPitch;
 const auto before=voice->state;const auto beforeLevel=voice->output.envelope.level;
 voice->requestRestart(72,.5f);check(voice->selectionBlocks()==0,"Queued owner retained old selection age");check(std::memcmp(&before,&voice->state,sizeof(before))==0&&voice->output.envelope.level==beforeLevel,"Request reset core or envelope immediately");
 for(int i=17;i<128;++i){sample(523.2511306011972);check(voice->restartQueued&&voice->renderedPitch==oldPitch,"Request applied before block boundary");}
 auto expected=voice->output.envelope;auto noise=voice->output.envelopeNoise;
 reference::adsr246RequestRestart(expected,DrWave246Voice::hostPitch(523.2511306011972),64*256);
 std::array<int16_t,128> envelope{};reference::adsr246Block(expected,adsr,voice->output.envelopeVolume,noise,envelope);
 for(int i=0;i<128;++i){sample(523.2511306011972);check(voice->renderedPitch==oldPitch&&voice->output.envelope.gain==gain,"Ramp changed old pitch/gain");}
 sameEnvelope(voice->output.envelope,expected);check(voice->restartArmed&&expected.stage==7,"Missing complete 128-frame ramp");
 sample(523.2511306011972);check(!voice->restartPending()&&voice->renderedPitch==expected.field12&&voice->output.envelopeVolume==64*256&&voice->output.envelope.gain==gain,"Pending words were not consumed / gain not preserved");
 auto fresh=std::make_unique<DrWave246Voice>();fresh->source(523.2511306011972,controls,adsr,bank,rate,false);
 check(voice->block==fresh->block&&voice->counter==1,"Core not reset at quiet commit boundary");
 // Latest queued target wins; no intermediate note reaches the pitch buffer.
 while(voice->cursor!=128)sample(523.2511306011972);
 voice->requestRestart(75,.2f);voice->requestRestart(79,.9f);
 for(int i=0;i<128;++i)sample(783.9908719634985);
 sample(783.9908719634985);check(voice->dynamicsNote==79&&voice->output.envelopeVolume==114*256,"Queued target was not replaced");
 // A key-up before commit cancels the new attack and preserves old release pitch.
 for(bool armed:{false,true}){
  auto v=std::make_unique<DrWave246Voice>();v->setDynamics(60,1);for(int i=0;i<256;++i)v->source(261.6255653,controls,adsr,bank,rate,false);
  const auto old=v->renderedPitch;v->requestRestart(84,1);
  if(armed)for(int i=0;i<128;++i)v->source(1046.502261,controls,adsr,bank,rate,false);
  for(int i=0;i<4096;++i){v->source(1046.502261,controls,adsr,bank,rate,true);check(!v->restartPending()&&v->renderedPitch==old,"Cancelled note reached render");}
  check(v->output.envelopeDone(),"Cancelled restart did not release");
 }
 // Dense steals and early cancellation at three actual host sample rates.
 for(double sr:{44100.,48000.,96000.})for(bool release:{false,true})check(!stream(sr,release).empty(),"Empty restart stream");
 std::cout<<"PASS: deferred request, original 128-frame ramp, pending pitch/velocity transfer, retained gain, quiet core reset, replacement, cancellation, panic and six host-rate stress streams\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
