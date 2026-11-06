// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "../core-modulator-reset/CoreModulatorReset246.h"
namespace pocket::research {
// Types 0 Tremolo, 1 Value, 2 Random. Same physical storage for both routes.
struct CoreModulatorFactory246 {
    CoreModulatorReset246 core;
    std::array<uint8_t,588> pool{}; // 8-byte guard, 528-byte primary, 44-byte shared, guard.
    static void put(uint8_t* p,uint32_t value,unsigned bytes=4) noexcept {for(unsigned i=0;i<bytes;++i)p[i]=uint8_t(value>>(8*i));}
    uint8_t* object(uint32_t pointer) noexcept {return pointer>=0xffb00a00u&&pointer<=0xffb00c10u?pool.data()+8+(pointer-0xffb00a00u):nullptr;}
    static void construct(uint8_t* p,unsigned type,uint32_t tag) noexcept {
        put(p,tag);
        if(type==0){put(p+4,0x80000000);put(p+8,0,2);put(p+12,0x80000000);put(p+16,0,2);put(p+20,0,2);put(p+24,0);}
        else if(type==1){put(p+4,0x80000000);put(p+8,0,2);}
        else{put(p+4,0,2);std::fill_n(p+8,36,uint8_t(0));}
    }
    bool factory(bool drum,unsigned type) noexcept {
        if(type>2)return false;
        const unsigned base=drum?9:3,count=drum?12:6,stride=type==0?28:type==1?12:44;
        const uint32_t tag=type==0?0x019107fcu:type==1?(drum?0x0191099cu:0x0191097cu):(drum?0x0191092cu:0x0191090cu);
        const uint32_t shared=type?0xffb00c10u:0;
        if(type)construct(object(shared),type,type==1?0x019109bcu:0x0191094cu);
        core.put(drum?60:56,shared);
        for(unsigned v=0;v<count;++v){const auto pointer=0xffb00a00u+v*stride;construct(object(pointer),type,tag);core.pointers[base+v]=pointer;}
        return true;
    }
};
}
