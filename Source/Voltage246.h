// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "String246.h"
namespace pocket::reference {
struct Voltage246State {
    uint32_t tag=0;
    struct Oscillator {uint32_t phase=0x80000000u;int16_t sample=0;uint16_t padding=0;};
    std::array<Oscillator,4> oscillators{};
    int32_t filter=0;int16_t charge=0,previous=0;int32_t detune=0;
    uint8_t released=0;std::array<uint8_t,3> padding{};
    void construct() noexcept {tag=0x0191185cu;for(auto& o:oscillators){o.phase=0x80000000u;o.sample=0;}filter=0;charge=previous=0;detune=0;released=0;}
    void reset() noexcept {for(auto& o:oscillators){o.phase=0x80000000u;o.sample=0;}filter=0;charge=previous=16384;detune=0;released=0;}
    void release() noexcept {released=1;}
};
static_assert(sizeof(Voltage246State)==52);
using Voltage246Pool=std::array<Voltage246State,6>;
inline int16_t voltage246Half(int value) noexcept {const auto bits=uint16_t(value);return int16_t(bits<32768?int(bits):int(bits)-65536);}
inline int16_t voltage246Shape(int16_t value) noexcept {
    auto curve=multiplyQ15(9831,value);curve=multiplyQ15(curve,value);curve=multiplyQ15(curve,value);
    const auto shaped=saturate16(int(value)-curve);
    return saturate16(int(shaped)+multiplyQ15(16384,shaped));
}
inline int16_t voltage246Sine(uint32_t phase,int16_t modulation,const std::array<int16_t,8192>& sine) noexcept {
    const auto angle=voltage246Half(int(phase>>16)+modulation);
    return sine[size_t(4096+floorDivide(angle,8))];
}
// Independent mono reconstruction. Caller owns pitch, sine and display; no
// physical identity, external RAM reader or RNG belongs to this callback.
inline void voltage246Block(Voltage246State& state,const std::array<int16_t,4>& c,
                            const std::array<uint32_t,128>& pitch,const std::array<int16_t,8192>& sine,
                            std::array<int16_t,128>& audio,std::array<int16_t,32>& display) noexcept {
    const auto inverse=saturate16(32767-int(c[2]));
    const auto tilt=int16_t(floorDivide(multiplyQ15(inverse,inverse),16));
    const auto coefficient=spring246Low[size_t(inverse>17000?128-(inverse-17000)/128:128)];
    const auto spread=int32_t(int16_t(floorDivide(multiplyQ15(c[3],c[3]),2)))*65536+65536;
    for(size_t n=0;n<128;++n){
        auto& a=state.oscillators[0];auto& b=state.oscillators[1];auto& d=state.oscillators[2];auto& e=state.oscillators[3];
        const auto offset=multiplyQ31(signedPhase(pitch[n]),spread);
        const auto higher=saturate32(int64_t(saturate32(int64_t(signedPhase(pitch[n]))+offset))-state.detune);
        const auto lower=saturate32(int64_t(state.detune)+saturate32(int64_t(signedPhase(pitch[n]))-offset));
        state.detune=saturate32(int64_t(state.detune)+(state.released?1:0));
        const auto oldB=b.phase;b.phase+=pitch[n];const bool crossing=signedPhase(b.phase)<signedPhase(oldB);
        const auto carrier=voltage246Sine(oldB,0,sine);
        d.sample=voltage246Sine(d.phase,multiplyQ15(c[1],d.sample),sine);d.phase+=uint32_t(lower);
        auto mix=voltage246Shape(d.sample);
        if(crossing)state.charge=multiplyQ15(state.charge,16384);
        const auto carrierHalf=int16_t(floorDivide(multiplyQ15(16384,carrier),1));
        a.sample=voltage246Sine(a.phase,multiplyQ15(c[1],a.sample),sine);a.phase+=pitch[n];
        e.sample=voltage246Sine(e.phase,multiplyQ15(c[1],e.sample),sine);e.phase+=uint32_t(higher);
        const auto absA=saturate16(a.sample<0?-int(a.sample):int(a.sample));
        const auto absE=saturate16(e.sample<0?-int(e.sample):int(e.sample));
        const auto shapedA=voltage246Shape(absA),shapedE=voltage246Shape(absE);
        const auto xorValue=voltage246Half(int(uint16_t(shapedA)^uint16_t(shapedE)));
        const auto ring=voltage246Shape(multiplyQ15(c[0],xorValue));
        state.charge=saturate16(int(state.charge)-multiplyQ15(shapedA,tilt));
        mix=saturate16(int(multiplyQ15(4096,mix))+multiplyQ15(8192,ring));
        mix=saturate16(int(mix)+multiplyQ15(16384,state.charge));
        state.filter=string246Filter(state.filter,mix,coefficient);
        const auto next=saturate16(int(carrierHalf)+string246High(state.filter));
        audio[n]=saturate16(int(floorDivide(next,2))+int(floorDivide(state.previous,2)));state.previous=next;
    }
    for(size_t i=0;i<32;++i)display[i]=audio[i*4];
}
}
