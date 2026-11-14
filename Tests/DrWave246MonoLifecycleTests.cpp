// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace pocket;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
 const std::array<int16_t,4> controls{{0,0,0,0}},adsr{{4096,8192,24000,16383}};
 Phase246Config internal;internal.prepare(88200);DrWave246Config bank;bank.prepare(internal);
 auto mono=std::make_unique<DrWave246Voice>();mono->output.envelope.gain=0xa5a50000u;mono->setDynamics(60,1);
 for(int i=0;i<273;++i)mono->source(261.6255653,controls,adsr,bank,internal,false);
 const auto oldGain=mono->output.envelope.gain,oldPitch=mono->renderedPitch;const auto oldVolume=mono->output.envelopeVolume;
 mono->requestRestart(72,.5f,true);
 for(int i=17;i<128;++i){mono->source(523.2511306,controls,adsr,bank,internal,false);check(mono->output.envelope.gain==oldGain&&mono->output.envelopeVolume==oldVolume,"Mono words changed before authored boundary");}
 auto expected=mono->output.envelope;auto noise=mono->output.envelopeNoise;
 reference::adsr246RequestRestart(expected,DrWave246Voice::hostPitch(523.2511306),64*256);
 expected.gain=(expected.gain&0xffff0000u)|(32768u-72u*128u);
 std::array<int16_t,128> samples{};reference::adsr246Block(expected,adsr,64*256,noise,samples);
 for(int i=0;i<128;++i){mono->source(523.2511306,controls,adsr,bank,internal,false);check(mono->renderedPitch==oldPitch&&mono->output.envelopeVolume==64*256&&mono->output.envelope.gain==expected.gain,"Mono request did not update words / changed pitch early");}
 const auto& actual=mono->output.envelope;
 check(actual.stage==expected.stage&&actual.blocks==expected.blocks&&actual.level==expected.level&&actual.field12==expected.field12&&actual.field16==expected.field16&&actual.gain==expected.gain&&mono->output.envelopeNoise.seed==noise.seed&&mono->output.envelopeNoise.previous==noise.previous,"Mono ramp differs from verified ADSR composition");
 mono->source(523.2511306,controls,adsr,bank,internal,false);check(!mono->restartPending()&&mono->renderedPitch==expected.field12,"Mono words not committed");
 size_t tails=0;
 for(double sr:{8000.,44100.,48000.,88200.,96000.,192000.}){
  Phase246Config rate;rate.prepare(sr);
  auto voice=std::make_unique<DrWave246Voice>();voice->setDynamics(60,1);
  for(int i=0;i<int(sr*.05);++i)voice->next(261.6255653,controls,adsr,bank,rate);
  int envelopeEnd=-1,voiceEnd=-1;float last=0;
  for(int i=0;i<int(sr*.2);++i){last=voice->next(261.6255653,controls,adsr,bank,rate,true);if(envelopeEnd<0&&voice->output.envelopeDone())envelopeEnd=i;if(voice->finished()){voiceEnd=i;break;}}
  check(envelopeEnd>=0&&voiceEnd>envelopeEnd&&voice->draining&&voice->drainSamples<=2048,"Voice retired before tail / did not retire within bound");
  check(std::abs(last)<2.e-7f,"Retirement truncated an audible SRC sample");
  for(int i=0;i<16;++i)check(std::abs(voice->next(261.6255653,controls,adsr,bank,rate,true))<2.e-7f,"Retirement left filter energy");++tails;
  // A note arriving while filters drain must revoke retirement and restart.
  auto reused=std::make_unique<DrWave246Voice>();reused->setDynamics(60,1);
  for(int i=0;i<int(sr*.05);++i)reused->next(261.6255653,controls,adsr,bank,rate);
  for(int i=0;i<int(sr*.2)&&!reused->output.envelopeDone();++i)reused->next(261.6255653,controls,adsr,bank,rate,true);
  check(reused->draining&&!reused->finished(),"No observable filter drain");reused->requestRestart(72,.5f,true);check(!reused->finished(),"Reuse retained stale retirement");
  for(int i=0;i<int(sr*.05);++i)reused->next(523.2511306,controls,adsr,bank,rate);
  check(!reused->finished()&&!reused->restartPending()&&reused->dynamicsNote==72&&reused->output.envelope.level>0,"Reuse during drain lost new note");
  auto engine=std::make_unique<Engine>();Parameters p;p.engine=6;p.hybridVoiceMode=1;p.hybridGlideMs=25;p.effect=0;p.drWave246Adsr=adsr;
  engine->prepare(sr);engine->selectEngine(6);engine->configureHybridPerformance(1,25);
  auto render=[&](double seconds){for(int i=0;i<int(sr*seconds);++i)check(std::isfinite(engine->next(p)),"Non-finite Mono output");};
  engine->noteOn(60,1,1);render(.05);const auto level=engine->visualState(6).envelope;
  engine->noteOn(72,.5f,2);check(engine->visualState(6).envelope==level,"Mono reset level before scheduled ramp");
  engine->noteOff(72,2);render(.05);auto state=engine->visualState(6);check(state.active==1&&state.note==60&&state.channel==1&&state.envelope>0,"Pending-note fallback lost held owner");
  engine->sustainPedal(1,true);engine->noteOff(60,1);engine->noteOn(67,.7f,3);render(.01);engine->noteOff(67,3);render(.02);check(engine->visualState(6).note==60,"Cross-channel pedal fallback failed");
  engine->sustainPedal(1,false);render(.2);check(engine->visualState(6).active==0,"Mono release/drain left voice active");
  engine->noteOn(60,1);render(.01);engine->noteOn(72,1);engine->reset();render(.03);check(engine->visualState(6).active==0,"Panic resurrected pending Mono voice");
 }
 check(tails==6,"Rate coverage");std::cout<<"PASS: Mono request gain/velocity and verified ramp, six-rate SRC/filter retirement and reuse, priority fallback, sustain and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
