// SPDX-License-Identifier: AGPL-3.0-only
#include "Filter246Effect.h"
#include "Phase246.h"
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
int main(){try{
 pocket::Phase246Config config;config.prepare(44100);auto effect=std::make_unique<pocket::Filter246Effect>();const auto c=pocket::phone246Defaults;
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  effect->prepare(rate);check(effect->latency==pocket::Filter246Effect::latencyFor(rate),"Wrong latency");
  std::vector<float> source(16000);for(size_t i=0;i<source.size();++i)source[i]=float(int(i%71)-35)/100;
  for(size_t i=0;i<source.size();++i){const auto x=effect->next(source[i],false,c,config.table,false,false,false,true);const float expected=i>=size_t(effect->latency)?source[i-size_t(effect->latency)]:0;check(x==expected,"Bypass is not the exact reported dry delay");}check(effect->underruns==0,"Host resampler underrun");
  effect->reset();double energy=0;
  for(int i=0;i<int(rate);++i){const float input=i<500?0:float(.2*std::sin(2*3.14159265358979323846*220*i/rate));const auto x=effect->next(input,true,c,config.table,false,false,false,true);check(std::isfinite(x)&&std::abs(x)<2,"Unstable host effect");if(i<500+effect->latency)check(x==0,"Phone emitted audio before first signal and declared delay");energy+=x*x;}check(energy>.001,"Phone is silent");check(effect->underruns==0,"Active resampler underrun");
  effect->reset();for(int i=0;i<1024;++i)check(effect->next(0,true,c,config.table,false,false,false,true)==0,"Phone emits startup sound after panic");
 }
 // At 44.1 kHz, with our DC/startup guards disabled, samples equal direct blocks.
 effect->prepare(44100);effect->removeDC=effect->guardSilence=false;
 std::vector<int16_t> input(8192),expected;for(size_t i=0;i<input.size();++i)input[i]=int16_t(14000*std::sin(double(i)*.15));pocket::reference::Phone246State state;auto memory=std::make_unique<pocket::reference::Phone246Memory>();std::array<uint32_t,16> meters{};
 for(size_t start=0;start<input.size();start+=64){std::array<int16_t,64> block;std::copy_n(input.begin()+start,64,block.begin());check(pocket::reference::phone246Block(state,c,pocket::reference::delay246Interpolation,pocket::reference::saturate32(int64_t(2147483647)-pocket::reference::filter246Frequency[0]),pocket::reference::phone246LowCoefficient,*memory,block,meters),"Direct block rejected");expected.insert(expected.end(),block.begin(),block.end());}
 for(size_t i=0;i<input.size()+size_t(effect->latency);++i){const auto x=effect->next(i<input.size()?input[i]/32768.f:0,true,c,config.table,false,false,false,true);if(i>=size_t(effect->latency))check(x*32768==expected[i-size_t(effect->latency)],"Native-rate streaming differs from complete-block kernel");}
 // Wet-only output must respond after the initially silent playback segment.
 effect->prepare(44100);effect->removeDC=effect->guardSilence=true;auto wet=c;wet[3]=32767;double energy=0;
 for(int i=0;i<44100;++i){const auto y=effect->next(float(.2*std::sin(i*.06)),true,wet,config.table,false,false,false,true);if(i>4410)energy+=y*y;}
 check(energy>.001,"Phone wet-only signal is silent");
 // All controls, including physical maximum frame size and fast resampling.
 for(double rate:{8000.,48000.,192000.}){effect->prepare(rate);for(int i=0;i<int(rate);++i){auto controls=c;controls[0]=int16_t(i%211<100?204:20480);controls[1]=int16_t(i%311<150?3072:17408);controls[2]=int16_t(i%411<200?1536:16896);controls[3]=32767;const auto y=effect->next(float(i%701<300?-.99999:.6*std::sin(i*.04)),i%991<900,controls,config.table,false,false,false,true);check(std::isfinite(y)&&std::abs(y)<2,"Phone extreme control automation unstable");}check(effect->underruns==0,"Phone control automation underrun");}
 std::cout<<"PASS: Phone exact delayed bypass at seven rates, no underruns, startup/panic, and 8192 native samples equal to direct kernel\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
