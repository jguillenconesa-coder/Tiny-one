// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
namespace pocket::research {
// Bounded Core dispatch/clear contract. Pointer values and invoked reset bodies
// are supplied; guards for invalid host indices are authored extensions.
struct CoreModulatorReset246 {
    std::array<uint8_t,68> globals{};
    std::array<uint32_t,21> pointers{};
    uint32_t get(unsigned offset) const noexcept {const auto* p=globals.data()+offset;return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
    void put(unsigned offset,uint32_t value) noexcept {for(unsigned i=0;i<4;++i)globals[offset+i]=uint8_t(value>>(i*8));}
    void clear(bool drum) noexcept {const unsigned base=drum?9:3,count=drum?12:6;std::fill_n(pointers.begin()+base,count,0);put(drum?60:56,0);}
    template<class Reset> bool reset(bool drum,unsigned voice,Reset&& invoke){
        const unsigned base=drum?9:3,count=drum?12:6,selector=drum?4:0,flags=drum?44:28;
        if(voice>=count)return false;
        const auto primary=pointers[base+voice];if(!primary)return false;
        const auto sound=get(selector);if(sound>=8||!globals[flags+sound])return false;
        const uint32_t controls=drum?0xff801644u:0xff8014c4u;
        invoke(primary,controls+sound*16);
        const auto secondary=get(drum?60:56);
        if(secondary){const auto current=get(selector);if(current>=8)return false;invoke(secondary,controls+current*16);}
        return true;
    }
};
}
