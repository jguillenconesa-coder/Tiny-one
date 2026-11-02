// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
// Complete isolated mono callback. PCM, pitch, gain and event order are supplied.
inline int32_t drum246Position(int16_t coarse,int16_t fine) noexcept {
    return multiplyQ31(saturate32(int64_t(coarse)*65536+int32_t(fine)*2),0x40998000);
}
struct Drum246State {
    uint32_t tag=0,handle=0;int32_t position=0,mode=0;uint32_t blocks=0;
    void construct() noexcept {tag=0x0190fac4u;handle=0;position=0;mode=0;}
    bool bind(uint32_t reference) noexcept {if(reference==0)return false;handle=reference;return true;}
    void reset(const std::array<int16_t,8>& controls) noexcept {mode=1;const bool reverse=controls[4]/16384!=0;position=reverse?drum246Position(controls[2],controls[6]):drum246Position(controls[1],controls[5]);blocks=0;}
    void release(const std::array<int16_t,8>& controls) noexcept {mode=(floorDivide(controls[3],8192)==1)?2:0;}
    uint32_t active() noexcept {if(mode==0)blocks=0;return blocks;}
};
static_assert(sizeof(Drum246State)==20);
struct Drum246Pcm {
    const int16_t* samples=nullptr;size_t count=0;int64_t origin=0;
    int16_t at(int64_t index) const noexcept {index+=origin;return samples&&index>=0&&uint64_t(index)<count?samples[size_t(index)]:0;}
};
inline int32_t drum246Fir(int32_t position,Drum246Pcm pcm,const std::array<int16_t,1025>& coefficients) noexcept {
    const auto frame=floorDivide(position,2048);const auto phase=(uint32_t(position)&4095u)>>6;int32_t value=0;
    for(size_t i=0;i<16;++i){const auto c=coefficients[63-phase+i*64];const auto sample=pcm.at(frame+int64_t(i));const auto product=saturate32(int64_t(c)*sample*2);value=saturate32(int64_t(value)+product);}
    return value;
}
inline bool drum246Block(Drum246State& state,const std::array<int16_t,8>& controls,
                         const std::array<uint32_t,128>& pitch,const std::array<int16_t,128>& gain,
                         Drum246Pcm pcm,const std::array<int16_t,1025>& coefficients,
                         std::array<int16_t,128>& audio,std::array<int32_t,128>& derived) noexcept {
    if(state.handle==0)return false;
    const auto start=drum246Position(controls[1],controls[5]),end=drum246Position(controls[2],controls[6]);if(start<0||end<0)return false;
    const bool reverse=floorDivide(controls[4],16384)!=0,loop=floorDivide(controls[3],8192)==2&&state.mode==1;bool exhausted=false;
    for(size_t i=0;i<128;++i){auto value=multiplyQ15(controls[7],gain[i]);value=multiplyQ15(value,controls[7]);value=multiplyQ15(value,gain[i]);derived[i]=int32_t(value)*65536;}
    for(size_t i=0;i<128;++i){
        if(!loop&&(reverse?state.position<start:state.position>=end)){audio[i]=0;exhausted=true;continue;}
        const auto value=multiplyQ31(drum246Fir(state.position,pcm,coefficients),derived[i]);audio[i]=int16_t(floorDivide(saturate32(int64_t(value)*16),65536));
        const auto step=int32_t(floorDivide(signedPhase(pitch[i]),131072));state.position=saturate32(int64_t(state.position)+(reverse?-int64_t(step):step));
        if(loop){if(reverse?state.position<start:state.position>=end)state.position=reverse?end:start;}
    }
    if(exhausted)state.mode=0;++state.blocks;return true;
}
}
