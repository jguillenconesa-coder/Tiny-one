// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
// Coupled sampler-region arithmetic. All eight destination words participate:
// changing a boundary may move loop points and clear their retained fractions.
// Numeric callback fixtures verify this independently of our host scheduler.
inline void sampler246Modulate(std::array<int16_t,8>& v,size_t target,int16_t modulation) noexcept {
    const auto add=[](int a,int b){return saturate16(a+b);};
    const auto fixed=[](int16_t coarse,int16_t fine){return saturate32(int64_t(coarse)*65536+int(fine)*2);};
    const auto high=[](int32_t x){return int16_t(floorDivide(x,65536));};
    const auto fraction=[](int32_t x){return int16_t(uint16_t(x)/2);};
    int span=add(v[3],-int(v[0]));if(span==0)span=32767;span=std::max(span,32);
    const auto delta=multiplyQ15(modulation,int16_t(span));
    if(target==0){
        const auto next=std::clamp(add(v[0],delta),int16_t(0),int16_t(32767));
        if(next>v[3])v[3]=next;
        if(next>v[2]){v[2]=next;v[6]=0;}
        const auto old=v[0];v[0]=next;
        if(v[1]==old){v[1]=next;v[5]=0;}
        else if(next>v[1]){v[1]=next;v[5]=0;}
    }else if(target==3){
        v[3]=std::min(int16_t(32767),std::max(add(v[3],delta),add(v[2],1)));
    }else if(target==1||target==5){
        const auto upper=fixed(v[2],v[6]);
        const auto next=target==1?fixed(add(v[1],delta),v[5]):saturate32(int64_t(fixed(v[1],v[5]))+int(modulation)*8);
        if(next<int32_t(v[0])*65536){v[1]=v[0];v[5]=0;}
        else if(next>upper){v[1]=v[2];v[5]=v[6];}
        else {v[1]=high(next);if(target==5)v[5]=fraction(next);}
    }else if(target==2||target==6){
        const auto old=v[2];
        const auto next=std::clamp(target==2?fixed(add(v[2],delta),v[6]):saturate32(int64_t(fixed(v[2],v[6]))+int(modulation)*8),int32_t(v[0])*65536,2147418112);
        if(next<fixed(v[1],v[5])){v[1]=high(next);v[5]=fraction(next);}
        v[2]=high(next);
        if(target==2){if(old==v[3]||int32_t(v[3])*65536<next){v[3]=high(next);v[6]=0;}}
        else {v[6]=fraction(next);if(old==v[3]||int32_t(v[3])*65536<next)v[3]=add(high(next),1);}
    }else if(target==4)v[4]=std::clamp(add(v[4],multiplyQ15(modulation,16384)),int16_t(8192),int16_t(24576));
    else if(target==7)v[7]=std::clamp(add(v[7],multiplyQ15(modulation,10485)),int16_t(0),int16_t(32767));
}
}
