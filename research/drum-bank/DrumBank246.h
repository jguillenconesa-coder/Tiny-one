// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "../drum-voice/DrumVoice246.h"
namespace pocket::research {
// Supplied order, null LFO/FX and MIDI output disabled. Registry operations
// are not a recovered per-block retirement policy or an audible panic.
struct DrumBank246 : DrumVoice246 {
    uint32_t phaseWord=0xa5a5a500u;
    std::array<unsigned char,128> externalNotes{};
    void reset(unsigned sound) noexcept {
        DrumVoice246::reset(sound);phaseWord=0xa5a5a500u;externalNotes.fill(127);
    }
    void clearBusy() noexcept {for(unsigned v=0;v<12;++v)*at(0x1070+v)=0;}
    bool beginBank() noexcept {
        if(phaseWord&255u)return false;
        phaseWord|=1u;clearBusy();return true;
    }
    bool finishBank(unsigned sound) noexcept {
        if(!(phaseWord&255u)||sound>=8)return false;
        audio.factory(sound);phaseWord&=0xffffff00u;return true;
    }
    bool retriggerHeld(unsigned sound,const Controls& controls) noexcept {
        if(sound>=8)return false;
        for(unsigned i=0;i<get(held.data()+2560);++i){
            const auto voice=get(held.data()+i*20+16);
            if(voice>=12)return false;
            const auto pad=value(0x1334+voice*4);if(pad>=24)return false;
            *at(0x1070+voice)=0;
            audio.dispatch(voice,sound,pad,false,controls[sound][pad]);
            *at(0x1070+voice)=1;
        }
        return true;
    }
    void clearRegistry() noexcept {
        clearBusy();
        for(unsigned i=0;i<get(held.data()+2560);++i){const auto* row=held.data()+i*20;const auto note=get(row);if(row[8]&&note<128)externalNotes[note]=0;}
        put(held.data()+2560,0);
    }
    uint32_t busy() noexcept {for(unsigned v=0;v<12;++v)if(*at(0x1070+v))return 1;return 0;}
};
}
