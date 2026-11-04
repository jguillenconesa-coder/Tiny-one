// SPDX-License-Identifier: AGPL-3.0-only
#include "Filter246Effect.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket;
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static std::array<int16_t,4> controls(int fx){
 switch(fx){case 1:return nitro246Defaults;case 2:return delay246Defaults;case 3:return grid246Defaults;case 4:return phone246Defaults;case 5:return spring246Defaults;case 6:return cwo246Defaults;default:return filter246Defaults;}
}
static float tick(Filter246Effect& bus,float input,int fx,const std::array<int16_t,8192>& sine,const Midi246FxAudio* midi){
 return bus.next(input,fx>=0,controls(fx),sine,fx==1,fx==2,fx==3,fx==4,fx==5,fx==6,midi);
}
int main(){try{
 std::array<int16_t,8192> sine{};for(size_t i=0;i<sine.size();++i)sine[i]=int16_t(std::lround(32767*std::sin(i*6.283185307179586/sine.size())));
 const std::array<int,4> routes{{17,0,0,0}};
 check(Midi246FxAudio(routes,{{64,64,64,64}}).apply(23456)==0,"CC64 does not close bipolar FX input");
 check(Midi246FxAudio(routes,{{0,64,64,64}}).apply(-32768)==32767,"FX bipolar inversion saturation differs");
 // Independent bus-placement check at 44.1 kHz: CC0 exactly inverts the already
 // quantized input. The unmodulated bus fed that inversion must match samplewise.
 for(int fx=0;fx<7;++fx){
  auto actual=std::make_unique<Filter246Effect>(),expected=std::make_unique<Filter246Effect>();actual->prepare(44100);expected->prepare(44100);
  const Midi246FxAudio invert(routes,{{0,64,64,64}});
  // Dry inputs differ in the two arrangements, so clean+(wet-clean)*mix may
  // round one float ULP apart even at mix=1; kernel history must remain exact.
  for(int n=0;n<4096;++n){const int q=n<2048?int(12000*std::sin(n*.13)):0;const float a=tick(*actual,q/32768.f,fx,sine,&invert),b=tick(*expected,std::clamp(-q,-32768,32767)/32768.f,fx,sine,nullptr);check(std::abs(a-b)<1.e-7f,"MIDI audio is not before the selected FX kernel");if(actual->cursor==0)check(actual->history==expected->history,"FX Q15 kernel history differs after input inversion");}
 }
 // Closing the FX input must not erase previously excited delay memories.
 {auto bus=std::make_unique<Filter246Effect>();bus->prepare(44100);float tail=0;
  for(int n=0;n<16384;++n){const Midi246FxAudio m(routes,{{n<2048?0u:64u,64,64,64}});const float v=tick(*bus,float(.3*std::sin(n*.11)),2,sine,&m);if(n>2500)tail=std::max(tail,std::abs(v));}
  check(tail>1.e-5f,"CC64 erased the stored FX tail instead of closing its input");
 }
 auto render=[&](double rate,int fx,int route,int partition,bool enabled){
  auto bus=std::make_unique<Filter246Effect>();bus->prepare(rate);std::vector<float> result;result.reserve(8192);
  for(int start=0;start<8192;start+=partition)for(int n=start;n<std::min(8192,start+partition);++n){
   std::array<int,4> r{{route,18,0,0}};std::array<uint32_t,4> cc{{n<1537?127u:n<4099?0u:64u,n<3073?100u:127u,64,64}};
   if(n>=6145)r={{0,0,0,0}};const Midi246FxAudio m(r,cc);
   const float input=n<5500?float(.2*std::sin(n*.071)+.1*std::sin(n*.137)):0;
   const float value=tick(*bus,input,fx,sine,enabled?&m:nullptr);check(std::isfinite(value)&&std::abs(value)<8,"MIDI FX audio unstable");result.push_back(value);
  }
  check(bus->underruns==0&&bus->latency==Filter246Effect::latencyFor(rate),"MIDI audio changes bus latency/coverage");
  bus->reset();check(bus->cursor==0&&bus->completed==-1,"MIDI bus reset retains history");return result;
 };
 size_t cases=0;
 for(double rate:{32000.,44100.,48000.,88200.,96000.,176400.,192000.})for(int fx=-1;fx<7;++fx)for(int route:{17,18}){
  const auto a=render(rate,fx,route,64,true),b=render(rate,fx,route,257,true),dry=render(rate,fx,route,64,false);
  check(a==b,"FX audio CC/route changes depend on host partition");
  if(fx<0)check(a==dry,"MIDI FX audio modifies bypass");else check(a!=dry,"MIDI FX audio does not reach selected FX");++cases;
 }
 std::cout<<"PASS: seven native FX input inversions and "<<cases<<" audio route/rate/bypass cases, sequential streams, timestamped CC/route changes, fixed PDC, retained delay tail at CC64, 64/257 partition and reset\n";
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
