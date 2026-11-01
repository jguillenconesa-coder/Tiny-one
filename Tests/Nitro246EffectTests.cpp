// SPDX-License-Identifier: AGPL-3.0-only
#include "Filter246Effect.h"
#include "Phase246.h"
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
int main(){try{
 pocket::Phase246Config config;config.prepare(44100);auto effect=std::make_unique<pocket::Filter246Effect>();const auto c=pocket::nitro246Defaults;
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  effect->prepare(rate);check(effect->latency==pocket::Filter246Effect::latencyFor(rate),"Wrong latency");
  std::vector<float> source(16000);for(size_t i=0;i<source.size();++i)source[i]=float(int(i%71)-35)/100;
  for(size_t i=0;i<source.size();++i){const auto x=effect->next(source[i],false,c,config.table,true);const float expected=i>=size_t(effect->latency)?source[i-size_t(effect->latency)]:0;check(x==expected,"Bypass is not the exact reported dry delay");}check(effect->underruns==0,"Host resampler underrun");
  effect->reset();double energy=0;
  for(int i=0;i<int(rate);++i){const float input=i<500?0:float(.2*std::sin(2*3.14159265358979323846*220*i/rate));const auto x=effect->next(input,true,c,config.table,true);check(std::isfinite(x)&&std::abs(x)<2,"Unstable host effect");if(i<500+effect->latency)check(x==0,"Nitro hybrid emitted audio before first signal and declared delay");energy+=x*x;}check(energy>.001,"Nitro hybrid is silent");check(effect->underruns==0,"Active resampler underrun");
  effect->reset();for(int i=0;i<1024;++i)check(effect->next(0,true,c,config.table,true)==0,"Nitro hybrid emits startup sound after panic");
 }
 // At 44.1 kHz, with our DC/startup guards disabled, samples equal direct blocks.
 effect->prepare(44100);effect->removeDC=effect->guardSilence=false;
 std::vector<int16_t> input(8192),expected;for(size_t i=0;i<input.size();++i)input[i]=int16_t(14000*std::sin(double(i)*.15));pocket::reference::Nitro246State state;std::array<uint32_t,16> meters{};
 for(size_t start=0;start<input.size();start+=64){std::array<int16_t,64> block;std::copy_n(input.begin()+start,64,block.begin());check(pocket::reference::nitro246HybridBlock(state,c,pocket::reference::nitro246HybridMix,pocket::reference::filter246Frequency,config.table,block,meters),"Direct block rejected");expected.insert(expected.end(),block.begin(),block.end());}
 for(size_t i=0;i<input.size()+size_t(effect->latency);++i){const auto x=effect->next(i<input.size()?input[i]/32768.f:0,true,c,config.table,true);if(i>=size_t(effect->latency))check(x*32768==expected[i-size_t(effect->latency)],"Native-rate streaming differs from complete-block kernel");}
 // Independent frequency-response checks for the two authored hybrid paths.
 auto energy=[&](std::array<int16_t,4> controls,double hz){effect->prepare(44100);effect->removeDC=effect->guardSilence=true;double e=0;for(int i=0;i<24000;++i){const auto y=effect->next(float(.02*std::sin(2*3.14159265358979323846*hz*i/44100)),true,controls,config.table,true);if(i>12000)e+=y*y;}return e;};
 const auto open=std::array<int16_t,4>{{64,0,0,16448}};
 auto high=open;high[0]=10000;auto low=open;low[3]=2000;
 check(energy(high,220)<energy(open,220)*.3,"Frequency 1 does not reject low frequencies");
 check(energy(low,6000)<energy(open,6000)*.3,"Frequency 2 does not reject high frequencies");
 for(double rate:{8000.,48000.,192000.}){effect->prepare(rate);for(int i=0;i<int(rate);++i){auto controls=c;controls[1]=int16_t(i%2?32767:-32768);controls[2]=20643;const auto y=effect->next(float(.7*std::sin(i*.03)),i%211<150,controls,config.table,true);check(std::isfinite(y)&&std::abs(y)<2,"Hybrid extreme control automation unstable");}check(effect->underruns==0,"Hybrid extreme automation underrun");}
 std::cout<<"PASS: Nitro hybrid exact delayed bypass at seven rates, no underruns, startup/panic, and 8192 native samples equal to direct kernel\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
