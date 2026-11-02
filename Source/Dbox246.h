// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Dsynth246.h"
namespace pocket::reference {
// Independently reconstructed mono callback. Pitch, gain and scheduling are inputs.
struct Dbox246State {
    uint32_t tag=0;std::array<int32_t,4> envelope{};int32_t slot=0;uint32_t blocks=0;
    std::array<Dsynth246State::Oscillator,2> oscillators{};std::array<int32_t,7> filters{};
    void construct() noexcept {tag=0x0190f8a4u;envelope.fill(0);for(auto& o:oscillators){o.phase=0x80000000u;o.sample=0;}filters.fill(0);}
    bool bind(int position) noexcept {if(position<0||position>=12)return false;slot=position;return true;}
    void release() noexcept {envelope[2]=envelope[3]=0;}
    uint32_t active() noexcept {if(saturate32(int64_t(envelope[0])+envelope[1])<16)blocks=0;return blocks;}
};
static_assert(sizeof(Dbox246State)==72);
struct Dbox246Context {
    std::array<int32_t,2> constants{};std::array<std::array<int16_t,2>,12> peaks{};
    std::array<std::array<int16_t,64>,12> history{};std::array<uint32_t,12> counts{};
    void initialize() noexcept {constants[0]=0x00a3d700;constants[1]=signedPhase((uint32_t(constants[1])&0xffff0000u)|0x4000u);peaks={};history={};}
    bool reset(Dbox246State& s) noexcept {if(s.slot<0||s.slot>=12)return false;s.envelope={0,0,INT32_MAX,INT32_MAX};s.blocks=0;for(auto& o:s.oscillators){o.phase=0x80000000u;o.sample=0;}s.filters.fill(0);history[size_t(s.slot)].fill(0);counts[size_t(s.slot)]=0;return true;}
    bool decayMeter(int slot) noexcept {if(slot<0||slot>=12)return false;for(auto& x:peaks[size_t(slot)])x=multiplyQ15(x,28672);return true;}
    bool resetCapture(int slot) noexcept {if(slot<0||slot>=12)return false;counts[size_t(slot)]=0;return true;}
    bool capture(int slot,const std::array<int16_t,128>& audio,int32_t initialEnvelope,unsigned wraps) noexcept {
        if(slot<0||slot>=12)return false;auto& n=counts[size_t(slot)];
        if((signedPhase(n)>0&&signedPhase(n)<60)||(n==0&&(wraps!=0||initialEnvelope==0))){
            if(n>60)return false;for(size_t i=0;i<4;++i)history[size_t(slot)][n++]=audio[i*32];
        }return true;
    }
};
static_assert(sizeof(Dbox246Context)==1640);
struct Dbox246Coefficients {
    int32_t pitchScale=0,pole=0,complement=0;std::array<int32_t,2> envelopePole{},envelopeInput{};
    std::array<int16_t,2> feedback{},offset{};int16_t cross=0,postGain=0,mixGain=0;int mode=0;
    Dbox246Coefficients(Dbox246State& s,const std::array<int16_t,8>& c,const std::array<int16_t,8192>& sine) noexcept {
        for(size_t i=0;i<2;++i){const auto v=c[2+i*4];const int centre=i?94:96;const auto decay=adsr246Decay[size_t(std::max(0,centre-std::abs(centre-int(multiplyQ15(v,192)))))];
            s.envelope[2+i]=multiplyQ31(s.envelope[2+i],saturate32(int64_t(saturate32(int64_t(INT32_MAX)-s.envelope[i]))*32768));
            envelopePole[i]=saturate32(int64_t(multiplyQ31(decay,decay))+s.envelope[2+i]);
            envelopeInput[i]=multiplyQ31(adsr246Attack[size_t(std::max(0,int(multiplyQ15(v,136))-68))],s.envelope[2+i]);
            const auto f=c[1+i*4];feedback[i]=saturate16(int(f)*2);auto p=multiplyQ15(f,f);p=multiplyQ15(p,f);offset[i]=multiplyQ15(p,f);
        }
        pitchScale=multiplyQ31(int32_t(c[4])*65536,int32_t(c[4])*65536);pole=filter246Frequency[size_t(c[7]/256)];complement=saturate32(int64_t(INT32_MAX)-pole);
        postGain=saturate16(32767-int(multiplyQ15(saturate16(int(c[7])*2),20480)));mixGain=sine[uint16_t(multiplyQ15(c[3],14336))&4095u];mode=c[3]/4096;cross=c[3]>=16384?-32768:0;
    }
};
inline int16_t dbox246Mix(int16_t a,int16_t b,int mode) noexcept {
    switch(mode){case 0:case 4:return std::max(dsynth246Abs(a),b);case 1:return saturate16(int(b)-a);case 2:case 6:return saturate16(int(b)+dsynth246Wrap(uint16_t(a)&uint16_t(b)));case 3:case 7:return saturate16(int(b)+dsynth246Wrap(uint16_t(dsynth246WrappedAbs(a))^uint16_t(b)));case 5:return b;}return a;
}
inline bool dbox246Block(Dbox246State& s,Dbox246Context& context,const std::array<int16_t,8>& controls,
                        const std::array<uint32_t,128>& pitch,const std::array<int16_t,128>& gain,
                        const std::array<int16_t,8192>& sine,std::array<int16_t,128>& audio,
                        std::array<uint32_t,128>& derived,std::array<int16_t,128>& second) noexcept {
    if(s.slot<0||s.slot>=12)return false;const auto initialEnvelope=s.envelope[0];const Dbox246Coefficients c(s,controls,sine);
    for(size_t i=0;i<128;++i)derived[i]=uint32_t(multiplyQ31(signedPhase(pitch[i]),c.pitchScale));
    auto& b=s.oscillators[1];
    for(size_t i=0;i<128;++i){b.sample=dsynth246Sine(b.phase,multiplyQ15(c.feedback[1],b.sample),sine);b.phase+=derived[i];
        const auto bias=c.offset[1];const auto input=saturate16(int(bias)+saturate16(int(bias)+multiplyQ15(sine[uint16_t(controls[3])&4095u],b.sample)));
        s.filters[1]=dsynth246Pole(s.filters[1],int32_t(input)*65536,c.pole);s.envelope[1]=saturate32(int64_t(c.envelopeInput[1])+multiplyQ31(s.envelope[1],c.envelopePole[1]));
        audio[i]=multiplyQ15(dsynth246High(s.filters[1]),saturate16(int(dsynth246High(s.envelope[1]))*4));
    }
    auto& meter=context.peaks[size_t(s.slot)];meter[1]=saturate16(int(meter[1])+dsynth246Abs(audio[0]));auto& a=s.oscillators[0];unsigned wraps=0;
    for(size_t i=0;i<128;++i){a.sample=dsynth246Sine(a.phase,multiplyQ15(c.feedback[0],a.sample),sine);
        s.envelope[0]=saturate32(int64_t(c.envelopeInput[0])+multiplyQ31(s.envelope[0],c.envelopePole[0]));const auto amplitude=saturate16(int(dsynth246High(s.envelope[0]))*4);
        const auto increment=int32_t(floorDivide(saturate32(int64_t(signedPhase(pitch[i]))+int32_t(multiplyQ15(audio[i],c.cross))*65536),16));const auto previous=a.phase;a.phase+=uint32_t(increment);if(signedPhase(a.phase)<signedPhase(previous))++wraps;
        const auto bias=multiplyQ15(c.offset[0],amplitude);const auto input=saturate16(int(bias)+saturate16(int(bias)+a.sample));s.filters[0]=dsynth246Pole(s.filters[0],int32_t(input)*65536,c.pole);
        const auto quantized=signedPhase(uint32_t(s.filters[0])&0xffff0000u);s.filters[3]=multiplyQ31(c.complement,saturate32(int64_t(saturate32(int64_t(s.filters[3])+quantized))-s.filters[4]));s.filters[4]=quantized;
        second[i]=multiplyQ15(dsynth246High(s.filters[3]),amplitude);
    }
    meter[0]=saturate16(int(meter[0])+dsynth246Abs(second[0]));
    for(size_t i=0;i<128;++i){const auto mixed=dbox246Mix(audio[i],second[i],c.mode);const auto input=multiplyQ15(multiplyQ15(multiplyQ15(mixed,c.mixGain),c.postGain),gain[i]);
        s.filters[2]=dsynth246Pole(s.filters[2],int32_t(input)*65536,c.pole);const auto highPassedInput=int32_t(saturate16(int(dsynth246High(s.filters[2]))*4))*65536;
        s.filters[5]=multiplyQ31(c.complement,saturate32(int64_t(saturate32(int64_t(highPassedInput)+s.filters[5]))-s.filters[6]));s.filters[6]=highPassedInput;audio[i]=saturate16(int(dsynth246High(s.filters[5]))*4);
    }
    ++s.blocks;return context.capture(s.slot,audio,initialEnvelope,wraps);
}
}
