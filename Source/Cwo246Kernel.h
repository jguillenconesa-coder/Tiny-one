// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Cwo246Time.h"
namespace pocket::reference {
struct Cwo246State {
    uint32_t tag=0;int32_t delayPole=0,phase=-1073741824;
    int16_t shifted=0,sideband=0,feedback=0,reserved18=0,wet=0,dry=0,allpassCoefficient=0,reserved26=0;
    int32_t delayCurrent=0;int16_t allpass=0,reserved34=0;
    Cwo246DelayState delay;std::array<int32_t,3> filter{};Cwo246PhaseState phaseNetwork;
    std::array<std::array<int32_t,2>,4> smooth{};int32_t slot=0;int16_t meter=0,reserved234=0;
    uint32_t rng=0x43574f48u; // Authored deterministic host seed.
    int32_t clock=0;int16_t trigger=0,reserved246=0;uint32_t displayPhase=0;
};
static_assert(sizeof(Cwo246State)==252);
inline bool cwo246HybridBlock(Cwo246State& s,const std::array<int16_t,4>& controls,Cwo246DelayMemory& memory,std::array<int16_t,64>& audio,std::array<uint32_t,16>& meters) noexcept {
    if(s.slot!=0||s.phase< -1073741824||s.phase>=1073741824||s.phaseNetwork.parity>1||s.delay.write<0||s.delay.write>65535)return false;
    for(auto c:controls)if(c<0)return false;
    auto add=[](int16_t a,int16_t b){return saturate16(int(a)+b);};auto sub=[](int16_t a,int16_t b){return saturate16(int(a)-b);};
    auto abs16=[](int16_t x){return saturate16(x<0?-int(x):int(x));};auto abs32=[](int32_t x){return saturate32(x<0?-int64_t(x):int64_t(x));};
    const auto frequencyRaw=cwo246Smooth(s.smooth[0],uint16_t(controls[0]),1558264832);
    const auto sidebandRaw=cwo246Smooth(s.smooth[1],uint16_t(controls[3]),1558264832);
    const auto feedbackRaw=cwo246Smooth(s.smooth[2],uint16_t(controls[2]),1558264832);
    const auto delayRaw=cwo246Smooth(s.smooth[3],uint16_t(controls[1]),779132416);
    auto frequency=cwo246Map32(uint16_t(frequencyRaw),cwo246Frequency);const auto fb=cwo246MapFeedback(uint16_t(feedbackRaw),cwo246Feedback);
    const auto delayTarget=cwo246Map32(uint16_t(delayRaw),cwo246Delay);
    s.delayPole=saturate32(int64_t(s.delayPole)+multiplyQ31(saturate32(int64_t(delayTarget)-s.delayPole),19492726));
    const auto delay=multiplyQ31(s.delayPole,90316800);
    const auto halfSide=sub(sidebandRaw,16384),side=saturate16(2*int(halfSide));
    const auto squared=multiplyQ15(fb[3],fb[3]);const auto shape=multiplyQ15(squared,abs16(halfSide));const auto factor=saturate16(-int(add(multiplyQ15(shape,shape),-32768)));
    int32_t correction=0;
    if(abs32(frequency)<214){frequency=0;const auto distance=saturate32(int64_t(-1073741824)-s.phase);const auto magnitude=abs32(distance);
        if(magnitude<107374)s.phase=-1073741824;
        else {auto adjusted=distance;if(magnitude>=1073741824)adjusted=distance<0?saturate32(int64_t(distance)+2147483648ll):saturate32(int64_t(distance)-2147483648ll);correction=multiplyQ31(adjusted,152634);}
    }
    auto ramp16=[&](int16_t& value,int16_t target){if(abs16(sub(value,target))<1)value=target;return int16_t(floorDivide(sub(target,value),64));};
    const auto sideStep=ramp16(s.sideband,side),fbStep=ramp16(s.feedback,fb[3]),wetStep=ramp16(s.wet,multiplyQ15(fb[0],factor)),dryStep=ramp16(s.dry,multiplyQ15(fb[1],factor)),allpassStep=ramp16(s.allpassCoefficient,fb[2]);
    const auto difference=signedPhase(uint32_t(s.delayCurrent)-uint32_t(delay));const auto neg=signedPhase(0u-uint32_t(difference));if(std::max(difference,neg)<64)s.delayCurrent=delay;
    const auto delta=int32_t(floorDivide(signedPhase(uint32_t(delay)-uint32_t(s.delayCurrent)),64));
    const auto increment=saturate32(int64_t(correction)+frequency);std::array<int16_t,64> sine,cosine;if(!cwo246Oscillator(s.phase,increment,32440,sine,cosine))return false;
    s.phase=signedPhase(((uint32_t(s.phase)+uint32_t(increment)*64u+0x40000000u)&0x7fffffffu)+0xc0000000u);
    for(size_t n=0;n<64;++n){s.sideband=add(s.sideband,sideStep);s.feedback=add(s.feedback,fbStep);s.wet=add(s.wet,wetStep);s.dry=add(s.dry,dryStep);s.allpassCoefficient=add(s.allpassCoefficient,allpassStep);s.delayCurrent=signedPhase(uint32_t(s.delayCurrent)+uint32_t(delta));
        const auto ap=cwo246Allpass(s.allpass,s.shifted,s.allpassCoefficient);auto input=add(add(multiplyQ15(audio[n],16384),multiplyQ15(ap,s.feedback)),multiplyQ15(ap,262));const auto clipped=cwo246Clip(input);int16_t delayed;
        if(!cwo246DelaySample(s.delay,memory,clipped,uint32_t(std::max(1,s.delayCurrent)),delta,delayed))return false;
        const auto filtered=cwo246Filter(s.filter,delayed,12203775);std::array<int32_t,2> pair;if(!cwo246Phase(s.phaseNetwork,int32_t(filtered)*65536,int32_t(filtered)*65536,cwo246PhaseHybrid,pair))return false;
        s.shifted=sub(multiplyQ15(multiplyQ15(cwo246HighWord(pair[1]),s.sideband),sine[n]),multiplyQ15(cwo246HighWord(pair[0]),cosine[n]));
        audio[n]=saturate16(2*int(add(multiplyQ15(s.shifted,s.wet),multiplyQ15(audio[n],s.dry))));
    }
    auto next=[](uint32_t x){x^=x<<13;x^=x>>17;x^=x<<5;return x;};s.meter=multiplyQ15(s.meter,31948);
    for(int j=0;j<8;++j){s.rng=next(s.rng);s.meter=add(s.meter,abs16(multiplyQ15(audio[s.rng&63u],204)));}
    auto peak=sub(32767,s.meter);peak=multiplyQ15(peak,peak);peak=multiplyQ15(peak,peak);
    const auto meter=sub(32767,peak),phaseMeter=multiplyQ15(sine[0],sub(32767,controls[0]));meters[0]=uint16_t(meter)|(uint32_t(uint16_t(phaseMeter))<<16);
    s.clock=signedPhase(uint32_t(s.clock)+0x20000u);s.trigger=multiplyQ15(s.trigger,32686);
    if(s.clock>=delay){s.clock=0;s.trigger=32767;const auto step=std::min<int32_t>(6553,cwo246HighWord(saturate32(int64_t(frequency)*64)));s.displayPhase=(s.displayPhase+uint32_t(step))&0x7fffffffu;
        const auto word=int16_t(s.displayPhase&65535u),inverse=sub(32767,word);const auto display=add(inverse,multiplyQ15(sub(word,inverse),controls[3]));meters[1]=(meters[1]&65535u)|(uint32_t(uint16_t(display))<<16);
    }
    const auto envelope=std::max<int16_t>(0,saturate16(2*int(sub(s.trigger,16384))));meters[1]=(meters[1]&0xffff0000u)|uint16_t(envelope);return true;
}
}
