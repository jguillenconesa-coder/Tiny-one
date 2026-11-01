// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
struct Tremolo246State {
    uint32_t tag=0,volumePhase=0x80000000u;
    uint16_t volumeWord=0,volumePadding=0;
    uint32_t pitchPhase=0x80000000u;
    uint16_t pitchWord=0,pitchPadding=0;
    int16_t gate=0;uint16_t gatePadding=0;
    int32_t smooth=0;
};
static_assert(sizeof(Tremolo246State)==28);
inline int32_t tremolo246Add(int32_t a,int32_t b) noexcept {return saturate32(int64_t(a)+b);}
inline int32_t tremolo246Scale(int32_t a,int scale) noexcept {return saturate32(int64_t(a)*scale);}
inline int16_t tremolo246High(int32_t a) noexcept {return int16_t(floorDivide(a,65536));}
inline int32_t tremolo246Cubic(uint32_t phase) noexcept {
    const auto x=tremolo246Add(int32_t(floorDivide(signedPhase(phase),2)),0x40000000);
    return tremolo246Scale(tremolo246Add(multiplyQ31(multiplyQ31(x,x),x),-0x40000000),2);
}
inline uint32_t tremolo246Increment(int16_t speed,uint32_t tempo) noexcept {
    if(speed<16384)return uint32_t(floorDivide(floorDivide(signedPhase(tempo),int64_t(1)<<(unsigned(uint16_t(speed))>>11)),128));
    const int32_t x=int32_t(saturate16(int(speed)-16384))*65536;
    auto power=multiplyQ31(x,x);
    for(int i=0;i<4;++i)power=multiplyQ31(power,x);
    return uint32_t(power);
}
inline void tremolo246NoteOn(Tremolo246State& s,std::array<uint32_t,8>& display,const std::array<int16_t,8>& c) noexcept {
    s.volumePhase=s.pitchPhase=0x80000000u;s.volumeWord=s.pitchWord=0;
    s.gate=c[3]<14745?0:32767;s.smooth=signedPhase(0x80000000u);
    for(size_t i=0;i<3;++i){display[i*2]=0x80000000u;display[i*2+1]&=0xffff0000u;}
}
// One set of coefficients per original 128-sample callback. Splitting pitch
// and amplitude processing is an adapter API, checked against the same oracle.
struct Tremolo246Coefficients {
    uint32_t increment=0;
    int32_t pitchCurve=0,pitchDepth=0;
    int16_t amplitudeCurve=0,amplitudeDepth=0;
    int shape=0;
    Tremolo246Coefficients(const std::array<int16_t,8>& c,uint32_t tempo,int16_t gate) noexcept {
        increment=tremolo246Increment(c[0],tempo);
        const auto expanded=tremolo246Scale(std::max(0,tremolo246Add(int32_t(c[1])*65536,-0x40000000)),2);
        pitchCurve=multiplyQ31(multiplyQ31(expanded,expanded),expanded);
        const auto absolute=c[1]<0?saturate16(-int(c[1])):c[1];
        pitchDepth=tremolo246Scale(saturate32(int64_t(multiplyQ15(absolute,c[1]))*gate*2),2);
        shape=int(floorDivide(multiplyQ15(20480,c[7]),4096));
        const auto amount=std::max<int16_t>(0,saturate16(int(c[2])-16384));
        const auto doubled=saturate16(int(amount)*2);
        amplitudeCurve=multiplyQ15(doubled,doubled);
        amplitudeDepth=saturate16(int(multiplyQ15(c[2],gate))*2);
    }
    bool validShape() const noexcept {return shape>=0&&shape<5;}
};
inline int32_t tremolo246Wave(uint32_t phase,int shape) noexcept {
    if(shape==1)return signedPhase(phase);
    if(shape==2)return tremolo246Cubic(phase);
    if(shape==3)return signedPhase(phase)<=0?2147483647:signedPhase(0x80000000u);
    return signedPhase(phase)>-1879048192?signedPhase(0x80000000u):2147483647;
}
inline uint32_t tremolo246Pitch(Tremolo246State& s,const Tremolo246Coefficients& c,uint32_t input,
                               const std::array<int16_t,8192>& sine) noexcept {
    if(!c.validShape())return input;
    const auto phase=s.pitchPhase;s.pitchPhase+=c.increment;
    int32_t wave=tremolo246Wave(phase,c.shape);
    if(c.shape==0){wave=int32_t(sine[phaseIndex(phase)])*65536;const auto nonlinear=multiplyQ31(wave,c.pitchCurve);for(int i=0;i<4;++i)wave=tremolo246Add(wave,nonlinear);}
    const auto value=signedPhase(input);
    return uint32_t(std::max(65536,tremolo246Add(value,multiplyQ31(multiplyQ31(value,wave),c.pitchDepth))));
}
inline int16_t tremolo246Amplitude(Tremolo246State& s,const Tremolo246Coefficients& c,int16_t input,
                                   const std::array<int16_t,8192>& sine,int16_t& carrier) noexcept {
    if(!c.validShape()){carrier=0;return input;}
    const auto phase=s.volumePhase;s.volumePhase+=c.increment;
    if(c.shape==0){
        const auto fundamental=sine[phaseIndex(phase)];const auto nonlinear=multiplyQ15(fundamental,c.amplitudeCurve);
        carrier=fundamental;for(int i=0;i<4;++i)carrier=saturate16(int(carrier)+nonlinear);
    }else{
        carrier=tremolo246High(s.smooth);
        s.smooth=tremolo246Add(multiplyQ31(s.smooth,0x7f000000),multiplyQ31(tremolo246Wave(phase,c.shape),0x01000000));
    }
    return saturate16(int(input)+multiplyQ15(multiplyQ15(input,carrier),c.amplitudeDepth));
}
inline void tremolo246Finish(Tremolo246State& s,std::array<uint32_t,8>& display,
                             const Tremolo246Coefficients& c,int16_t envelope,int16_t carrier,
                             const std::array<int16_t,8192>& sine,const std::array<int32_t,24>& decay) noexcept {
    const auto pitchMeter=multiplyQ15(sine[phaseIndex(display[4])],tremolo246High(c.pitchDepth));
    display[4]+=uint32_t(tremolo246Scale(signedPhase(c.increment),128));
    const auto incHigh=tremolo246High(signedPhase(c.increment));
    const auto first=multiplyQ15(sine[phaseIndex(display[0])],c.amplitudeDepth);
    const auto second=multiplyQ15(sine[phaseIndex(display[2])],c.amplitudeDepth);
    display[0]+=uint32_t(int32_t(saturate16(int(incHigh)*16))*65536);
    display[2]+=uint32_t(int32_t(saturate16(int(incHigh)*2))*65536);
    display[6]=uint16_t(multiplyQ15(carrier,c.amplitudeDepth))|(uint32_t(uint16_t(first))<<16);
    display[7]=uint16_t(second)|(uint32_t(uint16_t(pitchMeter))<<16);
    if(envelope<16384){const auto x=int16_t(floorDivide(envelope,4));s.gate=saturate16(int(s.gate)+saturate16(int(multiplyQ15(x,x))+1));}
    else s.gate=multiplyQ15(s.gate,tremolo246High(decay[size_t(31-envelope/1024)]));
}
// Complete callback: caller supplies initial buffers, display and event order.
inline bool tremolo246Block(Tremolo246State& s,std::array<uint32_t,8>& display,
                            const std::array<int16_t,8>& controls,uint32_t tempo,int kind,
                            std::array<uint32_t,128>& pitch,std::array<int16_t,128>& audio,
                            const std::array<int16_t,8192>& sine,const std::array<int32_t,24>& decay) noexcept {
    if(kind!=0&&kind!=3)return false;
    const Tremolo246Coefficients c(controls,tempo,s.gate);
    for(auto& word:pitch)word=tremolo246Pitch(s,c,word,sine);
    int16_t carrier=0;for(auto& sample:audio)sample=tremolo246Amplitude(s,c,sample,sine,carrier);
    tremolo246Finish(s,display,c,controls[3],carrier,sine,decay);return true;
}
}
