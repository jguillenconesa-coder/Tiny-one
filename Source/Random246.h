// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ValueVoice246.h"
#include "Tremolo246Tables.h"
namespace pocket::reference {
struct Random246State {
    uint32_t tag=0;int16_t gate=0;uint16_t padding=0;uint32_t counter=0;
    std::array<int16_t,8> targets{},smooth{};
    void noteOn(int16_t envelope) noexcept {gate=envelope<14745?0:32767;}
};
static_assert(sizeof(Random246State)==44);
inline int random246Period(int16_t speed) noexcept {
    const auto inverse=saturate16(32767-int(speed));
    return int(floorDivide(multiplyQ15(inverse,inverse),64))+1;
}
// Original synth callback, bounded here to the verified synth families.
// RNG and the shared 32-bit display word are caller-owned; no hidden entropy.
// Instrument signals 4..7 revisit controls 0..3 at 1/16 modulation; the
// auxiliary destination words are preserved for these verified range families.
inline bool random246VoiceStep(Random246State& s,uint32_t& rng,uint32_t& meter,
                                const std::array<int16_t,8>& c,int kind,int type,
                                std::array<int16_t,8>& values) noexcept {
    const auto destination=int(floorDivide(c[2],2048));
    if(!((destination==0&&kind==0)||(destination==1&&kind==1)))return false;
    const int index=valueVoice246RangeIndex(type);if(index<0)return false;
    ++s.counter;
    if(signedPhase(s.counter)>=random246Period(c[1])){
        for(auto& value:s.targets){rng=rng*1664525u+1013904223u;value=int16_t(floorDivide(signedPhase(rng),65536));}
        s.counter=0;
    }
    for(size_t i=0;i<8;++i)s.smooth[i]=saturate16(int(multiplyQ15(s.targets[i],4096))+multiplyQ15(s.smooth[i],28672));
    const auto amount=multiplyQ15(s.gate,c[0]);
    const size_t count=kind==1?4:8;
    for(size_t i=0;i<count;++i){auto modulation=multiplyQ15(s.smooth[i],amount);
        const size_t target=kind==0?i%4:i;
        if(kind==0&&i>=4)modulation=int16_t(floorDivide(modulation,16));
        values[target]=kind==1?value246Adjust(values[target],modulation,valueVoice246EnvelopeRanges[target]):valueVoice246Adjust(values[target],modulation,valueVoice246Ranges[size_t(index)][target]);
    }
    if(c[3]<16384){const auto x=int16_t(floorDivide(c[3],4));s.gate=saturate16(int(s.gate)+saturate16(int(multiplyQ15(x,x))+1));}
    else s.gate=multiplyQ15(s.gate,int16_t(floorDivide(tremolo246Decay[size_t(31-c[3]/1024)],65536)));
    meter=(meter&0xffff0000u)|uint16_t(multiplyQ15(s.smooth[0],amount));return true;
}
}
