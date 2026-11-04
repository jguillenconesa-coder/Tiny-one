// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Random246.h"
namespace pocket::reference {
// FX parameter callback only. Kind 4 is a distinct, unintegrated branch.
inline bool random246FxStep(Random246State& s,uint32_t& rng,uint32_t& meter,
                           const std::array<int16_t,8>& c,int kind,int fx,
                           std::array<int16_t,8>& values) noexcept {
    if(floorDivide(c[2],2048)!=2||(kind!=2&&kind!=5)||fx<0||fx>=8)return false;
    const auto amount=random246Advance(s,rng,c);
    for(size_t i=0;i<8;++i)values[i]=value246Adjust(values[i],multiplyQ15(s.smooth[i],amount),value246FxRanges[size_t(fx)][i]);
    random246Finish(s,meter,c,amount);return true;
}
}
