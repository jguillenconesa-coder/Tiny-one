// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Fm246Foundation.h"
#include "Fm246Model.h"
namespace pocket::reference {
// Numerical reconstruction with ten explicit authored table substitutions.
struct Fm246Workspace {std::array<int16_t,128> a{},b{};};
struct Fm246Coefficients {
    std::array<uint32_t,4> increments{};
    std::array<int16_t,3> levels{}; // B,C,D
    int16_t feedback=0;int algorithm=0;
    Fm246Coefficients(const Fm246State& state,const std::array<int16_t,4>& c,
                      uint32_t pitch,const std::array<int16_t,30>& table) noexcept {
        const auto fields=decodeFmFields(uint16_t(c[1]),uint16_t(c[2]));algorithm=fields.selector;
        const auto add=[](int16_t a,int16_t b){return saturate16(int(a)+b);};
        const auto sub=[](int16_t a,int16_t b){return saturate16(int(a)-b);};
        const auto shift=[](int16_t a,int bits){return saturate16(int(a)*(1<<bits));};
        const auto blue=shift(c[0],1),excess=std::max<int16_t>(sub(c[0],8192),0);
        feedback=multiplyQ15(multiplyQ15(excess,excess),excess);
        for(size_t i=0;i<3;++i)levels[2-i]=shift(add(blue,multiplyQ15(blue,state.gains[3+i])),1);
        std::array<int16_t,3> bases{},motion{};
        for(size_t i=0;i<3;++i){
            bases[i]=multiplyQ15(int16_t(pitch>>16),table[6+i*8+size_t(fields.tableIndices[i])]);
            motion[i]=multiplyQ15(bases[i],int16_t(floorDivide(c[3],8)));
        }
        const auto plus=[&](size_t bank,size_t gain){return shift(add(add(bases[bank],motion[bank]),multiplyQ15(motion[bank],state.gains[gain])),2);};
        const auto minus=[&](size_t bank,size_t gain){return shift(sub(sub(bases[bank],motion[bank]),multiplyQ15(motion[bank],state.gains[gain])),2);};
        const auto word=[](int16_t value){return uint32_t(uint16_t(value))<<16;};
        increments[0]=pitch&0xffff0000u;
        if(algorithm==5||algorithm==8){
            increments[3]=word(plus(2,2));increments[2]=word(minus(1,1));increments[1]=word(plus(0,0));
            std::swap(levels[0],levels[2]);
        }else if(algorithm==6){
            increments[3]=word(minus(1,1));increments[2]=word(plus(0,0));increments[1]=word(plus(2,2));
            const auto original=levels;levels={original[0],original[2],original[1]};
        }else {increments[3]=word(plus(0,0));increments[2]=word(minus(1,1));increments[1]=word(plus(2,2));}
    }
};
inline int16_t fm246Wrap(int value) noexcept {
    const auto bits=uint16_t(value);return int16_t(bits<32768?int(bits):int(bits)-65536);
}
inline void fm246Operator(Fm246Operator& state,uint32_t increment,int16_t feedback,
                          int16_t level,const std::array<int16_t,128>* input,
                          std::array<int16_t,128>& output,bool accumulate,
                          const std::array<int16_t,8192>& sine) noexcept {
    for(size_t n=0;n<128;++n){
        const int modulation=input?(*input)[n]:0;
        const auto phase=fm246Wrap(int(state.phase>>16)+modulation+multiplyQ15(feedback,state.sample));
        state.sample=sine[size_t(4096+floorDivide(phase,8))];state.phase+=increment;
        const auto value=int16_t(floorDivide(multiplyQ15(level,state.sample),4));
        output[n]=accumulate?saturate16(int(output[n])+value):value;
    }
}
// Mono block; invalid selectors advance contours but preserve output/scratch.
inline bool fm246Block(Fm246State& state,Fm246Workspace& work,
                       const std::array<int16_t,4>& controls,uint32_t pitch,
                       const std::array<int16_t,30>& table,const std::array<int16_t,8192>& sine,
                       std::array<int16_t,128>& output) noexcept {
    fm246AdvanceEnvelopes(state,multiplyQ15(controls[3],controls[3]),table,sine);
    const Fm246Coefficients c(state,controls,pitch,table);
    if(c.algorithm<0||c.algorithm>8)return false;
    auto render=[&](size_t op,const std::array<int16_t,128>* in,std::array<int16_t,128>& out,bool sum=false){
        fm246Operator(state.operators[op],c.increments[op],c.feedback,c.levels[op-1],in,out,sum,sine);
    };
    render(3,nullptr,work.a);
    switch(c.algorithm){
        case 0:render(2,&work.a,work.b);render(1,&work.b,work.a);break;
        case 1:render(2,nullptr,work.a,true);render(1,&work.a,work.b);break;
        case 2:render(2,&work.a,work.b);render(1,nullptr,work.b,true);break;
        case 3:case 7:render(2,&work.a,work.b);render(1,&work.a,work.b,true);break;
        case 4:render(2,&work.a,work.b);render(1,&work.b,work.a);break;
        case 5:case 8:render(2,nullptr,work.a,true);render(1,nullptr,work.a,true);break;
        case 6:render(2,nullptr,work.b);render(1,&work.a,work.a);break;
    }
    const bool parallel=c.algorithm==4||c.algorithm>=6;
    const auto* modulation=(c.algorithm==1||c.algorithm==2||c.algorithm==3||c.algorithm==4||c.algorithm==6)?&work.b:
                           c.algorithm==8?nullptr:&work.a;
    const auto& mix=c.algorithm==4||c.algorithm==6||c.algorithm==8?work.a:work.b;
    auto& carrier=state.operators[0];
    for(size_t n=0;n<128;++n){
        const auto phase=fm246Wrap(int(carrier.phase>>16)+(modulation?(*modulation)[n]:0)+multiplyQ15(c.feedback,carrier.sample));
        carrier.sample=sine[size_t(4096+floorDivide(phase,8))];carrier.phase+=c.increments[0];
        output[n]=parallel?saturate16(int(floorDivide(carrier.sample,2))+int(floorDivide(mix[n],2))):carrier.sample;
    }
    return true;
}
}
