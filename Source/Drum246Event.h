// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
#include "Drum246EventTables.h"
#include <cmath>
namespace pocket::reference {
inline int16_t drum246EventVelocityWord(uint32_t raw) noexcept {
    const auto low=raw&65535u;
    const int value=low<32768u?int(low):int(low)-65536;
    return saturate16(value*256);
}
// Complete discrete input domain captured from original note-on. The original
// computes int->float, /12, pow(2,x), /32, float->Q31; this is a derived lookup,
// not a recovered ROM table or a general soft-float/pow implementation.
inline bool drum246EventPitchWord(int semitones,uint32_t& word) noexcept {
    if(semitones<-128||semitones>127)return false;
    word=drum246EventPitchWords[size_t(semitones+128)];return true;
}
inline constexpr int16_t drum246MaximumMidiVelocityWord=127*256;
}
namespace pocket {
// Authored host bridges. Original event words do not recover the per-sample
// producers: fine pitch, bend, the factor-two normalization and amplitude scale
// remain explicit host policies.
inline int16_t drum246HostVelocityWord(float velocity) noexcept {
    if(!std::isfinite(velocity)||velocity<=0)return 0;
    const auto midi=uint32_t(std::lround(std::min(velocity,1.f)*127.f));
    return reference::drum246EventVelocityWord(midi);
}
inline uint32_t drum246HostPitchIncrement(double semitones,double bend) noexcept {
    if(!std::isfinite(semitones)||!std::isfinite(bend)||bend<=0)return 0;
    semitones=std::clamp(semitones,-24.,24.);
    const int coarse=int(std::floor(semitones));uint32_t word=0;
    if(!reference::drum246EventPitchWord(coarse,word))return 0;
    double pitch=word;const double fraction=semitones-coarse;
    if(fraction>0){uint32_t upper=0;if(!reference::drum246EventPitchWord(coarse+1,upper))return 0;
        pitch*=std::exp2(std::log2(double(upper)/word)*fraction);
    }
    const double increment=2.*pitch*bend;
    return uint32_t(std::llround(std::clamp(increment,0.,2147483647.)));
}
}
