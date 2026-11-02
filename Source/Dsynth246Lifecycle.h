// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
// Independent lifecycle reconstruction. This is not a D-synth audio renderer.
struct Dsynth246State {
    uint32_t tag=0;std::array<int32_t,4> envelope{};int32_t slot=0;
    int16_t selection=0;uint16_t padding=0;
    struct Oscillator {uint32_t phase=0x80000000u;int16_t sample=0;uint16_t padding=0;};
    std::array<Oscillator,2> oscillators{};std::array<int32_t,7> filters{};
    void construct() noexcept {
        tag=0x0190fbbcu;envelope.fill(0);selection=0;
        for(auto& o:oscillators){o.phase=0x80000000u;o.sample=0;}filters.fill(0);
    }
    bool bind(int position) noexcept {if(position<0||position>=6)return false;slot=position;return true;}
    void release() noexcept {envelope[2]=envelope[3]=0;}
};
static_assert(sizeof(Dsynth246State)==72);
struct Dsynth246Context {
    std::array<int32_t,2> constants{};
    std::array<std::array<int16_t,2>,6> peaks{};
    std::array<std::array<int16_t,64>,6> history{};
    std::array<uint32_t,6> counts{};
    void initialize() noexcept {constants[0]=0x00a3d700;constants[1]=signedPhase((uint32_t(constants[1])&0xffff0000u)|0x4000u);peaks={};history={};}
    bool decayMeter(int slot) noexcept {if(slot<0||slot>=6)return false;for(auto& x:peaks[size_t(slot)])x=multiplyQ15(x,28672);return true;}
    bool resetCapture(int slot) noexcept {if(slot<0||slot>=6)return false;counts[size_t(slot)]=0;return true;}
    bool reset(Dsynth246State& s,const std::array<int16_t,8>& controls) noexcept {
        if(s.slot<0||s.slot>=6)return false;
        s.envelope={0,0,INT32_MAX,INT32_MAX};s.selection=controls[3];
        for(auto& o:s.oscillators){o.phase=0x80000000u;o.sample=0;}s.filters.fill(0);
        history[size_t(s.slot)].fill(0);counts[size_t(s.slot)]=0;return true;
    }
    bool capture(int slot,const std::array<int16_t,128>& audio) noexcept {
        if(slot<0||slot>=6)return false;auto& n=counts[size_t(slot)];
        if(signedPhase(n)<60){if(n>59)return false;for(size_t i=0;i<4;++i)history[size_t(slot)][n++]=audio[i*32];}
        return true;
    }
};
static_assert(sizeof(Dsynth246Context)==824);
}
