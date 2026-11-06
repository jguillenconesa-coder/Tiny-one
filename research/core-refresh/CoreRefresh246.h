// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "../core-modulator-factory/CoreModulatorFactory246.h"
#include "Sampler246.h"
#include "Delay246.h"
#include <cstring>
namespace pocket::research {
// Complete refresh, bounded to drum sampler 8, synth sampler 6 and FX ID 0.
// Both routes intentionally overlay the same physical pools.
struct CoreRefresh246 {
    using Image=std::array<uint8_t,5628>;
    static uint32_t get(const uint8_t* p) noexcept {return (uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24));}
    static void put(uint8_t* p,uint32_t value) noexcept {CoreModulatorFactory246::put(p,value);}
    static bool refresh(Image& image) noexcept {
        auto* cfg=image.data()+1164;
        const auto actualSynth=get(image.data()),actualDrum=get(image.data()+4),editSynth=get(cfg+864),editDrum=get(cfg+868);
        if(actualSynth>=8||actualDrum>=8||editSynth>=8||editDrum>=8)return false;
        const auto sm=get(cfg+16+editSynth*4),dm=get(cfg+48+editDrum*4);
        if(sm>6||dm>6||get(cfg+880+editSynth*4)!=6||get(cfg+912+editDrum*4)!=8||get(cfg+960+editSynth*4)!=0||get(cfg+992+editDrum*4)!=0)return false;
        // Drum construction/binding happens first and leaves each counter intact.
        for(size_t v=0;v<12;++v){reference::Drum246State state;auto* p=image.data()+224+v*20;std::memcpy(&state,p,20);state.construct();state.bind(get(image.data()+2188+actualDrum*4));std::memcpy(p,&state,20);put(image.data()+92+v*4,0xffb00000u+uint32_t(v)*20);}
        CoreModulatorFactory246 m;std::copy_n(image.begin(),68,m.core.globals.begin());for(size_t i=0;i<21;++i)m.core.pointers[i]=get(image.data()+140+i*4);std::copy_n(image.begin()+528,588,m.pool.begin());std::copy_n(image.begin()+1116,48,m.backendFlags.begin());
        m.core.pointers[1]=0xffb00600; m.factory(true,dm);
        // Synth sampler overlays the first 96 bytes, preserving restart/padding.
        for(size_t v=0;v<6;++v){reference::Sampler246State state;auto* p=image.data()+224+v*16;std::memcpy(&state,p,16);state.construct();state.bind(get(image.data()+2220+actualSynth*4));std::memcpy(p,&state,16);put(image.data()+68+v*4,0xffb00000u+uint32_t(v)*16);}
        // FX 0 is constructed/initialized twice in the same physical object.
        reference::Delay246State fx;fx.tag=0x0190fb6c;fx.slot=0;fx.reset();
        std::memcpy(image.data()+488,&fx,20);
        m.core.pointers[0]=0xffb00600; m.factory(false,sm);
        std::copy(m.core.globals.begin(),m.core.globals.end(),image.begin());for(size_t i=0;i<21;++i)put(image.data()+140+i*4,m.core.pointers[i]);std::copy(m.pool.begin(),m.pool.end(),image.begin()+528);std::copy(m.backendFlags.begin(),m.backendFlags.end(),image.begin()+1116);
        return true;
    }
};
}
