// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Punch246.h"
namespace pocket::reference {
struct Nitro246State {
    uint32_t tag=0;int32_t firstFeedback=0,secondFeedback=0,firstPole=0,secondPole=0;
    int16_t inputMeter=0,outputMeter=0;int32_t slot=0;
    std::array<int32_t,4> first{},second{};
};
static_assert(sizeof(Nitro246State)==60);
// Authored symmetric finite-difference replacement for two protected coefficients.
// The readable endpoints suggest this shape; it is NOT recovered original data.
inline constexpr std::array<int32_t,5> nitro246HybridMix{{357913952,-1431655808,2147483647,-1431655808,357913952}};
// Readable instruction path, conditionally validated with explicit table/pointer
// substitutions. Passing that oracle does not establish the missing original L1.
inline bool nitro246HybridBlock(Nitro246State& s,const std::array<int16_t,4>& c,
                               const std::array<int32_t,5>& mix,const std::array<int32_t,129>& frequency,
                               const std::array<int16_t,8192>& sine,std::array<int16_t,64>& audio,
                               std::array<uint32_t,16>& meters) noexcept {
    if(c[0]<64||c[0]>16448||c[2]<0||c[2]>20643||c[3]<64||c[3]>16448||s.slot<0||s.slot>=4)return false;
    s.inputMeter=multiplyQ15(s.inputMeter,32512);for(auto x:audio)s.inputMeter=std::max(s.inputMeter,x);
    const auto follow=multiplyQ15(multiplyQ15(multiplyQ15(c[1],c[1]),c[1]),s.inputMeter);
    const auto i1=size_t(std::clamp(int(saturate16(int(c[0])+follow))/128,0,113));
    const auto i2=size_t(std::clamp(int(saturate16(int(c[3])+follow))/128,0,113));
    const auto a=frequency[i1],b=frequency[i2];
    s.firstPole=saturate32(int64_t(multiplyQ31(s.firstPole,0x70000000))+multiplyQ31(a,0x10000000));
    s.secondPole=saturate32(int64_t(multiplyQ31(s.secondPole,0x70000000))+multiplyQ31(b,0x10000000));
    const auto resonance=multiplyQ15(c[2],c[2]);
    const auto feedback=saturate16(int(resonance)+multiplyQ15(multiplyQ15(c[2],s.inputMeter),s.inputMeter));
    auto previous=s.firstFeedback;std::array<int32_t,64> intermediate{};
    for(size_t n=0;n<64;++n){
        const auto input=int32_t(audio[n])*16384;
        const auto difference=saturate32(int64_t(punch246Curve(previous,sine))-input);
        auto x=punch246Curve(saturate32(int64_t(input)-multiplyQ31(saturate32(int64_t(difference)*4),int32_t(feedback)*65536)),sine);
        auto total=multiplyQ31(x,mix[0]);
        for(size_t j=0;j<4;++j){const auto z=s.first[j];x=saturate32(int64_t(saturate32(int64_t(z)+multiplyQ31(x,a)))+multiplyQ31(saturate32(-int64_t(z)),a));s.first[j]=x;total=saturate32(int64_t(total)+multiplyQ31(x,mix[j+1]));}
        previous=x;intermediate[n]=punch246Curve(saturate32(int64_t(total)*4),sine);
    }
    s.firstFeedback=previous;previous=s.secondFeedback;
    for(size_t n=0;n<64;++n){
        const auto difference=saturate32(int64_t(punch246Curve(previous,sine))-intermediate[n]);
        auto x=punch246Curve(saturate32(int64_t(intermediate[n])-multiplyQ31(saturate32(int64_t(difference)*4),int32_t(resonance)*65536)),sine);
        for(size_t j=0;j<4;++j){const auto z=s.second[j];x=saturate32(int64_t(saturate32(int64_t(z)+multiplyQ31(x,b)))+multiplyQ31(saturate32(-int64_t(z)),b));s.second[j]=x;}
        previous=x;audio[n]=int16_t(floorDivide(punch246Curve(x,sine),65536));
    }
    s.secondFeedback=previous;s.outputMeter=multiplyQ15(s.outputMeter,32512);for(auto x:audio)s.outputMeter=std::max(s.outputMeter,x);
    const auto slot=size_t(s.slot)*4;meters[slot]=uint16_t(s.inputMeter)|(uint32_t(uint16_t(s.outputMeter))<<16);
    meters[slot+1]=uint32_t(i1)|(uint32_t(i2)<<16);
    return true;
}
}
