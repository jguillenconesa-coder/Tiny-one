// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Midi246.h"
namespace pocket::reference {
// Shared observed Bend input words. The low byte of words[1] is a flag;
// words[2] is a two-entry ring index. No physical accessory protocol is implied.
struct Bend246State {std::array<uint32_t,8> words{};std::array<int32_t,2> history{};uint32_t display=0;};
inline void bend246ResetInput(Bend246State& s) noexcept {s.words[1]&=0xffffff00u;}
inline void bend246Prepare(Bend246State& s) noexcept {s.display&=0xff00ffffu;}
inline void bend246Advance(Bend246State& s,std::array<int16_t,8>& c) noexcept {
 if((s.display>>16)&255u)return;
 s.display=(s.display&0xff00ffffu)|0x10000u;
 auto& w=s.words;++w[3];
 if(w[3]%10==0){const int angle=int(floorDivide(c[3],64)),direction=int(floorDivide(c[7],2048));
  if(signedPhase(w[7])!=direction){w[7]=uint32_t(direction);w[0]=0;w[1]&=0xffffff00u;}
  if((w[1]&255u)==0){w[1]=(w[1]&0xffffff00u)|1u;w[4]=uint32_t(angle);s.history={};w[5]=0;}
  int delta=angle-signedPhase(w[4]);if(delta>128)delta-=256;else if(delta<-128)delta+=256;w[4]=uint32_t(angle);if(direction==0)delta=-delta;
  s.history[size_t(w[2])]=delta;w[2]=(w[2]+1)%2;
  if(delta!=0)w[6]=0;else if(w[6]<60)++w[6];
  const auto sum=s.history[0]+s.history[1];int target=signedPhase(w[5]);
  if(sum>0)target+=sum*15;else if(sum<0)target+=sum*20;else if(w[6]>=60&&w[3]%20==0)--target;
  w[5]=uint32_t(std::clamp(target,0,127));
 }
 if(signedPhase(w[0])<signedPhase(w[5]))++w[0];else if(signedPhase(w[5])<signedPhase(w[0]))--w[0];c[4]=int16_t(uint16_t(w[0])*64u);
}
inline int16_t bend246Signal(const Bend246State& s,int16_t amount) noexcept {return multiplyQ15(saturate16(int(int16_t(s.words[0]))*256),amount);}
// Bounded parameter API: valid nonnegative selectors; four common ADSR,
// Instrument (including fine selectors), and FX kinds 2/5. Audio/pitch and
// auxiliary ENV routes are deliberately excluded from this reconstruction.
inline bool bend246Apply(const Bend246State& s,const std::array<int16_t,8>& c,int family,int kind,int type,std::array<int16_t,8>& targets) noexcept {
 const int destination=int(floorDivide(c[1],2048));const auto mod=bend246Signal(s,c[2]);
 if(family==0&&destination==1&&kind==1){const auto i=size_t(uint16_t(c[0])>>12);if(i>=4)return false;targets[i]=value246Adjust(targets[i],mod,valueVoice246EnvelopeRanges[i]);return true;}
 if(family==2&&destination==2&&(kind==2||kind==5)&&type>=0&&type<8){const auto i=size_t(uint16_t(c[0])>>12);if(i>=8)return false;targets[i]=value246Adjust(targets[i],mod,value246FxRanges[size_t(type)][i]);return true;}
 if(destination!=0||(family==0&&kind!=0)||family==2)return false;
 const auto colour=size_t(uint16_t(c[0])>>(midi246Extended(type)?11:12));if(colour>=8)return false;
 if(family==1){if(type!=8&&type!=9)return false;drum246Modulate(targets,type,colour,mod);return true;}
 const auto index=valueVoice246RangeIndex(type);if(index<0)return false;
 if(type==6)sampler246Modulate(targets,colour,mod);else {const auto i=type==12?colour:colour%4;targets[i]=valueVoice246Adjust(targets[i],type==12||colour<4?mod:int16_t(floorDivide(mod,16)),valueVoice246Ranges[size_t(index)][i]);}return true;
}
inline bool bend246Step(Bend246State& s,std::array<int16_t,8>& c,int family,int kind,int type,std::array<int16_t,8>& targets) noexcept {bend246Advance(s,c);return bend246Apply(s,c,family,kind,type,targets);}
}
