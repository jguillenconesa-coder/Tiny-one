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
 for(size_t k=0;k<4;++k){check(pocket::reference::valueVoice246Ranges[7][k][0]==pocket::voltage246Min[k]&&pocket::reference::valueVoice246Ranges[7][k][1]==pocket::voltage246Max[k],"Voltage host ranges diverge from verified descriptors");}
 {pocket::Voltage246Voice voice;pocket::Phase246Config rate;rate.prepare(48000);pocket::reference::Voltage246State state;state.construct();state.reset();const std::array<int16_t,4> env{{0,64,32767,16383}};
  for(int n=0;n<32;++n){if(n==16)state.release();std::array<uint32_t,128> pitch{};pitch.fill(uint32_t(440./88200*4294967296.+.5));std::array<int16_t,128> audio{};std::array<int16_t,32> display{};pocket::reference::voltage246Block(state,pocket::voltage246Defaults,pitch,rate.table,audio,display);for(int i=0;i<128;++i)check(std::isfinite(voice.source(440,pocket::voltage246Defaults,env,rate,n>=16)),"Voltage source nonfinite");check(std::memcmp(&state,&voice.state,52)==0&&audio==voice.block&&display==voice.display,"Voltage adapter diverges from direct callback");}}

 for(bool envelope:{false,true})for(int colour=0;colour<4;++colour){
  auto render=[&](bool enabled,int amount){pocket::Parameters p;p.engine=9;p.effect=0;p.volume=1;p.voltage246Adsr={2000,8000,16000,10000};p.valueVoice246=enabled;p.valueVoice246Envelope=envelope;p.valueVoice246Colour=colour;p.valueVoice246Speed=12288;p.valueVoice246Amount=amount;engine->prepare(48000);engine->next(p);engine->noteOn(57,.8f);std::vector<float> samples;for(int n=0;n<24000;++n){if(n==10000)engine->noteOff(57);const auto x=engine->next(p);check(std::isfinite(x),"Voltage Value nonfinite");samples.push_back(x);}return samples;};
  const auto baseline=render(false,0),neutral=render(true,0),modulated=render(true,24000);check(baseline==neutral,"Zero-depth Voltage Value changed audio");double difference=0;for(size_t n=0;n<baseline.size();++n)difference+=std::abs(baseline[n]-modulated[n]);if(difference<=.001)std::cerr<<"Value envelope="<<envelope<<" colour="<<colour<<" difference="<<difference<<"\n";check(difference>.001,"Voltage Value colour/routing inaudible");
 }

 for(bool envelope:{false,true}){auto render=[&](bool enabled,int amount){pocket::Parameters p;p.engine=9;p.effect=0;p.volume=1;p.random246=enabled;p.random246Controls={int16_t(amount),32767,int16_t(envelope?2048:0),24000};engine->prepare(48000);engine->next(p);engine->noteOn(57,.8f);std::vector<float> out;for(int n=0;n<16000;++n)out.push_back(engine->next(p));return out;};const auto base=render(false,0),zero=render(true,0),active=render(true,20000);check(base==zero,"Zero Random changed Voltage");double diff=0;for(size_t n=0;n<base.size();++n)diff+=std::abs(base[n]-active[n]);check(diff>.001,"Voltage Random routing inaudible");}
 for(int shape=0;shape<5;++shape){auto render=[&](bool enabled){pocket::Parameters p;p.engine=9;p.effect=0;p.volume=1;p.tremolo246=enabled;p.tremolo246Shape=shape;p.tremolo246Controls={12288,14000,16000,0};engine->prepare(48000);engine->next(p);engine->noteOn(57,.8f);std::vector<float> out;for(int n=0;n<16000;++n)out.push_back(engine->next(p));return out;};const auto base=render(false),active=render(true);double diff=0;for(size_t n=0;n<base.size();++n){check(std::isfinite(active[n]),"Voltage Tremolo nonfinite");diff+=std::abs(base[n]-active[n]);}check(diff>.001,"Voltage Tremolo inaudible");}

 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  pocket::Parameters p;p.engine=9;p.effect=0;p.volume=1;p.voltage246Adsr={0,64,32767,16383};
  engine->prepare(rate);engine->next(p);for(int n=48;n<56;++n)engine->noteOn(n,.8f);
  check(engine->visualState(9).active==6,"Voltage pool is not six voices");double energy=0;
  for(int n=0;n<int(rate*.3);++n){const auto x=engine->next(p);check(std::isfinite(x)&&std::abs(x)<2,"Voltage unstable");energy+=x*x;}
  check(energy>.001,"Voltage silent");engine->allNotesOff();for(int n=0;n<int(rate*2);++n)engine->next(p);check(engine->visualState(9).active==0,"Voltage release stuck");
  engine->reset();for(int n=0;n<256;++n)check(engine->next(p)==0,"Voltage panic retained sound");
  for(int corner=0;corner<16;++corner){engine->prepare(rate);engine->next(p);for(size_t k=0;k<4;++k)p.voltage246Controls[k]=int16_t(corner&(1<<k)?pocket::voltage246Max[k]:pocket::voltage246Min[k]);engine->noteOn(corner%2?0:127,1);for(int n=0;n<2048;++n)check(std::isfinite(engine->next(p)),"Voltage extremes unstable");}
 }
 std::cout<<"PASS: Voltage native-block parity/reset/release/state, Value/Random/Tremolo, six voices, seven rates, controls/MIDI extremes and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
