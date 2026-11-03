// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Random246.h"
namespace pocket::reference {
inline constexpr std::array<std::array<std::array<int16_t,4>,8>,2> drum246ModulationRanges{{
 {{{{-24320,24832,6208,0}},{{0,32767,0,0}},{{0,32767,0,0}},{{4096,20480,32767,0}},{{8192,24576,16384,0}},{{0,32767,0,0}},{{0,32767,0,0}},{{0,32767,4915,0}}}},
 {{{{-24320,24832,6208,0}},{{0,32767,10485,0}},{{0,32767,10485,0}},{{0,32767,10485,0}},{{0,32767,4915,0}},{{0,32767,10485,0}},{{0,32767,10485,0}},{{0,32767,8192,0}}}}
}};
// Drum dispatch has its own coupled start/end rules and eight direct targets.
// It does not borrow the synth sampler's four-boundary region algorithm.
inline void drum246Modulate(std::array<int16_t,8>& v,int type,size_t target,int16_t modulation) noexcept {
 if(type!=8&&type!=9)return;
 if(type==9||target==0||target==3||target==4||target==7){v[target]=valueVoice246Adjust(v[target],modulation,drum246ModulationRanges[size_t(type-8)][target]);return;}
 const auto fixed=[](int16_t coarse,int16_t fine){return saturate32(int64_t(coarse)*65536+int(fine)*2);};
 const auto high=[](int32_t value){return int16_t(floorDivide(value,65536));};const auto fine=[](int32_t value){return int16_t(uint16_t(value)/2);};
 const auto span=std::max<int16_t>(saturate16(int(v[2])-v[1]),32);
 if(target==1||target==5){
  const auto next=target==1?fixed(saturate16(int(v[1])+multiplyQ15(modulation,span)),v[5]):saturate32(int64_t(fixed(v[1],v[5]))+int(modulation)*8);
  if(next<0){v[1]=0;v[5]=0;}
  else if(next>fixed(v[2],v[6])){v[1]=v[2];v[5]=v[6];}
  else {v[1]=high(next);if(target==5)v[5]=fine(next);}
 }else if(target==2||target==6){
  const auto next=target==2?fixed(saturate16(int(v[2])+multiplyQ15(modulation,span)),v[6]):saturate32(int64_t(fixed(v[2],v[6]))+int(modulation)*8);
  if(next<fixed(v[1],v[5])){v[2]=v[1];v[6]=v[5];}
  else if(next>2147418112){v[2]=32767;v[6]=32767;}
  else {v[2]=high(next);if(target==6)v[6]=fine(next);}
 }
}
inline bool valueDrum246Step(Value246State& state,std::array<uint32_t,8>& display,const std::array<int16_t,8>& c,uint32_t tempo,int /*kind*/,int type,std::array<int16_t,8>& values,const std::array<int16_t,8192>& sine) noexcept {
 const auto colour=uint16_t(c[3])>>11;
 if(c[2]<0||(uint16_t(c[2])>>12)!=0||(type!=8&&type!=9)||colour>=8||c[0]<0||c[0]>16384)return false;
 const auto modulation=value246Advance(state,display,c,tempo,sine);drum246Modulate(values,type,colour,modulation);return true;
}
inline bool randomDrum246Step(Random246State& state,uint32_t& rng,uint32_t& meter,const std::array<int16_t,8>& c,int /*kind*/,int type,std::array<int16_t,8>& values) noexcept {
 if(floorDivide(c[2],2048)!=0||(type!=8&&type!=9))return false;
 const auto amount=random246Advance(state,rng,c);for(size_t i=0;i<8;++i)drum246Modulate(values,type,i,multiplyQ15(state.smooth[i],amount));random246Finish(state,meter,c,amount);return true;
}
}
namespace pocket {
struct Drum246ModulationConfig {
 bool value=false,random=false,free=false,secondary=false;int speed=1000,amount=4000,colour=0;bool envelope=false;
 std::array<int16_t,4> randomControls{};uint32_t tempo=0;double step=0;uint32_t* rng=nullptr;
};
// Own host-rate hold, event order and RNG scheduling. Original drum callbacks
// accept Instrument only; rejected destinations do not advance their DSP state.
struct Drum246ModulationControl {
 reference::Value246State value;reference::Random246State random;std::array<uint32_t,8> display{};
 std::array<int16_t,8> signals{};uint32_t meter=0;double position=1;uint64_t ticks=0;int mode=0;bool pending=true,applied=false;
 void noteOn() noexcept {pending=true;}
 float phase() const noexcept {return float(double(value.phase^0x80000000u)/4294967296.);}
 std::array<int16_t,8> next(const std::array<int16_t,8>& base,int type,const Drum246ModulationConfig& config,const std::array<int16_t,8192>& sine) noexcept {
  const int selected=config.random?2:config.value?1:0;if(selected!=mode){*this={};mode=selected;}
  if(!mode)return base;
  std::array<int16_t,8> c{};
  if(mode==2){std::copy(config.randomControls.begin(),config.randomControls.end(),c.begin());c[1]=std::max<int16_t>(0,c[1]);c[2]=c[2]>=2048?2048:0;c[3]=std::max<int16_t>(0,c[3]);}
  else c={{int16_t(std::clamp(config.speed,0,16384)),int16_t(std::clamp(config.amount,-32767,32767)),int16_t((config.envelope?4096:0)|(config.free?2048:0)),int16_t((std::clamp(config.colour,0,3)+(config.secondary?4:0))*2048),0,0,0,0}};
  if(pending){if(mode==2)random.noteOn(c[3]);else value.noteOn(c);pending=false;}
  if(position>=1){auto targets=base;
   if(mode==2){const auto amount=reference::multiplyQ15(random.gate,c[0]);applied=config.rng&&reference::randomDrum246Step(random,*config.rng,meter,c,0,type,targets);if(applied)for(size_t i=0;i<8;++i)signals[i]=reference::multiplyQ15(random.smooth[i],amount);}
   else {applied=reference::valueDrum246Step(value,display,c,config.tempo,0,type,targets,sine);if(applied)signals[0]=int16_t(display[6]);}
   position-=1;++ticks;
  }
  position+=config.step;
  if(!applied||(mode==2?c[2]>=2048:(uint16_t(c[2])>>12)!=0)){applied=false;return base;}
  auto result=base;if(mode==2)for(size_t i=0;i<8;++i)reference::drum246Modulate(result,type,i,signals[i]);else reference::drum246Modulate(result,type,size_t(uint16_t(c[3])>>11),signals[0]);return result;
 }
};
}
