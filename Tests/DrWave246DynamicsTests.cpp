// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include "Voice246Activation.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<float> render(double rate,int policy,float velocity){
 auto e=std::make_unique<pocket::Engine>();pocket::Parameters p;p.engine=6;p.hybridVoiceMode=policy;p.hybridGlideMs=25;p.effect=0;p.drWave246Adsr[3]=16383;
 e->prepare(rate);e->selectEngine(6);e->configureHybridPerformance(policy,25);
 std::vector<float> audio;double energy=0;
 for(int frame=0;frame<int(rate*.5);++frame){
  if(frame==0)e->noteOn(48,velocity,1);
  if(frame==100)e->noteOn(72,velocity,2);
  if(frame==200)e->sustainPedal(2,true);
  if(frame==300)e->noteOff(72,2);
  if(frame==400)e->sustainPedal(2,false);
  if(frame==500)for(int n=55;n<63;++n)e->noteOn(n,velocity,3);
  if(frame==1000)e->allNotesOff();
  float s=e->next(p);check(std::isfinite(s)&&std::abs(s)<1,"Unstable DrWave dynamics");audio.push_back(s);energy+=s*s;
 }
 check(energy>1e-5,"Dynamics stream silent");check(e->visualState(6).active==0,"Dynamics stream left notes active");return audio;
}
int main(){try{
 for(int note=0;note<128;++note)for(int code=0;code<128;++code){
  pocket::DrWave246Voice v;v.output.envelope.gain=0xa5a50000;v.setDynamics(note,float(code)/127);
  pocket::reference::Voice246Registers registers;
  check(pocket::reference::voice246PrepareRegisters(registers,{uint32_t(note),uint32_t(code),0,0},0x40000000,0),"Reference activation rejected");
  check(v.output.envelopeVolume==registers.velocity[0],"Host velocity bridge differs from verified registers");
  check(v.output.envelope.gain==(0xa5a50000u|uint32_t(note==0?32767:32768-note*128)),"Note gain/upper word preservation");
 }
 // Both floats map to MIDI velocity 64. Exact equality catches any second
 // floating velocity multiplier after the native envelope path.
 size_t streams=0;
 for(double rate:{44100.,48000.,96000.})for(int policy:{0,1,2}){
  auto a=render(rate,policy,.5001f),b=render(rate,policy,.502f),quiet=render(rate,policy,32.f/127);
  check(a==b,"DrWave velocity applied twice / float bypassed native code");
  double loudEnergy=0,quietEnergy=0;for(size_t i=0;i<a.size();++i){loudEnergy+=a[i]*a[i];quietEnergy+=quiet[i]*quiet[i];}
  check(quietEnergy/loudEnergy>.24&&quietEnergy/loudEnergy<.26,"Native velocity response ratio");++streams;
 }
 // New volume/gain words are latched together on the next internal block.
 pocket::Phase246Voice v;pocket::Phase246Config identity;std::array<int16_t,4> adsr{{0,0,32767,16383}};
 v.shapeSample(1,identity,&adsr);const auto prior=v.envelopeGain;v.envelopeVolume=8192;v.envelope.gain=20000;
 for(int i=1;i<128;++i){v.shapeSample(1,identity,&adsr);check(v.envelopeGain==prior,"Gain changed inside block");}
 v.shapeSample(1,identity,&adsr);check(v.envelopeGain==int32_t(pocket::reference::multiplyQ15(8192,20000))*65536,"Gain did not latch on next block");
 std::cout<<"PASS: 16384 velocity/register bridges, note gain, nine performance streams with exact MIDI-bucket equality, sustain/steal/release and block gain latch\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
