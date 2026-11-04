// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
#include <cstdlib>
namespace pocket::reference {
// Research-only model of the readable 0x01a66922 writer. The input controls,
// caller state and shared word are explicit; no hardware routing is implied.
// Zero defaults are API convenience, not a recovered caller constructor.
struct EnvelopePublisher246State {int32_t first=0,second=0;};
struct EnvelopePublisher246 {
 static void resetPublished(int32_t& published) noexcept {published=0;}
 static int16_t readPublished(int32_t published) noexcept {return int16_t(floorDivide(published,65536));}
 struct Coefficients {
  int32_t rise=0,fall=0,firstRise=0,firstFall=0,firstQuiet=0;
  int32_t negative=0,base=0,positive=0;
  explicit Coefficients(const std::array<int16_t,4>& c) noexcept {
   constexpr int32_t maximum=2147483647,small=0x20c49b;
   const int32_t inverse=saturate32(int64_t(maximum)-int64_t(c[3])*65536);
   const auto alpha=saturate32(int64_t(multiplyQ31(small,inverse))+small);
   const auto half=int32_t(floorDivide(alpha,2));
   rise=saturate32(int64_t(alpha)+1);fall=saturate32(int64_t(maximum)-alpha);
   firstRise=saturate32(int64_t(half)+1);firstFall=saturate32(int64_t(maximum)-half);
   const auto q=int32_t(floorDivide(inverse,4));auto power=multiplyQ31(q,q);
   for(int i=0;i<3;++i)power=multiplyQ31(power,q);
   firstQuiet=saturate32(int64_t(firstFall)+multiplyQ31(power,0x20000000));
   negative=int32_t(c[0])*65536;base=int32_t(c[1])*65536;positive=int32_t(c[2])*65536;
  }
 };
 static int16_t next(EnvelopePublisher246State& state,int32_t& published,int16_t sample,const Coefficients& c) noexcept {
  auto& first=state.first;auto& second=state.second;
  const auto magnitude=int32_t(std::min(32767,std::abs(int(sample))))*65536;
  if(magnitude>first)first=saturate32(int64_t(multiplyQ31(magnitude,c.firstRise))+multiplyQ31(first,c.firstFall));
  else first=multiplyQ31(first,c.firstQuiet);
  const bool rising=magnitude>second;second=multiplyQ31(second,c.fall);
  if(rising)second=saturate32(int64_t(second)+multiplyQ31(magnitude,c.rise));
  const auto difference=saturate32(int64_t(saturate32(int64_t(first)-second))*64);
  auto target=difference<0?saturate32(int64_t(c.base)-multiplyQ31(difference,c.negative)):saturate32(int64_t(c.base)+multiplyQ31(difference,c.positive));
  target=std::max(0,target);
  published=saturate32(int64_t(multiplyQ31(target,c.rise))+multiplyQ31(published,c.fall));
  return int16_t(floorDivide(saturate32(int64_t(multiplyQ31(int32_t(sample)*65536,published))*2),65536));
 }
 static void process(EnvelopePublisher246State& state,int32_t& published,const std::array<int16_t,4>& controls,std::array<int16_t,128>& audio) noexcept {
  const Coefficients c(controls);for(auto& value:audio)value=next(state,published,value,c);
 }
};
}
