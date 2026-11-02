// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "String246.h"
namespace pocket::reference {
struct Dna246State {
    uint32_t tag=0x019102a4u,phaseA=0,phaseB=0,counter=0,unknown16=0;
    int16_t seed=0,padding=0;int32_t filter=0;uint32_t flags=0;
    void construct() noexcept {tag=0x019102a4u;phaseA=phaseB=0;filter=0;seed=0;flags&=0xffffff00u;}
    void reset() noexcept {phaseA=phaseB=counter=0;seed=0;flags&=0xffffff00u;}
    void release() noexcept {flags=(flags&0xffffff00u)|1u;}
};
static_assert(sizeof(Dna246State)==32);
using Dna246Pool=std::array<Dna246State,6>;
using Dna246Wave=std::array<int16_t,12288>;
inline int16_t dna246Half(uint32_t value) noexcept {const auto x=value&65535u;return int16_t(x<32768?int(x):int(x)-65536);}
inline constexpr std::array<char,32> dna246VirtualIdentity{{'7','0','6','f','6','3','6','b','6','5','7','4','6','f','6','e','6','5','2','d','6','4','6','e','6','1','2','d','7','6','3','0'}};
// Exact post-OTP arithmetic, supplied with explicit identity bytes, not a ROM port.
inline void dna246Identity(Dna246Wave& wave,const std::array<char,32>& identity) noexcept {
    wave.fill(0);
    for(size_t group=0;group<4;++group){uint32_t word=0;for(unsigned j=0;j<8;++j)word+=uint32_t(int(static_cast<unsigned char>(identity[group*8+j]))-48)<<(4*j);
        for(unsigned bit=0;bit<32;++bit){const auto value=dna246Half(word>>bit);wave[group*64+31-bit]=value;wave[group*64+32+bit]=value;}}
}
struct Dna246Context {
    Dna246Pool pool{};Dna246Wave wave{};uint32_t feedback=0x40000000u;
    void prepare(const std::array<char,32>& identity=dna246VirtualIdentity) noexcept {pool={};feedback=0x40000000u;dna246Identity(wave,identity);}
};
inline uint32_t dna246Period(uint32_t pitch) noexcept {return pitch?uint32_t(std::clamp(signedPhase(uint32_t((uint64_t(1)<<46)/pitch)),int32_t(0x20000),int32_t(0x30000000))):0x30000000u;}
inline int16_t dna246Shape(int16_t x) noexcept {const auto linear=multiplyQ15(x,31129),cube=multiplyQ15(multiplyQ15(x,x),x);return saturate16(int(linear)+saturate16(int(linear)-cube));}
// The observed halfword SIGNBITS/ASHIFT sequence shifts nonzero input by 15.
inline int16_t dna246Normalize(int16_t x) noexcept {return saturate16(int(x)*32768);}
inline uint8_t dna246PoolByte(const Dna246Pool& pool,size_t index) noexcept {return reinterpret_cast<const unsigned char*>(pool.data())[index];}
// Controlled source-memory reader: callback aliases live pool bytes. The host
// deliberately models all other scanned L1 bytes as zero, not recovered memory.
inline uint8_t dna246VirtualByte(const Dna246Pool& pool,size_t index) noexcept {return index<sizeof(pool)?dna246PoolByte(pool,index):uint8_t(0);}
template<class Reader>
inline bool dna246Block(Dna246Context& context,size_t slot,const std::array<int16_t,4>& c,uint32_t pitch,uint32_t& rng,std::array<int16_t,128>& audio,Reader&& readByte) noexcept {
    if(slot>=6||pitch<0x40000u||pitch>0x7fffffffu||c[0]<-29491||c[1]<4608||c[1]>12800||c[2]<0||c[3]<0)return false;
    auto& s=context.pool[slot];const auto period=dna246Period(pitch);
    if(period>0x18000000u||s.phaseA>=period||s.phaseB>=2*period||s.counter>=0x7fffff00u)return false;
    const auto gain=int16_t(std::max(16384,int(saturate16(32767-int(saturate16(int(c[0])+24576))))));
    const auto positive=uint16_t(std::max(0,int(c[0])));const auto power=multiplyQ15(multiplyQ15(multiplyQ15(c[3],c[3]),c[3]),c[3]);
    const int shift=int(floorDivide(multiplyQ15(c[2],c[1]),1024)),aShift=16-c[1]/1024;
    const uint32_t stepA=uint32_t(1)<<unsigned(aShift),stepB=uint32_t(std::max(1,int((32768-c[2])>>(shift+aShift/2))))<<unsigned(shift);
    const auto filterCoefficient=spring246Low[size_t(saturate16(int(c[0])+32767)/256)];const auto length=period>>16;
    int16_t previous=s.seed;
    for(auto& output:audio){
        const auto base=31-int((s.phaseA&65535u)>>11);const auto fraction=int16_t((s.phaseA&2047u)*16u),inverse=int16_t(32767-fraction);
        auto a=s.phaseA>>16,b=s.phaseB>>16;int32_t accumulator=0;
        for(size_t j=0;j<7;++j){const auto index=size_t(base)+j*32;const auto weight=saturate16(int(multiplyQ15(delay246Interpolation[index],fraction))+multiplyQ15(delay246Interpolation[index+1],inverse));
            accumulator=saturate32(int64_t(accumulator)+string246Product(weight,context.wave[a]));if(++a>=length)a-=length;
            accumulator=saturate32(int64_t(accumulator)+string246Product(weight,context.wave[b]));if(++b>=length)b-=length;
        }
        const auto shaped=dna246Shape(string246High(accumulator));const auto oldCounter=s.counter;s.counter+=1;
        const auto driver=int16_t(readByte(context.pool,size_t(oldCounter>>unsigned(c[2]/2048))));
        const auto wrappedNegative=dna246Half(0u-uint16_t(previous));const auto absolute=std::max(previous,wrappedNegative);
        const auto feedback=dna246Half(positive^uint16_t(absolute));const auto mixture=dna246Half(positive^uint16_t(saturate16(int(shaped)+feedback)));
        previous=dna246Normalize(shaped);
        const auto noise=dna246Normalize(multiplyQ15(multiplyQ15(power,driver),string246High(signedPhase(string246Random(rng)))));
        const auto input=saturate16(int(multiplyQ15(mixture,gain))+noise);
        s.filter=string246Filter(s.filter,input,filterCoefficient);
        output=multiplyQ15(dna246Shape(string246High(s.filter)),gain);
        if((s.counter>>unsigned(c[2]/2048))>=(period>>12))s.counter=0;
        const auto oldA=s.phaseA,nextA=oldA+stepA,nextB=s.phaseB+stepB;
        s.phaseA=nextA>=period?nextA-period:nextA;
        // Both cursor wrap comparisons use the zero-valued data register R1.
        s.phaseB=nextB>=period?nextB-period:nextB;if(signedPhase(nextB)<0)s.phaseB=nextB+period;
    }
    s.seed=previous;context.feedback=(context.feedback&0xffff0000u)|uint16_t((s.flags&255u)?0:previous);
    return true;
}
inline bool dna246Block(Dna246Context& context,size_t slot,const std::array<int16_t,4>& controls,uint32_t pitch,uint32_t& rng,std::array<int16_t,128>& audio) noexcept {return dna246Block(context,slot,controls,pitch,rng,audio,dna246VirtualByte);}
}
