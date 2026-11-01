// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Envelope246Transition.h"

namespace pocket::reference {
// Independent model of 0x0191264c for VALID indices 0..5. A zero ready WORD
// initializes all six 24-byte states before returning the selected state;
// every nonzero word preserves the pool and ready value. No allocator/clock.
// Invalid-index rejection is adapter policy: the original asserts and may
// initialize the pool before doing so. That error path is deliberately excluded.
inline Adsr246State* adsr246Get(std::array<Adsr246State,6>& pool,
                              uint32_t& ready,int index) noexcept {
    if(index<0||index>=6)return nullptr;
    if(ready==0){
        ready=1;
        for(auto& state:pool)adsr246Initialize(state);
    }
    return &pool[size_t(index)];
}
// Existing ready-pool callers pass no word. Cold-pool compositions use the
// same owning accessor at the point where the original calls 0x0191264c.
inline Adsr246State& adsr246Access(std::array<Adsr246State,6>& pool,
                                size_t index,uint32_t* ready=nullptr) noexcept {
    return ready?*adsr246Get(pool,*ready,int(index)):pool[index];
}
}
