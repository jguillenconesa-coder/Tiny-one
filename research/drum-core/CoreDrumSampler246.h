// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Drum246.h"
namespace pocket::research {
// Bounded reconstruction of Core's twelve audio producers. Index guards belong
// to this research model; the original asserts on invalid voice/object inputs.
struct CoreDrumSampler246 {
    std::array<reference::Drum246State,12> pool{};
    std::array<uint32_t,12> pointers{};

    void addressPool() noexcept {
        for(size_t i=0;i<12;++i)pointers[i]=0xffb00000u+uint32_t(i)*20;
    }
    bool factory(unsigned sound) noexcept {
        if(sound>=8)return false;
        addressPool();
        for(auto& state:pool){state.construct();state.bind(0x02010000u+sound*8);}
        return true;
    }
    static uint32_t controlAddress(unsigned sound,unsigned pad) noexcept {
        return 0x01aaadb4u+sound*384u+pad*16u;
    }
    static uint32_t receiver(unsigned voice,int16_t adjustment) noexcept {
        return 0xffb00000u+voice*20u+uint32_t(int32_t(adjustment));
    }
    bool dispatch(unsigned voice,unsigned sound,unsigned pad,bool release,
                  const std::array<int16_t,8>& controls) noexcept {
        if(voice>=12||sound>=8||pad>=24)return false;
        if(release)pool[voice].release(controls);else pool[voice].reset(controls);
        return true;
    }
};
}
