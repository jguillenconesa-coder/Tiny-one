// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
namespace pocket::reference {
// Bounded lifecycle only. This does not implement the Spring audio renderer.
struct Spring246State {
    uint32_t tag=0;
    std::array<int32_t,13> filters{};
    std::array<int16_t,4> feedback{};
    std::array<int32_t,9> cursors{};
    int32_t slot=0;
};
static_assert(sizeof(Spring246State)==104);
using Spring246Memory=std::array<int16_t,29136>;
inline bool spring246Reset(Spring246State& s,Spring246Memory& memory,int32_t slot,int32_t& pending) noexcept {
    if(slot<0||slot>1)return false;
    s.feedback.fill(0);s.cursors.fill(0);s.slot=slot;
    std::fill(memory.begin(),memory.begin()+6984,0);pending=2;
    return true;
}
// Each pending callback clears the SAME final three lines, decrements the
// counter and returns without modifying audio, state, display or middle lines.
inline bool spring246DeferredClear(Spring246Memory& memory,int32_t& pending) noexcept {
    if(pending<1||pending>2)return false;
    std::fill(memory.begin()+18152,memory.end(),0);--pending;return true;
}
}
