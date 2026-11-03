// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Value246.h"
#include "Sampler246Modulation.h"
#include "ValueVoice246Ranges.h"
namespace pocket::reference {
// Verified scope: FM/Digital/Phase/Dr. Wave/Cluster/String/DNA/Voltage controls and eight D-synth controls and common ADSR, colours
// 0..3 (D-synth and sampler Instrument 0..7). Sampler regions are coupled;
// other auxiliary/drum/play-mode routes are pending.
inline int16_t valueVoice246Adjust(int16_t base,int16_t modulation,const std::array<int16_t,4>& range) noexcept {
    auto combined=saturate16(int(base)+multiplyQ15(modulation,range[2]));
    if(range[3]==1){
        if(combined<range[0])combined=saturate16(int(combined)+saturate16(int(range[1])-range[0]));
        else if(combined>range[1])combined=saturate16(int(combined)+saturate16(int(range[0])-range[1]));
    }else combined=std::clamp(combined,range[0],range[1]);
    return combined;
}
inline bool valueVoice246Step(Value246State& state,std::array<uint32_t,8>& display,
                              const std::array<int16_t,8>& controls,uint32_t tempo,int kind,int type,
                              std::array<int16_t,8>& targets,const std::array<int16_t,8192>& sine) noexcept {
    const auto destination=uint16_t(controls[2])>>12;
    const bool extended=kind==0&&(type==12||type==6);
    const auto colour=uint16_t(controls[3])>>(extended?11:12);
    if(!((destination==0&&kind==0)||(destination==1&&kind==1)))return false;
    const auto index=valueVoice246RangeIndex(type);
    if(index<0||colour>=(extended?8u:4u)||controls[0]<0||controls[0]>16384)return false;
    const auto modulation=value246Advance(state,display,controls,tempo,sine);
    const auto& descriptor=valueVoice246Ranges[size_t(index)][colour];
    if(kind==0&&type==6){sampler246Modulate(targets,colour,modulation);return true;}
    targets[colour]=kind==1?value246Adjust(targets[colour],modulation,valueVoice246EnvelopeRanges[colour]):valueVoice246Adjust(targets[colour],modulation,descriptor);
    return true;
}
}
