// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
 auto engine=std::make_unique<pocket::Engine>();
 for(size_t k=0;k<4;++k){check(pocket::reference::valueVoice246Ranges[8][k][0]==0&&pocket::reference::valueVoice246Ranges[8][k][1]==32767,"Dsynth host ranges diverge from verified descriptors");}
 {pocket::Dsynth246Voice voice;pocket::Phase246Config rate;rate.prepare(48000);pocket::reference::Dsynth246State state;state.construct();state.bind(0);pocket::reference::Dsynth246Context a{},b{};a.initialize();b.initialize();b.reset(state,pocket::dsynth246Defaults);pocket::reference::Adsr246State envState;envState.noteOn();pocket::reference::Adsr246Noise noise;const std::array<int16_t,4> env{{0,64,32767,16383}};
  for(int n=0;n<32;++n){if(n==16){state.release();envState.noteOff(env[2]);}std::array<uint32_t,128> pitch{},derived{};pitch.fill(uint32_t(440./88200*4294967296.+.5));std::array<int16_t,128> audio{},gain{},second{};pocket::reference::adsr246Block(envState,env,32767,noise,gain);pocket::reference::dsynth246Block(state,b,pocket::dsynth246Defaults,pitch,gain,rate.table,audio,derived,second);for(int i=0;i<128;++i)check(std::isfinite(voice.source(440,pocket::dsynth246Defaults,env,a,0,rate,n>=16)),"Dsynth source nonfinite");check(std::memcmp(&state,&voice.state,72)==0&&std::memcmp(&a,&b,824)==0&&audio==voice.block&&gain==voice.gain&&derived==voice.derived&&second==voice.second,"Dsynth adapter differs from direct gain-before-filter callback");}}

 for(bool envelope:{false,true})for(int colour=0;colour<(envelope?4:8);++colour){
  auto render=[&](bool enabled,int amount){pocket::Parameters p;p.engine=10;p.effect=0;p.volume=1;p.dsynth246Adsr={2000,8000,16000,10000};p.valueVoice246=enabled;p.valueVoice246Envelope=envelope;p.valueVoice246Colour=colour%4;p.dsynth246Secondary=colour>=4;p.valueVoice246Speed=12288;p.valueVoice246Amount=amount;engine->prepare(48000);engine->next(p);engine->noteOn(57,.8f);std::vector<float> samples;for(int n=0;n<24000;++n){if(n==10000)engine->noteOff(57);const auto x=engine->next(p);check(std::isfinite(x),"Dsynth Value nonfinite");samples.push_back(x);}return samples;};
  const auto baseline=render(false,0),neutral=render(true,0),modulated=render(true,24000);check(baseline==neutral,"Zero-depth Dsynth Value changed audio");double difference=0;for(size_t n=0;n<baseline.size();++n)difference+=std::abs(baseline[n]-modulated[n]);if(difference<=.001)std::cerr<<"Value envelope="<<envelope<<" colour="<<colour<<" difference="<<difference<<"\n";check(difference>.001,"Dsynth Value colour/routing inaudible");
 }

 for(bool envelope:{false,true}){auto render=[&](bool enabled,int amount){pocket::Parameters p;p.engine=10;p.effect=0;p.volume=1;p.random246=enabled;p.random246Controls={int16_t(amount),32767,int16_t(envelope?2048:0),24000};engine->prepare(48000);engine->next(p);engine->noteOn(57,.8f);std::vector<float> out;for(int n=0;n<16000;++n)out.push_back(engine->next(p));return out;};const auto base=render(false,0),zero=render(true,0),active=render(true,20000);check(base==zero,"Zero Random changed Dsynth");double diff=0;for(size_t n=0;n<base.size();++n)diff+=std::abs(base[n]-active[n]);check(diff>.001,"Dsynth Random routing inaudible");}
 for(int shape=0;shape<5;++shape){auto render=[&](bool enabled){pocket::Parameters p;p.engine=10;p.effect=0;p.volume=1;p.tremolo246=enabled;p.tremolo246Shape=shape;p.tremolo246Controls={12288,14000,16000,0};engine->prepare(48000);engine->next(p);engine->noteOn(57,.8f);std::vector<float> out;for(int n=0;n<16000;++n)out.push_back(engine->next(p));return out;};const auto base=render(false),active=render(true);double diff=0;for(size_t n=0;n<base.size();++n){check(std::isfinite(active[n]),"Dsynth Tremolo nonfinite");diff+=std::abs(base[n]-active[n]);}check(diff>.001,"Dsynth Tremolo inaudible");}

 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  pocket::Parameters p;p.engine=10;p.effect=0;p.volume=1;p.dsynth246Adsr={0,64,32767,16383};
  engine->prepare(rate);engine->next(p);for(int n=48;n<56;++n)engine->noteOn(n,.8f);
  check(engine->visualState(10).active==6,"Dsynth pool is not six voices");double energy=0;
  for(int n=0;n<int(rate*.3);++n){const auto x=engine->next(p);check(std::isfinite(x)&&std::abs(x)<2,"Dsynth unstable");energy+=x*x;}
  check(energy>.001,"Dsynth silent");engine->allNotesOff();for(int n=0;n<int(rate*2);++n)engine->next(p);check(engine->visualState(10).active==0,"Dsynth release stuck");
  engine->reset();for(int n=0;n<256;++n)check(engine->next(p)==0,"Dsynth panic retained sound");
  for(int corner=0;corner<256;++corner){engine->prepare(rate);engine->next(p);for(size_t k=0;k<8;++k)p.dsynth246Controls[k]=int16_t(corner&(1<<k)?32767:0);engine->noteOn(corner%2?0:127,1);for(int n=0;n<256;++n)check(std::isfinite(engine->next(p)),"Dsynth extremes unstable");}
 }
 std::cout<<"PASS: Dsynth native-block parity/reset/release/state, Value/Random/Tremolo, six voices, seven rates, controls/MIDI extremes and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
