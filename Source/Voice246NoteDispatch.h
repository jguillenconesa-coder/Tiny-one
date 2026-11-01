// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Voice246InstrumentDispatch.h"
namespace pocket::reference {
// Compatibility contract: selectors 1..3, metadata/pitch zero. Shared complete
// event semantics live in the instrument dispatcher; no notification is reached.
inline Voice246NoteDispatchResult voice246NoteDispatch(
        Voice246NoteArgument& argument,uint32_t discriminator,uint8_t& noteFlag,
        Voice246NoteArgument& localArgument,DrWave246Pool& waves,
        Voice246Registers& registers,std::array<Adsr246State,6>& envelopes,
        Voice246ReleaseList& list,uint8_t gate,int loaded,int selected,
        int16_t selectorWord,int32_t tuning,uint32_t& random) noexcept {
    const auto mode=floorDivide(selectorWord,4096);
    if(argument[8]||voice246ArgumentWord(argument,12)||mode<1||mode>3)return Voice246NoteDispatchResult::rejected;
    std::array<uint8_t,128> unusedMap{};
    return voice246InstrumentNoteOn(argument,localArgument,noteFlag,unusedMap,0,discriminator,
        waves,registers,envelopes,list,gate,loaded,selected,selectorWord,tuning,random);
}
}
