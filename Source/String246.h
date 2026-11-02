// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
#include "Spring246Tables.h"
#include "Delay246Tables.h"
#include "Cluster246.h"
namespace pocket::reference {
// Functional mono reconstruction; original pointers are metadata, never dereferenced.
struct String246State {
    uint32_t tag=0x019118c4u;int32_t noise1=0,noise2=0,feedback=0;
    uint32_t cursor=0,pointer=0x010225ecu;int16_t excitation=0,clear=0;uint32_t flags=0;
    void construct() noexcept {tag=0x019118c4u;noise1=noise2=feedback=0;cursor=pointer=0;excitation=clear=0;flags&=0xffffff00u;}
    void release() noexcept {excitation=256;flags=(flags&0xffffff00u)|1u;}
};
static_assert(sizeof(String246State)==32);
using String246Memory=std::array<int16_t,24576>;
inline int16_t string246High(int32_t x) noexcept {return int16_t(floorDivide(x,65536));}
inline int32_t string246Product(int16_t a,int16_t b) noexcept {return saturate32(int64_t(a)*b*2);}
inline uint32_t string246Random(uint32_t& rng) noexcept {rng=rng*1664525u+1013904223u;return rng;}
inline int32_t string246Filter(int32_t old,int16_t input,int16_t coefficient) noexcept {
    return saturate32(int64_t(saturate32(int64_t(old)+string246Product(input,coefficient)))-string246Product(string246High(old),coefficient));
}
inline void string246Reset(String246State& s,uint32_t& rng) noexcept {
    const auto first=string246Filter(s.noise1,string246High(signedPhase(string246Random(rng))),28672);
    const auto second=string246Filter(s.noise2,string246High(first),28672);
    s.noise1=string246Filter(first,string246High(signedPhase(string246Random(rng))),28672);
    s.noise2=string246Filter(second,string246High(s.noise1),28672);
    s.excitation=32767;s.cursor=0;s.clear=0;s.flags&=0xffffff00u;
}
inline uint32_t string246Period(uint32_t pitch) noexcept {
    return pitch?uint32_t(std::clamp(signedPhase(uint32_t((uint64_t(1)<<48)/pitch)),int32_t(0x20000),int32_t(0x60000000))):0x60000000u;
}
inline bool string246Block(String246State& s,String246Memory& memory,const std::array<int16_t,4>& controls,
                           uint32_t pitch,uint32_t& rng,std::array<int16_t,128>& audio) noexcept {
    const auto period=string246Period(pitch);
    if(!pitch||pitch>0x7fffffffu||s.cursor>=0x60000000u||s.clear<0||s.clear>24576||s.clear%1536)return false;
    if(controls[0]<0||controls[0]>16384||controls[1]<0||controls[1]>24575||controls[2]<0||controls[3]<0||controls[3]>16511)return false;
    auto damp=spring246Low[size_t(128-controls[0]/128)];if(s.flags&255u)damp=multiplyQ15(damp,28672);
    const auto noise=spring246Low[size_t(controls[3]/128)];
    const auto offset=uint32_t(floorDivide(int64_t(period)*controls[2],32768));if(uint64_t(s.cursor)+offset>=0x80000000ull)return false;
    auto shifted=s.cursor+offset;if(shifted>=period)shifted-=period;
    if(s.clear<24576){for(int i=0;i<1536;++i)memory[size_t(s.clear++)]=0;
        const auto index=int(shifted>>16);if(int(s.clear)-140<index){const int start=std::min(index,24436);std::fill_n(memory.begin()+start,140,int16_t(0));}}
    const auto length=period>>16;
    for(auto& output:audio){
        s.noise1=string246Filter(s.noise1,string246High(signedPhase(string246Random(rng))),noise);
        s.noise2=string246Filter(s.noise2,string246High(s.noise1),noise);
        auto& sample=memory[size_t(s.cursor>>16)];sample=saturate16(int(sample)+multiplyQ15(string246High(s.noise2),s.excitation));
        const auto base=31-int((s.cursor&65535u)>>11);const auto fraction=int16_t((s.cursor&2047u)*16u),inverse=int16_t(32767-fraction);
        auto a=shifted>>16,b=s.cursor>>16;int32_t accumulator=0;
        for(size_t j=0;j<7;++j){const size_t index=size_t(base)+j*32;
            const auto weight=saturate16(int(multiplyQ15(delay246Interpolation[index],fraction))+multiplyQ15(delay246Interpolation[index+1],inverse));
            accumulator=saturate32(int64_t(accumulator)+string246Product(weight,memory[b]));if(++b>=length)b-=length;
            accumulator=saturate32(int64_t(accumulator)+string246Product(weight,memory[a]));if(++a>=length)a-=length;
        }
        output=string246High(accumulator);
        s.feedback=string246Filter(s.feedback,sample,damp);sample=string246High(s.feedback);
        s.cursor+=65536u;if(s.cursor>=period)s.cursor-=period;
        shifted+=65536u;if(shifted>=period)shifted-=period;
    }
    s.excitation=multiplyQ15(s.excitation,string246High(cluster246Damping[size_t(controls[1]/1024)]));
    return true;
}
}
