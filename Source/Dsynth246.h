// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Dsynth246Lifecycle.h"
#include <cstdlib>
#include "Envelope246Tables.h"
#include "Filter246Tables.h"
namespace pocket::reference {
inline int16_t dsynth246High(int32_t x) noexcept {return int16_t(floorDivide(x,65536));}
inline int16_t dsynth246Wrap(int x) noexcept {const auto u=uint16_t(x);return int16_t(u<32768?int(u):int(u)-65536);}
inline int16_t dsynth246Abs(int16_t x) noexcept {return saturate16(x<0?-int(x):int(x));}
inline int16_t dsynth246WrappedAbs(int16_t x) noexcept {return x==-32768?x:dsynth246Abs(x);}
inline int32_t dsynth246Pole(int32_t state,int32_t input,int32_t coefficient) noexcept {
    const auto negative=saturate32(-int64_t(state));
    return saturate32(int64_t(saturate32(int64_t(state)+multiplyQ31(input,coefficient)))+multiplyQ31(negative,coefficient));
}
inline int16_t dsynth246Sine(uint32_t phase,int16_t feedback,const std::array<int16_t,8192>& sine) noexcept {
    const auto angle=dsynth246Wrap(int(phase>>16)+feedback);return sine[size_t(4096+floorDivide(angle,8))];
}
struct Dsynth246Coefficients {
    int32_t pitchScale=0,pole=0,complement=0;
    std::array<int32_t,2> envelopePole{},envelopeInput{};
    std::array<int16_t,2> feedback{},offset{};
    int16_t mixA=0,mixB=0,cross=0,shapeA=0,shapeB=0,shapeEnvelope=0,shapeOffset=0,postGain=0;
    int mode=0;
    Dsynth246Coefficients(Dsynth246State& s,const std::array<int16_t,8>& c) noexcept {
        const auto fold=[](int16_t v){return 96-std::abs(96-int(multiplyQ15(v,192)));};
        const auto decayA=adsr246Decay[size_t(fold(c[2]))],decayB=adsr246Decay[size_t(fold(c[6]))];
        const auto a=multiplyQ31(decayA,decayA),b=multiplyQ31(decayB,decayB);
        const auto excite=[](int32_t target,int32_t level){return multiplyQ31(target,saturate32(int64_t(saturate32(int64_t(INT32_MAX)-level))*32768));};
        s.envelope[2]=excite(s.envelope[2],s.envelope[0]);s.envelope[3]=excite(s.envelope[3],s.envelope[1]);
        envelopePole={saturate32(int64_t(a)+s.envelope[2]),saturate32(int64_t(b)+s.envelope[3])};
        const auto attackA=adsr246Attack[size_t(std::max(0,int(multiplyQ15(c[2],136))-68))],attackB=adsr246Attack[size_t(std::max(0,int(multiplyQ15(c[6],136))-68))];
        envelopeInput={multiplyQ31(attackA,s.envelope[2]),multiplyQ31(attackB,s.envelope[3])};
        pitchScale=multiplyQ31(int32_t(c[4])*65536,int32_t(c[4])*65536);
        for(size_t i=0;i<2;++i){const auto v=c[1+i*4];feedback[i]=saturate16(int(v)*2);auto p=multiplyQ15(v,v);p=multiplyQ15(p,v);offset[i]=multiplyQ15(p,v);}
        pole=filter246Frequency[size_t(c[7]/256)];complement=saturate32(int64_t(INT32_MAX)-pole);
        postGain=saturate16(32767-int(multiplyQ15(saturate16(int(c[7])*2),20480)));
        s.selection=saturate16(int(multiplyQ15(s.selection,32448))+multiplyQ15(c[3],319));
        mixB=int16_t((uint16_t(s.selection)*8u)&32767u);mixA=int16_t(32767-mixB);mode=c[3]/4096;
        cross=mode==3?multiplyQ15(dsynth246Wrap(-int(mixB)),mixB):mode>=4?-32768:0;
        const auto shape=int16_t((uint16_t(c[0])*4u)&32767u),other=int16_t(32767-shape);
        switch(c[0]/8192){
            case 0:shapeA=other;shapeB=shape;shapeEnvelope=32767;shapeOffset=0;break;
            case 1:shapeA=shape;shapeB=other;shapeEnvelope=other;shapeOffset=shape;break;
            case 2:shapeA=other;shapeB=shape;shapeEnvelope=dsynth246Wrap(-int(shape));shapeOffset=32767;break;
            default:shapeA=dsynth246Wrap(-int(shape));shapeB=32767;shapeEnvelope=-32767;shapeOffset=32767;break;
        }
    }
};
inline int16_t dsynth246Mix(int16_t a,int16_t b,const Dsynth246Coefficients& c) noexcept {
    const auto bitAnd=dsynth246Wrap(uint16_t(a)&uint16_t(b));
    const auto bitXor=dsynth246Wrap(uint16_t(dsynth246WrappedAbs(a))^uint16_t(b));
    const auto maximum=std::max(b,dsynth246Abs(a));
    switch(c.mode){
        case 0:return saturate16(int(multiplyQ15(c.mixA,maximum))+multiplyQ15(c.mixB,saturate16(int(b)-a)));
        case 1:return saturate16(int(saturate16(int(b)-multiplyQ15(c.mixA,a)))+multiplyQ15(c.mixB,bitAnd));
        case 2:case 6:return saturate16(int(saturate16(int(b)+multiplyQ15(c.mixA,bitAnd)))+multiplyQ15(c.mixB,bitXor));
        case 3:return saturate16(int(multiplyQ15(c.mixA,saturate16(int(b)+bitXor)))+multiplyQ15(c.mixB,maximum));
        case 4:return saturate16(int(multiplyQ15(c.mixA,maximum))+multiplyQ15(c.mixB,b));
        case 5:return saturate16(int(b)+multiplyQ15(c.mixB,bitAnd));
        case 7:return saturate16(int(saturate16(int(b)+multiplyQ15(c.mixA,bitXor)))-multiplyQ15(c.mixB,a));
    }
    return a;
}
// Caller supplies complete pitch and gain streams. Clock/voice scheduling is
// outside this independent mono reconstruction; no original instructions used.
inline bool dsynth246Block(Dsynth246State& s,Dsynth246Context& context,const std::array<int16_t,8>& controls,
                           const std::array<uint32_t,128>& pitch,const std::array<int16_t,128>& gain,
                           const std::array<int16_t,8192>& sine,std::array<int16_t,128>& audio,
                           std::array<uint32_t,128>& derived,std::array<int16_t,128>& second) noexcept {
    if(s.slot<0||s.slot>=6)return false;const Dsynth246Coefficients c(s,controls);
    for(size_t i=0;i<128;++i)derived[i]=uint32_t(multiplyQ31(signedPhase(pitch[i]),c.pitchScale));
    auto& b=s.oscillators[1];
    for(size_t i=0;i<128;++i){
        b.sample=dsynth246Sine(b.phase,multiplyQ15(c.feedback[1],b.sample),sine);b.phase+=derived[i];
        const auto input=saturate16(int(c.offset[1])+saturate16(int(c.offset[1])+b.sample));
        s.filters[1]=dsynth246Pole(s.filters[1],int32_t(input)*65536,c.pole);
        s.envelope[1]=saturate32(int64_t(c.envelopeInput[1])+multiplyQ31(s.envelope[1],c.envelopePole[1]));
        const auto amplitude=saturate16(int(c.shapeOffset)+multiplyQ15(saturate16(int(dsynth246High(s.envelope[1]))*4),c.shapeEnvelope));
        audio[i]=multiplyQ15(dsynth246High(s.filters[1]),amplitude);
    }
    auto& meter=context.peaks[size_t(s.slot)];meter[1]=saturate16(int(meter[1])+dsynth246Abs(audio[0]));
    auto& a=s.oscillators[0];int16_t amplitudeSeed=c.feedback[1];
    for(size_t i=0;i<128;++i){
        a.sample=dsynth246Sine(a.phase,multiplyQ15(c.feedback[0],a.sample),sine);
        s.envelope[0]=saturate32(int64_t(c.envelopeInput[0])+multiplyQ31(s.envelope[0],c.envelopePole[0]));
        const auto increment=saturate32(int64_t(signedPhase(pitch[i]))+int32_t(multiplyQ15(audio[i],c.cross))*65536);
        const auto amplitude=saturate16(int(c.shapeB)+multiplyQ15(saturate16(int(amplitudeSeed)*4),c.shapeA));amplitudeSeed=dsynth246High(s.envelope[0]);
        const auto bias=multiplyQ15(c.offset[0],amplitude);const auto input=saturate16(int(bias)+saturate16(int(bias)+a.sample));
        s.filters[0]=dsynth246Pole(s.filters[0],int32_t(input)*65536,c.pole);
        const auto quantized=signedPhase(uint32_t(s.filters[0])&0xffff0000u);
        s.filters[3]=multiplyQ31(c.complement,saturate32(int64_t(saturate32(int64_t(s.filters[3])+quantized))-s.filters[4]));s.filters[4]=quantized;
        second[i]=multiplyQ15(dsynth246High(s.filters[3]),amplitude);a.phase+=uint32_t(increment);
    }
    meter[0]=saturate16(int(meter[0])+dsynth246Abs(second[0]));
    for(size_t i=0;i<128;++i){
        const auto mixed=dsynth246Mix(audio[i],second[i],c);
        const auto input=saturate16(int(multiplyQ15(multiplyQ15(mixed,c.postGain),gain[i]))*2);
        s.filters[2]=dsynth246Pole(s.filters[2],int32_t(input)*65536,c.pole);
        const auto highPassedInput=int32_t(saturate16(int(dsynth246High(s.filters[2]))*8))*65536;
        s.filters[5]=multiplyQ31(c.complement,saturate32(int64_t(saturate32(int64_t(highPassedInput)+s.filters[5]))-s.filters[6]));s.filters[6]=highPassedInput;
        audio[i]=dsynth246High(s.filters[5]);
    }
    return context.capture(s.slot,audio);
}
}
