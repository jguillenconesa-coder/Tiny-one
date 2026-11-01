// SPDX-License-Identifier: AGPL-3.0-only
#include "Filter246Effect.h"
#include "Phase246.h"
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
int main(){try{
 pocket::Phase246Config config;config.prepare(44100);auto effect=std::make_unique<pocket::Filter246Effect>();const auto c=pocket::delay246Defaults;
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  effect->prepare(rate);check(effect->latency==pocket::Filter246Effect::latencyFor(rate),"Wrong latency");
  std::vector<float> source(16000);for(size_t i=0;i<source.size();++i)source[i]=float(int(i%71)-35)/100;
  for(size_t i=0;i<source.size();++i){const auto x=effect->next(source[i],false,c,config.table,false,true);const float expected=i>=size_t(effect->latency)?source[i-size_t(effect->latency)]:0;check(x==expected,"Bypass is not the exact reported dry delay");}check(effect->underruns==0,"Host resampler underrun");
  effect->reset();double energy=0;
  for(int i=0;i<int(rate);++i){const float input=i<500?0:float(.2*std::sin(2*3.14159265358979323846*220*i/rate));const auto x=effect->next(input,true,c,config.table,false,true);check(std::isfinite(x)&&std::abs(x)<2,"Unstable host effect");if(i<500+effect->latency)check(x==0,"Delay emitted audio before first signal and declared delay");energy+=x*x;}check(energy>.001,"Delay is silent");check(effect->underruns==0,"Active resampler underrun");
  effect->reset();for(int i=0;i<1024;++i)check(effect->next(0,true,c,config.table,false,true)==0,"Delay emits startup sound after panic");
 }
 // At 44.1 kHz, with our DC/startup guards disabled, samples equal direct blocks.
 effect->prepare(44100);effect->removeDC=effect->guardSilence=false;
 std::vector<int16_t> input(8192),expected;for(size_t i=0;i<input.size();++i)input[i]=int16_t(14000*std::sin(double(i)*.15));pocket::reference::Delay246State state;auto memory=std::make_unique<std::array<int32_t,44100>>();std::array<uint32_t,16> meters{};
 for(size_t start=0;start<input.size();start+=64){std::array<int16_t,64> block;std::copy_n(input.begin()+start,64,block.begin());check(pocket::reference::delay246Block(state,c,pocket::reference::delay246Interpolation,*memory,block,meters),"Direct block rejected");expected.insert(expected.end(),block.begin(),block.end());}
 for(size_t i=0;i<input.size()+size_t(effect->latency);++i){const auto x=effect->next(i<input.size()?input[i]/32768.f:0,true,c,config.table,false,true);if(i>=size_t(effect->latency))check(x*32768==expected[i-size_t(effect->latency)],"Native-rate streaming differs from complete-block kernel");}
 // A real first echo must return only after traversing the circular memory.
 pocket::reference::Delay246State impulseState;memory->fill(0);meters.fill(0);const std::array<int16_t,4> impulseControls{{11264,32767,0,32767}};double early=0,echo=0;
 for(int start=0;start<4096;start+=64){std::array<int16_t,64> block{};if(start==0)block[0]=16000;check(pocket::reference::delay246Block(impulseState,impulseControls,pocket::reference::delay246Interpolation,*memory,block,meters),"Impulse rejected");for(int j=0;j<64;++j){const int n=start+j;if(n>0&&n<1000)early+=double(block[size_t(j)])*block[size_t(j)];if(n>=1300&&n<1500)echo+=double(block[size_t(j)])*block[size_t(j)];}}
 check(early==0&&echo>1000,"Delay does not produce a causal returning echo");
 impulseState.reset();memory->fill(0);auto noSend=impulseControls;noSend[3]=0;
 for(int start=0;start<4096;start+=64){std::array<int16_t,64> block{};if(start==0)block[0]=16000;const auto dry=block;check(pocket::reference::delay246Block(impulseState,noSend,pocket::reference::delay246Interpolation,*memory,block,meters)&&block==dry,"Zero-send kernel is not exact dry from a cleared ring");}
 std::cout<<"PASS: Delay exact delayed bypass at seven rates, no underruns, startup/panic, and 8192 native samples equal to direct kernel\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
