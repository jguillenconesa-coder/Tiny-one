// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Midi246.h"
namespace pocket::reference {
// Independently authored bounded Crank input model. Words preserve the observed
// shared RAM layout; only valid phase, ring-index and velocity states are used.
struct Crank246State {
 std::array<uint32_t,7> words{};
 std::array<int32_t,10> history{};
 int32_t previous=0;
};
inline void crank246Prepare(Crank246State& s) noexcept {s.words[0]&=0xffffff00u;}
inline void crank246ResetInput(Crank246State& s) noexcept {s.words[0]&=0xffff00ffu;}
inline int crank246Level(const Crank246State& s) noexcept {
 const int phase=int(s.words[2]);return phase>127?std::abs(phase-255):phase;
}
inline int crank246Advance(Crank246State& s,std::array<int16_t,8>& c) noexcept {
 auto& w=s.words;
 if((w[0]&255u)!=0)return crank246Level(s);
 w[0]=(w[0]&0xffffff00u)|1u;++w[1];
 if(w[1]%50==0){
  const int angle=int(floorDivide(c[0],64))+128;
  if((w[0]&0xff00u)==0){w[0]|=0x100u;s.previous=angle;s.history={};w[3]=w[5]=0;}
  int delta=angle-s.previous;s.previous=angle;
  if(delta>128)delta-=256;else if(delta<-128)delta+=256;
  s.history[size_t(w[4])]=delta;w[4]=(w[4]+1)%10;
  int sum=0;for(auto v:s.history)sum+=v;sum=std::clamp(sum,-120,120);
  const int velocity=signedPhase(w[3]);int target=signedPhase(w[5]);
  auto towardsZero=[&](int step){if(target>0)target=std::max(0,target-step);else if(target<0)target=std::min(0,target+step);};
  if((velocity<0&&sum>0)||(velocity>0&&sum<0))target=velocity>0?std::max(0,target-4):std::min(0,target+4);
  else if(std::abs(sum)<std::abs(target))towardsZero(2);
  else if(sum>0)target=std::min(sum,target+5);
  else if(sum<0)target=std::max(sum,target-5);
  w[5]=uint32_t(target);
 }
 if((w[1]&3u)==0){if(signedPhase(w[3])<signedPhase(w[5]))++w[3];else if(signedPhase(w[3])>signedPhase(w[5]))--w[3];}
 const int velocity=signedPhase(w[3]),magnitude=std::abs(velocity);
 const int step=(magnitude>>5)+(w[6]<uint32_t(magnitude&31)?1:0);
 if(++w[6]>31)w[6]=0;
 int phase=int(w[2]);if(velocity>0){phase+=step;if(phase>255)phase-=256;}else if(velocity<0){phase-=step;if(phase<0)phase+=256;}
 w[2]=uint32_t(phase);c[4]=int16_t(phase*64);return crank246Level(s);
}
inline int16_t crank246Signal(const Crank246State& s,int16_t amount) noexcept {
 return multiplyQ15(saturate16((crank246Level(s)-64)*512),amount);
}
// Parameter routes only. Special gain/pitch/audio and FX kind 4 are excluded.
inline bool crank246Apply(const Crank246State& s,const std::array<int16_t,8>& c,int family,int kind,int type,std::array<int16_t,8>& targets) noexcept {
 const int destination=int(floorDivide(c[2],2048));const auto mod=crank246Signal(s,c[1]);
 if(family==0&&destination==1&&kind==1){const auto i=size_t(uint16_t(c[3])>>12);if(i>=4)return false;targets[i]=value246Adjust(targets[i],mod,valueVoice246EnvelopeRanges[i]);return true;}
 if(family==2&&destination==2&&(kind==2||kind==5)&&type>=0&&type<8){const auto i=size_t(uint16_t(c[3])>>12);if(i>=8)return false;targets[i]=value246Adjust(targets[i],mod,value246FxRanges[size_t(type)][i]);return true;}
 if(destination!=0||(family==0&&kind!=0)||family==2)return false;
 const auto colour=size_t(uint16_t(c[3])>>(midi246Extended(type)?11:12));if(colour>=8)return false;
 if(family==1){if(type!=8&&type!=9)return false;drum246Modulate(targets,type,colour,mod);return true;}
 const auto index=valueVoice246RangeIndex(type);if(index<0)return false;
 if(type==6)sampler246Modulate(targets,colour,mod);else {const auto i=type==12?colour:colour%4;targets[i]=valueVoice246Adjust(targets[i],type==12||colour<4?mod:int16_t(floorDivide(mod,16)),valueVoice246Ranges[size_t(index)][i]);}return true;
}
inline bool crank246Step(Crank246State& s,std::array<int16_t,8>& c,int family,int kind,int type,std::array<int16_t,8>& targets) noexcept {
 crank246Advance(s,c);return crank246Apply(s,c,family,kind,type,targets);
}
}
