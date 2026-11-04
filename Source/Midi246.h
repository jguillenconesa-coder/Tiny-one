// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Drum246ModulationReference.h"
#include "Midi246Special.h"
namespace pocket::reference {
inline int16_t midi246Modulation(uint32_t value) noexcept {return saturate16(int(int16_t(uint16_t(value)-64u))*1024);}
inline constexpr bool midi246Extended(int type) noexcept {return type==6||type==8||type==9||type==12;}
// Supplied four CC words are observed IPC input, not an implementation of the
// physical MIDI receiver. Streams run in order and may revisit a destination.
inline bool midi246SynthStep(const std::array<int16_t,8>& controls,const std::array<uint32_t,4>& cc,std::array<int16_t,4>& display,int kind,int type,std::array<int16_t,8>& targets,std::array<uint32_t,128>& pitch,std::array<int16_t,128>& gain) noexcept {
 if(valueVoice246RangeIndex(type)<0)return false;
 bool accepted=false;
 for(size_t i=0;i<4;++i){const int destination=int(floorDivide(controls[4+i],2048));const auto colour=size_t(uint16_t(controls[i])>>(midi246Extended(type)?11:12));const auto mod=midi246Modulation(cc[i]);
  if(destination==0&&kind==0&&colour<8){if(type==6)sampler246Modulate(targets,colour,mod);else {const auto index=type==12?colour:colour%4;const auto amount=type==12||colour<4?mod:int16_t(floorDivide(mod,16));targets[index]=valueVoice246Adjust(targets[index],amount,valueVoice246Ranges[size_t(valueVoice246RangeIndex(type))][index]);}accepted=true;}
  else if(destination==1&&kind==1){const auto target=size_t(uint16_t(controls[i])>>12);if(target<4){targets[target]=value246Adjust(targets[target],mod,valueVoice246EnvelopeRanges[target]);accepted=true;}}
  else if(destination==3&&kind==0){const auto target=uint16_t(controls[i])>>12;
   if(target==0){display[i]=midi246VoiceGainCoefficient(display[i],cc[i]);for(auto& v:gain)v=midi246VoiceGain(v,display[i]);accepted=true;}
   else if(target==1){for(auto& v:pitch)v=midi246VoicePitch(v,cc[i]);accepted=true;}
  }
 }return accepted;
}
inline bool midi246DrumStep(const std::array<int16_t,8>& controls,const std::array<uint32_t,4>& cc,int /*kind*/,int type,std::array<int16_t,8>& targets) noexcept {
 if(type!=8&&type!=9)return false;
 bool accepted=false;for(size_t i=0;i<4;++i)if(floorDivide(controls[4+i],2048)==0){const auto colour=size_t(uint16_t(controls[i])>>11);if(colour<8){drum246Modulate(targets,type,colour,midi246Modulation(cc[i]));accepted=true;}}return accepted;
}
// FX kinds 2/5 only. The separate kind-4 auxiliary ENV route is outside this API.
inline bool midi246FxStep(const std::array<int16_t,8>& controls,const std::array<uint32_t,4>& cc,int kind,int fx,std::array<int16_t,8>& targets,std::array<int16_t,128>& audio) noexcept {
 if(fx<0||fx>=8||(kind!=2&&kind!=5))return false;
 bool accepted=false;
 for(size_t i=0;i<4;++i){const auto destination=int(floorDivide(controls[4+i],2048));const auto colour=size_t(uint16_t(controls[i])>>12);
  if(destination==2&&colour<8){targets[colour]=value246Adjust(targets[colour],midi246Modulation(cc[i]),value246FxRanges[size_t(fx)][colour]);accepted=true;}
  else if(destination==3&&(colour==2||colour==3)){const auto factor=midi246FxAudioFactor(cc[i],colour==3);for(size_t j=0;j<64;++j)audio[j]=multiplyQ15(audio[j],factor);accepted=true;}
 }return accepted;
}
}

namespace pocket {
inline constexpr std::array<const char*,4> midi246Ids{{"midi246CC1","midi246CC2","midi246CC3","midi246CC4"}};
// Authored host snapshot for the two recovered FX-audio multipliers. Stream order
// and Q15 arithmetic are original; sampling the latch per internal input sample
// and placement before the mono FX bus are host policies, not original routing.
struct Midi246FxAudio {
 std::array<int16_t,4> factors{};unsigned mask=0;
 Midi246FxAudio() noexcept = default;
 Midi246FxAudio(const std::array<int,4>& routes,const std::array<uint32_t,4>& cc) noexcept {
  for(size_t i=0;i<4;++i)if(routes[i]==17||routes[i]==18){mask|=1u<<i;factors[i]=reference::midi246FxAudioFactor(cc[i],routes[i]==18);}
 }
 int16_t apply(int16_t sample) const noexcept {
  for(size_t i=0;i<4;++i)if(mask&(1u<<i))sample=reference::multiplyQ15(sample,factors[i]);return sample;
 }
};
// Own host route choices: Off, eight Instrument words, four ADSR, four FX
// parameters, two FX-input curves and the recovered melodic voice gain/pitch.
inline std::array<int16_t,8> midi246Controls(const std::array<int,4>& routes,int type) noexcept {
 std::array<int16_t,8> c{};for(size_t i=0;i<4;++i){const int r=routes[i];c[4+i]=-2048;
  if(r>=1&&r<=8&&(reference::midi246Extended(type)||r<=4)){c[i]=int16_t((r-1)*(reference::midi246Extended(type)?2048:4096));c[4+i]=0;}
  else if(r>=9&&r<=12){c[i]=int16_t((r-9)*4096);c[4+i]=2048;}
  else if(r>=13&&r<=16){c[i]=int16_t((r-13)*4096);c[4+i]=4096;}
  else if(r==17||r==18){c[i]=int16_t((r-15)*4096);c[4+i]=6144;}
  else if(r==19||r==20){c[i]=int16_t((r-19)*4096);c[4+i]=6144;}
 }return c;
}
template<size_t N> inline std::array<int16_t,N> midi246Apply(const std::array<int16_t,N>& base,int kind,int type,const std::array<int,4>& routes,const std::array<uint32_t,4>& cc) noexcept {
 static_assert(N==4||N==8);std::array<int16_t,8> targets{};std::copy(base.begin(),base.end(),targets.begin());
 auto c=midi246Controls(routes,type);
 // Special audio/gain/pitch are handled by their stateful host adapters.
 for(size_t i=0;i<4;++i)if(c[4+i]==6144)c[4+i]=-2048;
 if(kind==2){if(type<0||type>=8)return base;for(size_t i=0;i<4;++i)if(c[4+i]==4096){const auto index=size_t(c[i]/4096);targets[index]=reference::value246Adjust(targets[index],reference::midi246Modulation(cc[i]),reference::value246FxRanges[size_t(type)][index]);}}
 else if(type==8||type==9){reference::midi246DrumStep(c,cc,kind,type,targets);}
 else {std::array<int16_t,4> display{};std::array<uint32_t,128> pitch{};std::array<int16_t,128> gain{};reference::midi246SynthStep(c,cc,display,kind,type,targets,pitch,gain);}
 std::array<int16_t,N> result{};std::copy_n(targets.begin(),N,result.begin());return result;
}
}
