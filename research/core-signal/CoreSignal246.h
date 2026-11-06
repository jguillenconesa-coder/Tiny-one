// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::research {
// Original 64-word Core callback; caller/routing and semantic buffer role are
// unproven. The first feedback word is supplied separately from full history.
struct CoreSignal246 {
    int32_t previousInput=0,previousQuantized=0,previousOutput=0;
    void block(int32_t coefficient,const std::array<int16_t,64>& input,std::array<int16_t,64>& output) noexcept {
        using namespace reference;
        for(size_t i=0;i<input.size();++i){
            const auto x=int32_t(input[i])*65536;
            const auto difference=saturate32(int64_t(previousOutput)-previousInput);
            const auto sum=saturate32(int64_t(difference)+x);
            const auto y=saturate32(int64_t(sum)-multiplyQ31(coefficient,previousQuantized));
            const auto high=int16_t(floorDivide(y,65536));output[i]=high;
            previousInput=x;previousQuantized=int32_t(high)*65536;previousOutput=y;
        }
    }
};
}
