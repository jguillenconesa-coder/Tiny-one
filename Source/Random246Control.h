// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Random246.h"
namespace pocket {
inline constexpr std::array<const char*,4> random246Ids{{"random246Amount","random246Speed","random246Destination","random246Envelope"}};
// Authored host clock and shared RNG scheduling; the original L1 scheduler is unknown.
struct Random246Control {
    reference::Random246State state;
    double position=1;uint64_t ticks=0;uint32_t meter=0;
    std::array<int16_t,8> modulation{};bool active=false,pendingNote=false;
    void reset() noexcept {*this={};}
    void noteOn() noexcept {pendingNote=true;}
    template<size_t Count=4>
    std::array<int16_t,Count> next(bool enabled,const std::array<int16_t,4>& raw,int type,double clockStep,
                               uint32_t& rng,const std::array<int16_t,Count>& base) noexcept {
        const int index=reference::valueVoice246RangeIndex(type);
        if(!enabled||index<0){if(active)reset();return base;}
        active=true;const bool envelope=raw[2]>=2048;
        const std::array<int16_t,8> c{{raw[0],int16_t(std::max(0,int(raw[1]))),int16_t(envelope?2048:0),int16_t(std::max(0,int(raw[3]))),0,0,0,0}};
        if(pendingNote){state.noteOn(c[3]);pendingNote=false;}
        if(position>=1){
            const auto amount=reference::multiplyQ15(state.gate,c[0]);std::array<int16_t,8> targets{};
            reference::random246VoiceStep(state,rng,meter,c,envelope?1:0,type,targets);
            for(size_t i=0;i<8;++i)modulation[i]=reference::multiplyQ15(state.smooth[i],amount);
            position-=1;++ticks;
        }
        position+=clockStep;auto result=base;
        const bool extended=type==12&&!envelope&&Count==8;
        for(size_t i=0;i<(envelope?4u:8u);++i){const size_t target=extended?i:i%4;const auto mod=i<4||extended?modulation[i]:int16_t(reference::floorDivide(modulation[i],16));
            result[target]=envelope?reference::value246Adjust(result[target],mod,reference::valueVoice246EnvelopeRanges[target]):reference::valueVoice246Adjust(result[target],mod,reference::valueVoice246Ranges[size_t(index)][target]);
        }
        return result;
    }
};
}
