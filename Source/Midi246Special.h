// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
// Independently reconstructed special MIDI arithmetic; caller-owned buffers and
// display values are explicit. These functions do not implement their producers.
inline int16_t midi246VoiceGainCoefficient(int16_t previous,uint32_t value) noexcept {
 const auto raw=saturate16(int(int16_t(value))*256);
 return saturate16(int(multiplyQ15(raw,4096))+multiplyQ15(previous,28672));
}
inline int16_t midi246VoiceGain(int16_t input,int16_t coefficient) noexcept {
 return saturate16(int(multiplyQ15(input,coefficient))*2);
}
inline uint32_t midi246VoicePitch(uint32_t input,uint32_t value) noexcept {
 const auto raw=saturate16(int(int16_t(value))*256);
 const auto cubic=multiplyQ15(multiplyQ15(raw,raw),raw);
 return uint32_t(saturate32(int64_t(multiplyQ31(signedPhase(input),int32_t(cubic)*65536))*8));
}
inline int16_t midi246FxAudioFactor(uint32_t value,bool steep) noexcept {
 const auto delta=int16_t(uint16_t(value)-64u);
 return steep?saturate16(int(delta)*1024):int16_t(uint16_t(delta)*512u);
}
}
