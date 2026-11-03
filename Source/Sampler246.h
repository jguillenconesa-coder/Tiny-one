// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Drum246.h"
namespace pocket::reference {
// Isolated six-voice sampler reader. PCM, pitch scaling and event order supplied.
struct Sampler246State {
    uint32_t tag=0,handle=0;int32_t position=0;uint8_t released=0,restart=0;uint16_t padding=0;
    void construct() noexcept {tag=0x019119f4;handle=0;position=0;released=0;}
    void bind(uint32_t h) noexcept {handle=h;}
    void reset(const std::array<int16_t,8>& c) noexcept {released=0;restart=1;position=multiplyQ31(int32_t(c[c[4]/16384?3:0])*65536,0x40998000);}
    void release() noexcept {released=1;}
};
static_assert(sizeof(Sampler246State)==16);
inline int32_t sampler246Position(int16_t coarse,int16_t fine=0) noexcept {
    return multiplyQ31(saturate32(int64_t(coarse)*65536+int64_t(uint16_t(fine))*2),0x40998000);
}
inline void sampler246Block(Sampler246State& s,const std::array<int16_t,8>& c,
    const std::array<uint32_t,128>& pitch,int32_t scale,Drum246Pcm pcm,
    const std::array<int16_t,1025>& fir,std::array<int16_t,128>& output) noexcept {
    const auto start=sampler246Position(c[0]),end=sampler246Position(c[3]);
    const auto loopStart=sampler246Position(c[1],c[5]),loopEnd=sampler246Position(c[2],c[6]);
    if(s.restart){s.position=std::max(start,s.position);s.restart=0;}
    const bool reverse=c[4]/16384!=0,loop=c[0]<c[2]&&!s.released;
    const int32_t gain=int32_t(multiplyQ15(c[7],c[7]))*65536;
    for(size_t i=0;i<128;++i){
        if(loop&&(reverse?s.position<loopStart:s.position>=loopEnd))s.position=reverse?loopEnd:loopStart;
        if(reverse?s.position<start:s.position>=end){output[i]=0;continue;}
        const auto frame=int32_t(floorDivide(s.position,4096));const auto phase=(uint32_t(s.position)&4095u)>>6;
        int32_t value=0;for(size_t tap=0;tap<16;++tap)value=saturate32(int64_t(value)+int64_t(pcm.at(int64_t(frame)+tap))*fir[63-phase+tap*64]*2);
        value=saturate32(int64_t(multiplyQ31(value,gain))*16);output[i]=int16_t(floorDivide(value,65536));
        const auto step=floorDivide(multiplyQ31(signedPhase(pitch[i]),scale),128);
        s.position=saturate32(int64_t(s.position)+(reverse?-int64_t(step):int64_t(step)));
    }
}
}
