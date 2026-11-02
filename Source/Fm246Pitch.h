// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Fm246.h"
namespace pocket {
// Authored fractional extension, NOT recovered original pitch arithmetic.
// Integer-word pitches retain the exact callback. Fractional pitches interpolate
// the signed increments of adjacent original coefficient evaluations and retain
// the full carrier increment. Routing, feedback and six contours are unchanged.
inline bool fm246HybridBlock(reference::Fm246State& state,reference::Fm246Workspace& work,
                             const std::array<int16_t,4>& controls,uint32_t pitch,
                             const std::array<int16_t,30>& table,const std::array<int16_t,8192>& sine,
                             std::array<int16_t,128>& output) noexcept {
    if((pitch&65535u)==0||pitch>=0x7fff0000u)
        return reference::fm246Block(state,work,controls,pitch,table,sine,output);
    reference::fm246AdvanceEnvelopes(state,reference::multiplyQ15(controls[3],controls[3]),table,sine);
    auto low=reference::Fm246Coefficients(state,controls,pitch,table);
    const auto high=reference::Fm246Coefficients(state,controls,(pitch&0xffff0000u)+65536u,table);
    const auto fraction=int64_t(pitch&65535u);
    low.increments[0]=pitch;
    for(size_t i=1;i<4;++i){
        const int64_t a=reference::signedPhase(low.increments[i]),b=reference::signedPhase(high.increments[i]);
        low.increments[i]=uint32_t(a+reference::floorDivide((b-a)*fraction+32768,65536));
    }
    return reference::fm246Render(state,work,low,sine,output);
}
}
