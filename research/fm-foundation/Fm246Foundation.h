// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
#include "Fractional246.h"
namespace pocket::reference {
struct Fm246Operator {uint32_t phase=0x80000000u;int16_t sample=0;uint16_t padding=0;};
struct Fm246State {
    uint32_t tag=0;std::array<Fm246Operator,4> operators{};
    std::array<int16_t,6> gains{},angles{};
    void construct() noexcept {tag=0x0190ff84;for(auto& o:operators){o.phase=0x80000000u;o.sample=0;}gains.fill(0);angles.fill(0);}
    void reset() noexcept {for(auto& o:operators){o.phase=0x80000000u;o.sample=0;}gains.fill(0);angles.fill(-32768);}
};
static_assert(sizeof(Fm246State)==60);
inline std::array<int16_t,30> fm246ConvertTables(const std::array<uint32_t,30>& words) noexcept {
    std::array<int16_t,30> result{};for(size_t i=0;i<30;++i)result[i]=fractional246Q15FromBits(words[i]);return result;
}
// Shared table copy preserves all 30 words, including the three ratio banks.
inline void fm246CopyTables(std::array<int16_t,30>& target,const std::array<int16_t,30>& source) noexcept {target=source;}
// This is the complete internal six-envelope callback, not an FM audio renderer.
inline void fm246AdvanceEnvelopes(Fm246State& state,int16_t amount,const std::array<int16_t,30>& table,
                                  const std::array<int16_t,8192>& sine) noexcept {
    for(size_t i=0;i<6;++i){
        const auto index=size_t(4096+floorDivide(state.angles[i],8));
        state.gains[i]=multiplyQ15(amount,sine[index]);
        const auto bits=uint16_t(uint16_t(state.angles[i])+uint16_t(multiplyQ15(table[i],amount)));
        state.angles[i]=int16_t(bits<32768?int(bits):int(bits)-65536);
    }
}
}
