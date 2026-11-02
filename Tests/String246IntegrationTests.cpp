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
 for(size_t k=0;k<4;++k){check(pocket::reference::valueVoice246Ranges[5][k][0]==pocket::string246Min[k]&&pocket::reference::valueVoice246Ranges[5][k][1]==pocket::string246Max[k],"String host ranges diverge from verified descriptors");}
 for(bool envelope:{false,true})for(int colour=0;colour<4;++colour){
  auto render=[&](bool enabled,int amount){pocket::Parameters p;p.engine=2;p.effect=0;p.volume=1;p.string246Adsr={2000,8000,16000,10000};p.valueVoice246=enabled;p.valueVoice246Envelope=envelope;p.valueVoice246Colour=colour;p.valueVoice246Speed=12288;p.valueVoice246Amount=amount;engine->prepare(48000);engine->next(p);engine->noteOn(57,.8f);std::vector<float> samples;for(int n=0;n<24000;++n){if(n==10000)engine->noteOff(57);const auto x=engine->next(p);check(std::isfinite(x),"String Value nonfinite");samples.push_back(x);}return samples;};
  const auto baseline=render(false,0),neutral=render(true,0),modulated=render(true,24000);check(baseline==neutral,"Zero-depth String Value changed audio");double difference=0;for(size_t n=0;n<baseline.size();++n)difference+=std::abs(baseline[n]-modulated[n]);if(difference<=.001)std::cerr<<"Value envelope="<<envelope<<" colour="<<colour<<" difference="<<difference<<"\n";check(difference>.001,"String Value colour/routing inaudible");
 }

 // At the core clock and with output filtering bypassed, the source follows
 // exact original blocks, including reset/release, memory and shared RNG.
 {pocket::String246Voice voice;pocket::reference::String246State direct;pocket::reference::String246Memory memory{},expected{};pocket::Phase246Config rate;rate.prepare(48000);for(auto& f:rate.filters)f={1,0,0,0,0};uint32_t a=123,b=123;pocket::reference::string246Reset(direct,b);const std::array<int16_t,4> adsr{{0,64,32767,16383}};
  for(int block=0;block<32;++block){const bool release=block>=16;if(block==16)direct.release();std::array<int16_t,128> audio{};const auto pitch=uint32_t(440./88200*4294967296.+.5);check(pocket::reference::string246Block(direct,expected,pocket::string246Defaults,pitch,b,audio),"Direct String rejected");for(int i=0;i<128;++i)check(std::isfinite(voice.source(440,pocket::string246Defaults,adsr,memory,a,rate,release)),"String source nonfinite");check(a==b&&std::memcmp(&direct,&voice.state,32)==0&&memory==expected&&audio==voice.block,"String host source diverges from original blocks");}}
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  pocket::Parameters p;p.engine=2;p.effect=0;p.volume=1;p.string246Adsr={0,64,32767,16383};
  engine->prepare(rate);engine->next(p);for(int n=48;n<56;++n)engine->noteOn(n,.8f);
  check(engine->visualState(2).active==6,"String pool is not six voices");double energy=0;
  for(int n=0;n<int(rate*.3);++n){const auto x=engine->next(p);check(std::isfinite(x)&&std::abs(x)<2,"String unstable");energy+=x*x;}
  check(energy>.001,"String silent");engine->allNotesOff();for(int n=0;n<int(rate*2);++n)engine->next(p);check(engine->visualState(2).active==0,"String release stuck");
  engine->reset();for(int n=0;n<256;++n)check(engine->next(p)==0,"String panic retained sound");
  for(int corner=0;corner<16;++corner){engine->prepare(rate);engine->next(p);for(size_t k=0;k<4;++k)p.string246Controls[k]=int16_t(corner&(1<<k)?pocket::string246Max[k]:pocket::string246Min[k]);engine->noteOn(corner%2?0:127,1);for(int n=0;n<2048;++n)check(std::isfinite(engine->next(p)),"String extremes unstable");}
 }
 std::cout<<"PASS: String native-block parity/reset/release/RNG/memory, six voices, seven rates, controls/MIDI extremes and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
