// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
struct Phone246State {
    uint32_t tag=0;int32_t low=0,high=0,previous=0;
    uint8_t crossing=0,pending=0;uint16_t reserved=0;
    int32_t recordBuffer=0,nextBuffer=1,playBuffer=2,periods=0,repeats=0;
    int16_t meter=0,padding=0;int32_t recordCursor=0,playCursor=0,cursor=16;uint32_t phase=0;
    std::array<int16_t,32> ring{};std::array<int32_t,3> lengths{{128,128,128}};int32_t slot=0;
};
static_assert(sizeof(Phone246State)==140);
// Three logical 5880-sample buffers and two explicit guard cells. The original
// max-length sentinel can touch the first cell beyond the logical slot stride.
using Phone246Memory=std::array<int16_t,17642>;
inline int16_t phone246HighWord(int32_t x) noexcept {return int16_t(signedPhase((uint32_t(x)>>16)|(x<0?0xffff0000u:0u)));}
inline int32_t phone246Fractional16(int16_t a,int16_t b) noexcept {return saturate32(int64_t(a)*b*2);}
inline void phone246Reset(Phone246State& s,Phone246Memory& memory) noexcept {
    s.recordBuffer=0;s.nextBuffer=1;s.playBuffer=2;s.periods=s.repeats=0;s.meter=0;
    s.recordCursor=s.playCursor=0;s.cursor=16;s.phase=0;s.crossing=s.pending=0;s.ring.fill(0);s.lengths.fill(128);s.slot=0;
    std::fill(memory.begin()+11760,memory.begin()+17640,0);
}
inline bool phone246Block(Phone246State& s,const std::array<int16_t,4>& c,const std::array<int16_t,225>& table,
                          int32_t highCoefficient,int16_t lowCoefficient,Phone246Memory& memory,
                          std::array<int16_t,64>& audio,std::array<uint32_t,16>& meters) noexcept {
    if(c[0]<204||c[0]>20480||c[1]<3072||c[1]>17408||c[2]<1536||c[2]>16896||c[3]<0||s.slot!=0||s.recordBuffer<0||s.recordBuffer>2||s.nextBuffer<0||s.nextBuffer>2||s.playBuffer<0||s.playBuffer>2||s.recordBuffer==s.nextBuffer||s.recordBuffer==s.playBuffer||s.playBuffer==s.nextBuffer||s.recordCursor<0||s.recordCursor>=5880||s.playCursor<0||s.playCursor>5880||s.cursor<0||s.cursor>=32||s.phase>=2097152)return false;
    for(auto length:s.lengths)if(length<1||length>5880)return false;
    const int phonic=c[1]/2048,baud=c[2]/1024,maxLength=phonic*735;
    const auto step=uint32_t(multiplyQ15(c[0],c[0]))*32u;
    auto at=[&](int buffer,int position)->int16_t& {return memory[size_t(buffer)*5880+size_t(position)];};
    for(auto x:audio){
        const auto magnitude=x==int16_t(-32768)?int16_t(-32768):int16_t(x<0?-int(x):int(x));
        s.meter=saturate16(int(multiplyQ15(s.meter,32693))+multiplyQ15(saturate16(int(magnitude)*256),74));
        at(s.recordBuffer,s.recordCursor)=x;++s.recordCursor;
        if(s.recordCursor>=maxLength){s.lengths[size_t(s.recordBuffer)]=s.recordCursor;s.periods=0;s.playCursor=s.lengths[size_t(s.playBuffer)];at(s.playBuffer,s.playCursor)=0;s.recordCursor=0;s.repeats=baud;}
        else if(x<-32)s.crossing=0;
        else if(x>32&&!s.crossing){s.crossing=1;if(s.recordCursor>128){s.periods=signedPhase(uint32_t(s.periods)+1);if(s.periods>=phonic){s.lengths[size_t(s.recordBuffer)]=s.recordCursor;s.pending=1;s.recordCursor=s.periods=0;std::swap(s.recordBuffer,s.nextBuffer);}}}
    }
    for(auto& input:audio){
        const int count=std::clamp(int((s.phase+step)>>16)-int(s.phase>>16),0,10);const int start=s.cursor;
        for(int n=0;n<count;++n){s.ring[size_t((start+n)&31)]=at(s.playBuffer,s.playCursor);++s.playCursor;
            if(s.playCursor>=s.lengths[size_t(s.playBuffer)]){s.repeats=signedPhase(uint32_t(s.repeats)+1);s.playCursor=0;if(s.pending&&s.repeats>=baud){s.pending=0;s.repeats=0;std::swap(s.playBuffer,s.nextBuffer);}}}
        for(int n=0;n<count;++n){auto& cell=s.ring[size_t((start+n)&31)];const auto sample=int32_t(cell)*65536;
            s.high=multiplyQ31(saturate32(int64_t(saturate32(int64_t(sample)+s.high))-s.previous),highCoefficient);s.previous=sample;cell=phone246HighWord(s.high);}
        for(int n=0;n<count;++n){auto& cell=s.ring[size_t((start+n)&31)];
            const auto a=phone246Fractional16(lowCoefficient,cell),b=phone246Fractional16(lowCoefficient,phone246HighWord(s.low));
            s.low=saturate32(int64_t(saturate32(int64_t(s.low)+a))-b);cell=phone246HighWord(s.low);}
        s.cursor=(start+count)&31;
        const auto fraction=int16_t((s.phase&2047u)*16),inverse=saturate16(32767-int(fraction));const auto phaseIndex=size_t(31-((s.phase&65535u)>>11));int16_t total=0;
        for(size_t j=0;j<7;++j){const auto x=s.ring[((s.phase>>16)+j)&31u];const auto index=phaseIndex+j*32;
            const auto a=multiplyQ15(multiplyQ15(x,table[index]),fraction),b=multiplyQ15(multiplyQ15(x,table[index+1]),inverse);
            total=saturate16(int(total)+a);total=saturate16(int(total)+b);}
        s.phase+=step;if(s.phase>=2097152)s.phase-=2097152;
        const auto wet=multiplyQ15(multiplyQ15(total,s.meter),c[3]),dry=multiplyQ15(input,saturate16(32767-int(c[3])));input=saturate16(int(wet)+dry);
    }
    const auto pos=size_t(s.slot)*4;meters[pos]=uint16_t(s.meter)|(uint32_t(uint16_t(saturate16(s.recordBuffer*16384)))<<16);
    meters[pos+1]=uint16_t(saturate16(s.nextBuffer*16384))|(uint32_t(uint16_t(saturate16(s.playBuffer*16384)))<<16);
    return true;
}
}
