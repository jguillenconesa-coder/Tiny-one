// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace pocket {
// Authored causal external-voice input: 44.1 kHz, latest 64 signed Q15 samples.
// Voices capture that history for 128 gain words, each input driving a pair.
struct Element246VoiceInput {
 struct Filter {double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;double next(double x) noexcept {const auto y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return y;}};
 std::array<Filter,2> filters{};std::array<int16_t,64> words{};unsigned head=0;double rate=48000,ratio=48000./44100,previous=0;uint64_t host=0,input=0;bool active=false;
 void reset() noexcept {words.fill(0);head=0;host=input=0;previous=0;active=false;for(auto& f:filters)f.z1=f.z2=0;}
 void prepare(double sampleRate) noexcept {rate=std::max(8000.,sampleRate);ratio=rate/44100.;const double w=6.283185307179586*std::min(17640.,rate*.4)/rate,cs=std::cos(w),sn=std::sin(w),q[]={.541196100146197,1.306562964876377};for(size_t i=0;i<2;++i){const double a=sn/(2*q[i]),a0=1+a;filters[i]={(1-cs)/(2*a0),(1-cs)/a0,(1-cs)/(2*a0),-2*cs/a0,(1-a)/a0,0,0};}reset();}
 void next(bool enabled,float external) noexcept {if(!enabled){if(active)reset();return;}if(!active){reset();active=true;}double filtered=std::isfinite(external)?double(external):0.;if(rate!=44100)for(auto& f:filters)filtered=f.next(filtered);while(double(input)*ratio<=double(host)+1.e-9){const double fraction=host==0?1:std::clamp(double(input)*ratio-(double(host)-1),0.,1.);const double value=previous+(filtered-previous)*fraction;words[head]=int16_t(std::clamp(std::round(value*32768),-32768.,32767.));head=(head+1)%64;++input;}previous=filtered;++host;}
};
}
