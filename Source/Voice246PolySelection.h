// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
// Recovered busy poly scan: signed maximum starts at -1; first tie wins.
// -1 identifies the excluded original assert domain, not an original return.
inline int voice246SelectBusy(const std::array<uint32_t,6>& blocks) noexcept {
    int voice=-1;int32_t greatest=-1;
    for(size_t i=0;i<blocks.size();++i){const auto age=signedPhase(blocks[i]);
        if(age>greatest){greatest=age;voice=int(i);}}
    return voice;
}
}
