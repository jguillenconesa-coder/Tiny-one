// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "../core-modulator-reset/CoreModulatorReset246.h"
namespace pocket::research {
// Types 0 Tremolo, 1 Value, 2 Random, 3 Element, 4 MIDI, 5 Bend, 6 Crank. Same physical storage for both routes.
struct CoreModulatorFactory246 {
    CoreModulatorReset246 core;
    std::array<uint8_t,588> pool{}; // 8-byte guard, 528-byte primary, 44-byte shared, guard.
    std::array<uint8_t,48> backendFlags{}; // observed window 0xff907aa8..0xff907ad8
    static void put(uint8_t* p,uint32_t value,unsigned bytes=4) noexcept {for(unsigned i=0;i<bytes;++i)p[i]=uint8_t(value>>(8*i));}
    uint8_t* object(uint32_t pointer) noexcept {return pointer>=0xffb00a00u&&pointer<=0xffb00c10u?pool.data()+8+(pointer-0xffb00a00u):nullptr;}
    static void construct(uint8_t* p,unsigned type,uint32_t tag) noexcept {
        put(p,tag);
        if(type==0){put(p+4,0x80000000);put(p+8,0,2);put(p+12,0x80000000);put(p+16,0,2);put(p+20,0,2);put(p+24,0);}
        else if(type==1){put(p+4,0x80000000);put(p+8,0,2);}
        else if(type==2){put(p+4,0,2);std::fill_n(p+8,36,uint8_t(0));}
    }
    bool factory(bool drum,unsigned type) noexcept {
        if(type>6)return false;
        const unsigned base=drum?9:3,count=drum?12:6,stride=type==0?28:type==1?12:type==2?44:4;
        constexpr std::array<uint32_t,7> synthTags{{0x019107fc,0x0191097c,0x0191090c,0x0191089c,0x0191082c,0x0191071c,0x0191078c}};
        constexpr std::array<uint32_t,7> sharedTags{{0,0x019109bc,0x0191094c,0x019108dc,0x0191086c,0x0191075c,0x019107cc}};
        const uint32_t tag=synthTags[type]+((drum&&type)?32u:0u);
        const uint32_t shared=type?0xffb00c10u:0;
        if(type==5)backendFlags[8]=0;
        if(type==6)backendFlags[37]=0;
        if(type)construct(object(shared),type,sharedTags[type]);
        core.put(drum?60:56,shared);
        for(unsigned v=0;v<count;++v){const auto pointer=0xffb00a00u+v*stride;construct(object(pointer),type,tag);core.pointers[base+v]=pointer;}
        return true;
    }
};
}
