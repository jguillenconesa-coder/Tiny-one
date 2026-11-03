// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Midi246.h"
namespace pocket {
struct Drum246ModulationConfig {
 bool midi=false;std::array<int,4> midiRoutes{{1,2,3,4}};std::array<uint32_t,4> midiCC{{64,64,64,64}};
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
  const int selected=config.midi?3:config.random?2:config.value?1:0;if(selected!=mode){*this={};mode=selected;}
  if(!mode)return base;
  if(mode==3){applied=std::any_of(config.midiRoutes.begin(),config.midiRoutes.end(),[](int r){return r>=1&&r<=8;});return midi246Apply(base,0,type,config.midiRoutes,config.midiCC);}
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
