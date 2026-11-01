// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Punch246.h"
namespace pocket::reference {
struct Filter246State {
    uint32_t tag=0;int32_t feedback=0,pole=0;int16_t inputMeter=0,outputMeter=0;int32_t slot=0;
    std::array<int32_t,4> stages{};
};
static_assert(sizeof(Filter246State)==36);
// Mono original callback: block peak is computed BEFORE rendering any sample.
// Caller supplies the generated mix/frequency tables, sine and display workspace.
inline bool filter246Block(Filter246State& state,const std::array<int16_t,4>& c,
                           const std::array<int32_t,65>& mixTable,const std::array<int32_t,129>& frequency,
                           const std::array<int16_t,8192>& sine,std::array<int16_t,64>& audio,
                           std::array<uint32_t,16>& meters) noexcept {
    if(c[0]<64||c[0]>14528||c[1]<0||c[1]>24576||c[2]<0||c[2]>22528||c[3]<8192||state.slot<0||state.slot>=4)return false;
    state.inputMeter=multiplyQ15(state.inputMeter,32512);
    for(auto x:audio)state.inputMeter=std::max(state.inputMeter,x);
    state.pole=saturate32(int64_t(multiplyQ31(state.pole,0x70000000))+multiplyQ31(frequency[size_t(c[0]/128)],0x10000000));
    const auto fraction=int32_t(c[2]&2047)*1048576;
    const auto inverse=saturate32(int64_t(2147483647)-fraction);
    const auto a=saturate32(int64_t(multiplyQ31(fraction,fraction))*2);
    const auto b=saturate32(int64_t(multiplyQ31(inverse,inverse))*2);
    std::array<int32_t,5> mix{};const auto section=size_t(c[2]/2048);
    for(size_t i=0;i<5;++i)mix[i]=saturate32(int64_t(multiplyQ31(mixTable[section*5+i],b))+multiplyQ31(mixTable[(section+1)*5+i],a));
    const auto drive=multiplyQ15(c[3],c[3]);
    const auto amount=saturate16(int(multiplyQ15(multiplyQ15(c[1],c[1]),c[3]))+multiplyQ15(state.inputMeter,c[1]));
    auto previous=state.feedback;
    for(auto& input:audio){
        const auto driven=saturate32(int64_t(input)*drive*2);
        const auto difference=saturate32(int64_t(punch246Curve(previous,sine))-driven);
        const auto returned=multiplyQ31(saturate32(int64_t(difference)*4),int32_t(amount)*65536);
        auto x=punch246Curve(saturate32(int64_t(driven)-returned),sine);
        auto total=multiplyQ31(x,mix[0]);
        for(size_t i=0;i<4;++i){
            // Spell the Blackfin arithmetic shift as unsigned bits, including
            // sign extension. Avoid MSVC's optimized division/remainder path
            // here: native oracle diagnostics exposed low-bit stage drift.
            const auto offset=signedPhase((uint32_t(x)>>4)|(x<0?0xf0000000u:0u));
            const auto coefficient=std::clamp(saturate32(int64_t(frequency[size_t(c[0]/128)])+offset),int32_t(0x00400000),int32_t(0x73ffffff));
            const auto negative=saturate32(-int64_t(state.stages[i]));
            x=saturate32(int64_t(saturate32(int64_t(state.stages[i])+multiplyQ31(x,coefficient)))+multiplyQ31(negative,coefficient));
            state.stages[i]=x;total=saturate32(int64_t(total)+multiplyQ31(x,mix[i+1]));
        }
        previous=x;
        const auto shaped=punch246Curve(punch246Curve(saturate32(int64_t(total)*4),sine),sine);
        input=int16_t(floorDivide(shaped,65536));
    }
    state.feedback=previous;state.outputMeter=multiplyQ15(state.outputMeter,32512);
    for(auto x:audio)state.outputMeter=std::max(state.outputMeter,x);
    meters[size_t(state.slot)*4]=uint16_t(state.inputMeter)|(uint32_t(uint16_t(state.outputMeter))<<16);
    return true;
}
}
