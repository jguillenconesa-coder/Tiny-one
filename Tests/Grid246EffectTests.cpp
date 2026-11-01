// SPDX-License-Identifier: AGPL-3.0-only
#include "Filter246Effect.h"
#include "Phase246.h"
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
int main(){try{
 pocket::Phase246Config config;config.prepare(44100);auto effect=std::make_unique<pocket::Filter246Effect>();const auto c=pocket::grid246Defaults;
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  effect->prepare(rate);check(effect->latency==pocket::Filter246Effect::latencyFor(rate),"Wrong latency");
  std::vector<float> source(16000);for(size_t i=0;i<source.size();++i)source[i]=float(int(i%71)-35)/100;
  for(size_t i=0;i<source.size();++i){const auto x=effect->next(source[i],false,c,config.table,false,false,true);const float expected=i>=size_t(effect->latency)?source[i-size_t(effect->latency)]:0;check(x==expected,"Bypass is not the exact reported dry delay");}check(effect->underruns==0,"Host resampler underrun");
  effect->reset();double energy=0;
  for(int i=0;i<int(rate);++i){const float input=i<500?0:float(.2*std::sin(2*3.14159265358979323846*220*i/rate));const auto x=effect->next(input,true,c,config.table,false,false,true);check(std::isfinite(x)&&std::abs(x)<2,"Unstable host effect");if(i<500+effect->latency)check(x==0,"Grid emitted audio before first signal and declared delay");energy+=x*x;}check(energy>.001,"Grid is silent");check(effect->underruns==0,"Active resampler underrun");
  effect->reset();for(int i=0;i<1024;++i)check(effect->next(0,true,c,config.table,false,false,true)==0,"Grid emits startup sound after panic");
 }
 // At 44.1 kHz, with our DC/startup guards disabled, samples equal direct blocks.
 effect->prepare(44100);effect->removeDC=effect->guardSilence=false;
 std::vector<int16_t> input(8192),expected;for(size_t i=0;i<input.size();++i)input[i]=int16_t(14000*std::sin(double(i)*.15));pocket::reference::Grid246State state;auto memory=std::make_unique<pocket::reference::Grid246Memory>();std::array<uint32_t,16> meters{};
 for(size_t start=0;start<input.size();start+=64){std::array<int16_t,64> block;std::copy_n(input.begin()+start,64,block.begin());check(pocket::reference::grid246Block(state,c,config.table,*memory,block,meters),"Direct block rejected");expected.insert(expected.end(),block.begin(),block.end());}
 for(size_t i=0;i<input.size()+size_t(effect->latency);++i){const auto x=effect->next(i<input.size()?input[i]/32768.f:0,true,c,config.table,false,false,true);if(i>=size_t(effect->latency))check(x*32768==expected[i-size_t(effect->latency)],"Native-rate streaming differs from complete-block kernel");}
 // Abrupt axis changes are normalized by the authored adapter before dispatch.
 effect->prepare(48000);effect->removeDC=effect->guardSilence=true;
 for(int i=0;i<96000;++i){auto controls=c;if(i%211<100){controls[0]=1344;controls[1]=16704;}else{controls[0]=16704;controls[1]=1344;}controls[2]=int16_t(i%2?32767:0);controls[3]=int16_t(i%17?32767:0);const auto y=effect->next(float(.6*std::sin(i*.04)),i%991<900,controls,config.table,false,false,true);check(std::isfinite(y)&&std::abs(y)<2,"Grid abrupt resize is unstable");}
 check(effect->underruns==0,"Grid automation underrun");
 // Wet-only output must contain a response after a single excitation.
 pocket::reference::Grid246State plate;memory->fill(0);meters.fill(0);const std::array<int16_t,4> wet{{1344,1344,32767,32767}};double response=0;
 for(int start=0;start<4096;start+=64){std::array<int16_t,64> block{};if(start==0)block[0]=16000;check(pocket::reference::grid246Block(plate,wet,config.table,*memory,block,meters),"Grid impulse rejected");for(size_t j=0;j<64;++j)if(start+int(j)>0)response+=double(block[j])*block[j];}
 check(response>1000,"Grid wet-only plate does not respond to excitation");
 std::cout<<"PASS: Grid exact delayed bypass at seven rates, no underruns, startup/panic, and 8192 native samples equal to direct kernel\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
