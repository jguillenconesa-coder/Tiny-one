// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Cwo246Foundation.h"
#include "Cwo246Tables.h"
namespace pocket::reference {
struct Cwo246DelayState {int32_t write=0;int16_t last=0,previous=0;uint32_t flags=1;};
static_assert(sizeof(Cwo246DelayState)==12);
using Cwo246DelayMemory=std::array<int16_t,65536>;
inline bool cwo246DelaySample(Cwo246DelayState& s,Cwo246DelayMemory& memory,int16_t input,uint32_t delay,int32_t delta,int16_t& output) noexcept {
    if(s.write<0||s.write>65535||delay<1||delay>0x7ffffffu)return false;
    memory[size_t(s.write)]=input;auto integer=int(delay>>11);auto fraction=delay&2047u;if(integer>0){fraction+=2048;--integer;}
    const auto negative=signedPhase(0u-uint32_t(delta));const bool shortPath=std::max(delta,negative)<61;
    const auto read=s.write-integer-(shortPath?2:6);
    if((s.flags&255u)&&read<0)s.last=s.previous=0;
    else {
        const auto a=cwo246Interpolation[fraction*2],b=cwo246Interpolation[fraction*2+1];
        auto at=[&](int index){return memory[uint32_t(index)&65535u];};
        if(shortPath){const auto before=s.last;const auto x=multiplyQ15(b,saturate16(int(at(read+1))-s.last)),y=multiplyQ15(a,saturate16(int(at(read+2))-s.previous));s.last=saturate16(int(at(read))+saturate16(int(x)+y));s.previous=before;}
        else {
            auto low=at(read),high=at(read+1);s.last=low;s.previous=high;
            for(int j=0;j<4;++j){const auto next=at(read+j+2);const auto x=multiplyQ15(b,saturate16(int(high)-s.last)),y=multiplyQ15(a,saturate16(int(next)-s.previous));s.previous=s.last;s.last=saturate16(int(low)+saturate16(int(x)+y));low=high;high=next;}
        }
    }
    output=s.last;if(++s.write>65535){s.write=0;s.flags&=0xffffff00u;}return true;
}
inline bool cwo246SinCos(int32_t phase,std::array<int16_t,2>& out) noexcept {
    if(phase< -1073741824||phase>=1073741824)return false;
    const auto shifted=signedPhase((uint32_t(phase)>>21)|(phase<0?0xfffff800u:0u));const auto index=size_t(shifted+512),cosine=(index+256)&1023u;const auto fraction=int16_t((uint32_t(phase)&2097151u)>>6);
    auto lookup=[&](size_t p){return saturate16(int(cwo246Sine[p])+multiplyQ15(saturate16(int(cwo246Sine[p+1])-cwo246Sine[p]),fraction));};out={{lookup(index),lookup(cosine)}};return true;
}
inline bool cwo246Oscillator(int32_t phase,int32_t increment,int16_t amplitude,std::array<int16_t,64>& sine,std::array<int16_t,64>& cosine) noexcept {
    std::array<int16_t,2> initial,step;if(!cwo246SinCos(phase,initial)||!cwo246SinCos(increment,step))return false;
    auto s=saturate16(2*int(multiplyQ15(initial[0],amplitude))),c=saturate16(2*int(multiplyQ15(initial[1],amplitude)));
    for(size_t i=0;i<64;++i){sine[i]=s;cosine[i]=c;const auto a=saturate16(int(multiplyQ15(c,step[1]))-multiplyQ15(s,step[0]));const auto b=saturate16(int(multiplyQ15(c,step[0]))-multiplyQ15(s,saturate16(-int(step[1]))));c=saturate16(2*int(a));s=saturate16(2*int(b));}return true;
}
}
