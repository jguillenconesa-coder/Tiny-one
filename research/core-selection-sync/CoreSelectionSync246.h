// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <cstdint>
namespace pocket::research {
// Outer synchronization protocol only; callback implements an explicit response.
// The budget is an authored research guard, not an original timeout.
struct CoreSelectionSync246 {
    std::array<uint8_t,132> windows{};
    static uint32_t get(const uint8_t* p) noexcept {return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
    static void put(uint8_t* p,uint32_t value) noexcept {for(unsigned i=0;i<4;++i)p[i]=uint8_t(value>>(8*i));}
    uint32_t actual(bool drum) const noexcept {return get(windows.data()+(drum?4:0));}
    uint32_t selected(bool drum) const noexcept {return get(windows.data()+68+(drum?12:8));}
    template<class Invoke> bool sync(bool drum,uint32_t& r2,Invoke&& invoke,unsigned budget=6) {
        unsigned calls=0;
        while(actual(drum)!=selected(drum)) {
            if(calls++==budget)return false;
            const auto desired=selected(drum);
            if(drum)invoke(0u,1u,desired,r2);
            else {
                r2=0;
                if(get(windows.data()+108)==1)invoke(1u,0u,0u,r2);
                else invoke(0u,1u,desired,r2);
            }
        }
        return true;
    }
};
}
