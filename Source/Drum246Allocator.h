// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
// Original type-1 drum selection (VoiceCtl 0x01a84ba0): first zero byte,
// otherwise greatest signed activity count, first on ties. Query order matters:
// inactive original sampler objects clear their block counter on consultation.
// -1 represents the original no-candidate assertion, not a recovered fallback.
template<class Query>
inline int drum246SelectVoice(const std::array<uint8_t,12>& occupied,Query&& query) noexcept {
    int selected=-1;int32_t greatest=-1;
    for(unsigned voice=0;voice<12;++voice){
        if(occupied[voice]==0)return int(voice);
        const auto count=signedPhase(query(voice));
        if(count>greatest){greatest=count;selected=int(voice);}
    }
    return selected;
}
}
