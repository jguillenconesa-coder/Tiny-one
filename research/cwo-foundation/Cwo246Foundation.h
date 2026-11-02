// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
// Building blocks only: not the complete CWO callback or its host adapter.
inline int16_t cwo246HighWord(int32_t x) noexcept {return int16_t(signedPhase((uint32_t(x)>>16)|(x<0?0xffff0000u:0u)));}
inline int32_t cwo246Map32(uint16_t raw,const std::array<int32_t,257>& table) noexcept {
    const auto index=size_t(raw>>7);if(raw>32767)return 0;
    const auto delta=saturate32(int64_t(table[index+1])-table[index]);
    return saturate32(int64_t(table[index])+multiplyQ31(delta,int32_t(raw&127)*16777216));
}
inline std::array<int16_t,4> cwo246MapFeedback(uint16_t raw,const std::array<std::array<int16_t,4>,257>& table) noexcept {
    std::array<int16_t,4> result{};if(raw>32767)return result;const auto index=size_t(raw>>7);const auto fraction=int16_t((raw&127)*256);
    for(size_t j=0;j<4;++j)result[j]=saturate16(int(table[index][j])+multiplyQ15(saturate16(int(table[index+1][j])-table[index][j]),fraction));return result;
}
inline int16_t cwo246Clip(int16_t& x) noexcept {
    const auto doubled=saturate16(int(x)*2);auto cubic=multiplyQ15(10922,doubled);cubic=multiplyQ15(cubic,doubled);cubic=multiplyQ15(cubic,doubled);
    x=saturate16(int(doubled)-cubic);return multiplyQ15(x,16384);
}
inline int16_t cwo246Allpass(int16_t& state,int16_t input,int16_t coefficient) noexcept {
    auto a=saturate16(int(state)+multiplyQ15(state,coefficient));
    auto b=saturate16(int(a)+multiplyQ15(saturate16(int(a)-input),coefficient));
    a=saturate16(int(a)+b);state=saturate16(int(b)+multiplyQ15(b,coefficient));return saturate16(int(a)+state);
}
inline int16_t cwo246Filter(std::array<int32_t,3>& s,int16_t input,int32_t coefficient) noexcept {
    const auto first=multiplyQ31(int32_t(input)*65536,0x3fae1480);
    const auto sum=saturate32(int64_t(first)+multiplyQ31(s[1],0x3fae1480));s[1]=int32_t(input)*65536;
    const auto error=saturate32(int64_t(sum)-multiplyQ31(s[0],0x7f5c2900));
    s[0]=saturate32(int64_t(error)+saturate32(int64_t(s[0])+error));
    s[2]=saturate32(int64_t(s[2])+multiplyQ31(saturate32(int64_t(s[0])-s[2]),coefficient));
    return cwo246HighWord(saturate32(int64_t(s[0])-s[2]));
}
inline int16_t cwo246Smooth(std::array<int32_t,2>& s,uint16_t raw,int32_t coefficient) noexcept {
    if(raw>32767)return 0;
    const auto magnitude=saturate32(s[0]<0?-int64_t(s[0]):int64_t(s[0]));
    const auto k=multiplyQ31(saturate32(int64_t(saturate32(int64_t(magnitude)*2))+0x147ae14),coefficient);
    const auto target=int32_t(raw)*65536;
    const auto acceleration=saturate32(int64_t(saturate32(int64_t(saturate32(int64_t(target)-s[0]))-multiplyQ31(s[0],0x60000000)))-s[1]);
    s[0]=saturate32(int64_t(s[0])+multiplyQ31(k,acceleration));s[1]=saturate32(int64_t(s[1])+multiplyQ31(k,s[0]));
    const auto difference=saturate32(int64_t(s[1])-target);const auto distance=saturate32(difference<0?-int64_t(difference):int64_t(difference));
    if(distance<0x1a36e)s[1]=target;return std::max(int16_t(0),cwo246HighWord(s[1]));
}
struct Cwo246PhaseState {uint32_t parity=0;int32_t delayed=0;std::array<int32_t,32> cells{};};
static_assert(sizeof(Cwo246PhaseState)==136);
// Caller supplies all coefficients. Original comparisons using substituted
// coefficients establish arithmetic only, not the original missing phase data.
inline bool cwo246Phase(Cwo246PhaseState& s,int32_t a,int32_t b,const std::array<int32_t,8>& coefficients,std::array<int32_t,2>& out) noexcept {
    if(s.parity>1)return false;const auto old=s.delayed;s.delayed=b;b=old;
    for(size_t j=0;j<4;++j){const auto index=j*2+s.parity;
        const auto ya=saturate32(int64_t(multiplyQ31(saturate32(int64_t(a)+s.cells[index+8]),coefficients[j]))-s.cells[index]);
        const auto yb=saturate32(int64_t(multiplyQ31(saturate32(int64_t(b)+s.cells[index+24]),coefficients[j+4]))-s.cells[index+16]);
        s.cells[index]=a;s.cells[index+8]=ya;s.cells[index+16]=b;s.cells[index+24]=yb;a=ya;b=yb;
    }
    s.parity^=1;out={{a,b}};return true;
}
}
