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
 for(size_t k=0;k<4;++k){check(pocket::reference::valueVoice246Ranges[6][k][0]==pocket::dna246Min[k]&&pocket::reference::valueVoice246Ranges[6][k][1]==pocket::dna246Max[k],"DNA host ranges diverge from verified descriptors");}
 {pocket::reference::Dna246Context a,b;a.prepare();b.prepare();pocket::Dna246Voice voice;pocket::Phase246Config rate;rate.prepare(48000);for(auto& f:rate.filters)f={1,0,0,0,0};uint32_t ar=123,br=123;const std::array<int16_t,4> env{{0,64,32767,16383}};b.pool[0]={};b.pool[0].construct();b.pool[0].reset();
  for(int n=0;n<32;++n){if(n==16)b.pool[0].release();std::array<int16_t,128> audio{};const auto pitch=uint32_t(440./88200*4294967296.+.5);check(pocket::reference::dna246Block(b,0,pocket::dna246Defaults,pitch,br,audio),"Direct DNA rejected");for(int i=0;i<128;++i)check(std::isfinite(voice.source(440,pocket::dna246Defaults,env,a,0,ar,rate,n>=16)),"DNA source nonfinite");check(ar==br&&std::memcmp(a.pool.data(),b.pool.data(),192)==0&&a.feedback==b.feedback&&a.wave==b.wave&&audio==voice.block,"DNA adapter diverges from direct callback");}}
 for(bool envelope:{false,true})for(int colour=0;colour<4;++colour){
  auto render=[&](bool enabled,int amount){pocket::Parameters p;p.engine=8;p.effect=0;p.volume=1;p.dna246Adsr={2000,8000,16000,10000};p.valueVoice246=enabled;p.valueVoice246Envelope=envelope;p.valueVoice246Colour=colour;p.valueVoice246Speed=12288;p.valueVoice246Amount=amount;engine->prepare(48000);engine->next(p);engine->noteOn(57,.8f);std::vector<float> samples;for(int n=0;n<24000;++n){if(n==10000)engine->noteOff(57);const auto x=engine->next(p);check(std::isfinite(x),"DNA Value nonfinite");samples.push_back(x);}return samples;};
  const auto baseline=render(false,0),neutral=render(true,0),modulated=render(true,24000);check(baseline==neutral,"Zero-depth DNA Value changed audio");double difference=0;for(size_t n=0;n<baseline.size();++n)difference+=std::abs(baseline[n]-modulated[n]);if(difference<=.001)std::cerr<<"Value envelope="<<envelope<<" colour="<<colour<<" difference="<<difference<<"\n";check(difference>.001,"DNA Value colour/routing inaudible");
 }

 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  pocket::Parameters p;p.engine=8;p.effect=0;p.volume=1;p.dna246Adsr={0,64,32767,16383};
  engine->prepare(rate);engine->next(p);for(int n=48;n<56;++n)engine->noteOn(n,.8f);
  check(engine->visualState(8).active==6,"DNA pool is not six voices");double energy=0;
  for(int n=0;n<int(rate*.3);++n){const auto x=engine->next(p);check(std::isfinite(x)&&std::abs(x)<2,"DNA unstable");energy+=x*x;}
  check(energy>.001,"DNA silent");engine->allNotesOff();for(int n=0;n<int(rate*2);++n)engine->next(p);check(engine->visualState(8).active==0,"DNA release stuck");
  engine->reset();for(int n=0;n<256;++n)check(engine->next(p)==0,"DNA panic retained sound");
  for(int corner=0;corner<16;++corner){engine->prepare(rate);engine->next(p);for(size_t k=0;k<4;++k)p.dna246Controls[k]=int16_t(corner&(1<<k)?pocket::dna246Max[k]:pocket::dna246Min[k]);engine->noteOn(corner%2?0:127,1);for(int n=0;n<2048;++n)check(std::isfinite(engine->next(p)),"DNA extremes unstable");}
 }
 std::cout<<"PASS: DNA native-block parity/reset/release/RNG/memory, six voices, seven rates, controls/MIDI extremes and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
