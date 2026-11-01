// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
struct Delay246State {
    uint32_t tag=0,read=9u<<16,clear=8u<<16,write=1u<<16,slot=0;
    // Original cursor reset preserves tag/slot; clearing host memory is separate.
    void reset() noexcept {read=9u<<16;clear=8u<<16;write=1u<<16;}
};
static_assert(sizeof(Delay246State)==20);
// Explicit Blackfin signed high-word extraction avoids the MSVC optimized
// signed division/remainder path that drifted by one in the native oracle.
inline int16_t delay246HighWord(int32_t x) noexcept {return int16_t(signedPhase((uint32_t(x)>>16)|(x<0?0xffff0000u:0u)));}
inline bool delay246Block(Delay246State& s,const std::array<int16_t,4>& c,
                          const std::array<int16_t,225>& table,std::array<int32_t,44100>& memory,
                          std::array<int16_t,64>& audio,std::array<uint32_t,16>& meters) noexcept {
    if(c[0]<1024||c[0]>11264||c[1]<3276||c[2]<0||c[2]>16384||c[3]<0||s.slot>=4)return false;
    const uint32_t length=44100u>>unsigned(c[0]/2048);const uint32_t period=length<<16,step=uint32_t(c[1])*2;
    s.read%=period;s.clear%=period;s.write%=period;
    for(auto& value:audio){
        const auto phase=(s.read&65535u)>>11;const auto fraction=int16_t((s.read&2047u)*16);
        const auto inverse=saturate16(32767-int(fraction));int32_t total=0;
        for(size_t j=0;j<7;++j){const auto x=delay246HighWord(memory[((s.read>>16)+j)%length]);const auto index=size_t(31-phase)+j*32;
            total=saturate32(int64_t(total)+saturate32(int64_t(x)*multiplyQ15(table[index],fraction)*2));
            total=saturate32(int64_t(total)+saturate32(int64_t(x)*multiplyQ15(table[index+1],inverse)*2));}
        memory[s.clear>>16]=0;
        const auto input=saturate32(int64_t(value)*c[3]*2);
        const auto feedback=saturate32(int64_t(multiplyQ31(total,int32_t(c[2])*65536))*2);
        auto feed=multiplyQ31(saturate32(int64_t(input)+feedback),int32_t(c[1])*65536);
        value=saturate16(int(value)+int(delay246HighWord(total)));
        const auto writePhase=(s.write&65535u)>>11;const auto f=int16_t((s.write&2047u)*16),inv=saturate16(32767-int(f));
        for(size_t j=0;j<7;++j){const auto index=size_t(31-writePhase)+j*32;auto& cell=memory[((s.write>>16)+j)%length];
            const auto a=multiplyQ15(table[index],f),b=multiplyQ15(table[index+1],inv);
            cell=saturate32(int64_t(saturate32(int64_t(cell)+multiplyQ31(feed,int32_t(a)*65536)))+multiplyQ31(feed,int32_t(b)*65536));}
        s.read+=step;if(s.read>=period)s.read-=period;s.clear+=step;if(s.clear>=period)s.clear-=period;s.write+=step;if(s.write>=period)s.write-=period;
    }
    const auto index=size_t(s.slot)*4;auto meter=int16_t(meters[index]&65535u);meter=std::max(meter,saturate16(int(audio[0])*64));meter=multiplyQ15(meter,32512);meters[index]=(meters[index]&0xffff0000u)|uint16_t(meter);
    return true;
}
}
