// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
#include "Spring246Lifecycle.h"
namespace pocket::reference {
inline constexpr std::array<int,9> spring246Lengths{{382,1272,5330,3418,1848,5902,596,3490,6898}};
inline constexpr std::array<int,9> spring246Taps{{89,233,1597,977,1187,1493,199,521,2207}};
inline int16_t spring246HighWord(int32_t x) noexcept {return int16_t(signedPhase((uint32_t(x)>>16)|(x<0?0xffff0000u:0u)));}
inline int32_t spring246Fractional16(int16_t a,int16_t b) noexcept {return saturate32(int64_t(a)*b*2);}
inline bool spring246Block(Spring246State& s,const std::array<int16_t,4>& c,const std::array<int32_t,129>& frequency,
                           const std::array<int16_t,129>& low,const std::array<int16_t,4>& constants,
                           Spring246Memory& memory,int32_t& pending,std::array<int16_t,64>& audio,std::array<uint32_t,16>& meters) noexcept {
    if(s.slot!=0||pending<0||pending>2||c[0]<1344||c[0]>16448||c[1]<7744||c[1]>16448||c[2]<0||c[2]>16384||c[3]<0)return false;
    for(size_t j=0;j<9;++j)if(s.cursors[j]<0||s.cursors[j]>=spring246Lengths[j])return false;
    if(pending)return spring246DeferredClear(memory,pending);
    const int turns=c[1]/128,tone=std::min(int(c[0]/128),220-turns);
    const auto high=frequency[size_t(tone)];const auto coefficient=low[size_t(turns)];
    const auto damp=multiplyQ15(multiplyQ15(c[2],c[2]),c[2]);const auto gain=saturate16(32767-int(damp));const auto send=multiplyQ15(c[3],c[3]);
    auto add=[](int16_t a,int16_t b){return saturate16(int(a)+b);};
    auto hp=[&](size_t index,int16_t x,int16_t from){auto& cell=s.filters[index];const auto neg=saturate32(-int64_t(cell));cell=saturate32(int64_t(saturate32(int64_t(cell)+multiplyQ31(int32_t(x)*65536,high)))+multiplyQ31(neg,high));return saturate16(int(from)-spring246HighWord(cell));};
    auto lp=[&](size_t index,int16_t x){auto& cell=s.filters[index];const auto before=spring246Fractional16(coefficient,spring246HighWord(cell));cell=saturate32(int64_t(saturate32(int64_t(cell)+spring246Fractional16(coefficient,x)))-before);return multiplyQ15(gain,spring246HighWord(cell));};
    for(auto& input:audio){
        const auto main=hp(0,multiplyQ15(constants[1],s.feedback[3]),multiplyQ15(input,send));
        int16_t total=0;size_t offset=0;
        for(size_t group=0;group<3;++group){
            const size_t base=group*3,hpIndex=group==0?1:group==1?5:9;
            const auto excitation=hp(hpIndex,multiplyQ15(constants[1],s.feedback[group]),main);
            int16_t feedback=0,wet=0;
            for(size_t k=0;k<3;++k){const auto j=base+k;const int cursor=s.cursors[j],length=spring246Lengths[j],tap=(cursor+length-spring246Taps[j])%length;
                const auto attenuation=(j==2||j==4||j==5||j==8)?multiplyQ15(constants[0],constants[2]):constants[2];
                const auto inner=multiplyQ15(add(multiplyQ15(attenuation,memory[offset+size_t(cursor)]),memory[offset+size_t(tap)]),constants[3]);
                feedback=add(feedback,inner);const auto output=lp(hpIndex+1+k,inner);wet=add(wet,output);
                memory[offset+size_t(cursor)]=add(excitation,output);s.cursors[j]=(cursor+1)%length;offset+=size_t(length);
            }
            s.feedback[group]=feedback;total=add(total,wet);
        }
        s.feedback[3]=add(s.feedback[2],add(s.feedback[1],s.feedback[0]));input=add(input,total);
    }
    meters[0]=(meters[0]&0xffff0000u)|uint16_t(s.feedback[3]);return true;
}
}
