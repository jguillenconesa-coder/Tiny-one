// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
// Selector-3 six-voice path; seed/cadence and commercial mode identity are separate.
inline int32_t voice246UnisonDeviation(uint32_t& random,unsigned voice,bool active) noexcept {
    random=random*1664525u+1013904223u;
    const auto high=random>>16;
    const auto signedHigh=high<32768u?int32_t(high):int32_t(high)-65536;
    const auto step=int32_t(floorDivide(signedHigh,active?512:256));
    return signedPhase((uint32_t(step)*voice)<<16);
}
inline int32_t voice246ApplyUnison(int32_t base,int32_t deviation) noexcept {
    return saturate32(int64_t(base)+multiplyQ31(base,deviation));
}
}
