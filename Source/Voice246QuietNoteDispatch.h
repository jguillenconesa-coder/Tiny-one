// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Voice246InstrumentDispatch.h"
namespace pocket::reference {
// Compatibility entry points for the earlier selectors 1..3 / pitch-zero
// contract. All event and notification semantics live in the shared dispatcher.
inline Voice246NoteDispatchResult voice246QuietNoteOn(
        Voice246NoteArgument& argument,Voice246NoteArgument& local,uint8_t& flag,
        std::array<uint8_t,128>& map,uint32_t outputWord,DrWave246Pool& waves,
        Voice246Registers& registers,std::array<Adsr246State,6>& envelopes,
        Voice246ReleaseList& list,uint8_t gate,int loaded,int selected,
        int16_t selectorWord,int32_t tuning,uint32_t& random,uint32_t* ready=nullptr) noexcept {
    const auto mode=floorDivide(selectorWord,4096);
    if(mode<1||mode>3||voice246ArgumentWord(argument,12))return Voice246NoteDispatchResult::rejected;
    return voice246InstrumentNoteOn(argument,local,flag,map,outputWord,0,waves,registers,
        envelopes,list,gate,loaded,selected,selectorWord,tuning,random,ready);
}
inline Voice246ReleaseResult voice246QuietNoteOff(const Voice246NoteArgument& argument,
        Voice246NoteArgument& local,std::array<uint8_t,128>& map,uint32_t outputWord,
        Voice246ReleaseList& list,int16_t selectorWord,std::array<Adsr246State,6>& envelopes,
        int16_t sustain,int32_t tuning,std::array<int32_t,6>& pitch,uint32_t& random,uint32_t* ready=nullptr) noexcept {
    const auto mode=floorDivide(selectorWord,4096);
    if(mode<1||mode>3)return Voice246ReleaseResult::rejected;
    return voice246InstrumentNoteOff(argument,local,map,outputWord,0,0,list,selectorWord,
        envelopes,sustain,tuning,pitch,random,ready);
}
}
