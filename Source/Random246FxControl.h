// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Random246Fx.h"
namespace pocket {
// Authored shared-bus lifecycle, capture clock and sample-and-hold adapter.
struct Random246FxControl {
    reference::Random246State state;
    double position=1;uint64_t ticks=0;uint32_t meter=0;
    std::array<int16_t,8> modulation{};bool active=false,pendingNote=false;
    void reset() noexcept {*this={};}
    void noteOn() noexcept {pendingNote=true;}
    std::array<int16_t,4> next(bool enabled,const std::array<int16_t,4>& raw,int fx,double step,
                              uint32_t& rng,const std::array<int16_t,4>& base) noexcept {
        if(!enabled){if(active)reset();return base;}
        active=true;
        const std::array<int16_t,8> c{{raw[0],int16_t(std::max(0,int(raw[1]))),4096,int16_t(std::max(0,int(raw[3]))),0,0,0,0}};
        if(pendingNote){state.noteOn(c[3]);pendingNote=false;}
        if(position>=1){
            const auto amount=reference::random246Advance(state,rng,c);
            for(size_t i=0;i<8;++i)modulation[i]=reference::multiplyQ15(state.smooth[i],amount);
            reference::random246Finish(state,meter,c,amount);
            position-=1;++ticks;
        }
        position+=step;
        auto result=base;
        if(fx>=0&&fx<8)for(size_t i=0;i<4;++i)result[i]=reference::value246Adjust(base[i],modulation[i],reference::value246FxRanges[size_t(fx)][i]);
        return result;
    }
};
}
