// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "../core-modulator-factory/CoreModulatorFactory246.h"
#include "../core-selection-sync/CoreSelectionSync246.h"
#include "Drum246.h"
#include "Value246.h"
#include "Random246.h"
#include "Tremolo246.h"
#include <cstring>
namespace pocket::research {
// Original drum-mode outer changer; reconstruction service is explicitly authored.
struct DrumBankSwitch246 {
    using Image=std::array<uint8_t,8796>;
    using Trace=std::array<uint8_t,128>;
    struct Stats {size_t service=0,held=0,primary=0,secondary=0;std::array<size_t,7> resets{};};
    static uint32_t get(const uint8_t* p) noexcept {return CoreSelectionSync246::get(p);}
    static void put(uint8_t* p,uint32_t value) noexcept {CoreSelectionSync246::put(p,value);}
    static CoreModulatorFactory246 load(const Image& image) noexcept {
        CoreModulatorFactory246 m;std::copy_n(image.begin(),68,m.core.globals.begin());for(size_t i=0;i<21;++i)m.core.pointers[i]=get(image.data()+140+i*4);std::copy_n(image.begin()+528,588,m.pool.begin());std::copy_n(image.begin()+1116,48,m.backendFlags.begin());return m;
    }
    static void save(Image& image,const CoreModulatorFactory246& m) noexcept {
        std::copy(m.core.globals.begin(),m.core.globals.end(),image.begin());for(size_t i=0;i<21;++i)put(image.data()+140+i*4,m.core.pointers[i]);std::copy(m.pool.begin(),m.pool.end(),image.begin()+528);std::copy(m.backendFlags.begin(),m.backendFlags.end(),image.begin()+1116);
    }
    // Caller/response action, not a recovered implementation of the L1 entry.
    static bool constructDrum(Image& image,unsigned bank,unsigned family) noexcept {
        if(bank>=8||family>6)return false;put(image.data()+4,bank);
        for(size_t v=0;v<12;++v){reference::Drum246State s;auto* p=image.data()+224+v*20;std::memcpy(&s,p,20);s.construct();s.bind(get(image.data()+2188+bank*4));std::memcpy(p,&s,20);put(image.data()+92+v*4,0xffb00000u+uint32_t(v)*20);}
        auto m=load(image);m.factory(true,family);save(image,m);return true;
    }
    static int16_t half(const uint8_t* p) noexcept {const auto v=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(v<32768?int(v):int(v)-65536);}
    template<class S,class F>static void resetObject(uint8_t* p,F&& f){S s;std::memcpy(&s,p,sizeof(s));f(s);std::memcpy(p,&s,sizeof(s));}
    static bool change(Image& image,unsigned target,unsigned delay,Trace& trace,unsigned& calls,Stats& stats) {
        using namespace reference;trace.fill(0x5a);calls=0;const auto mode=get(image.data()+8716),selected=get(image.data()+2032);
        if(target>=8||selected>=8||(mode!=1&&mode!=2))return false;
        if(mode==2||target==selected)return true;
        const auto count=get(image.data()+4812);if(count>24||delay<1||delay>4)return false;
        for(size_t i=0;i<count;++i){const auto v=get(image.data()+2252+i*20+16);if(v>=12||get(image.data()+5580+v*4)>=24)return false;}
        image[8704]=1;put(image.data()+2032,target);
        CoreSelectionSync246 sync;auto snapshot=[&](){std::copy_n(image.begin(),68,sync.windows.begin());std::copy_n(image.begin()+2020,32,sync.windows.begin()+68);std::copy_n(image.begin()+4816,32,sync.windows.begin()+100);};snapshot();uint32_t r2=0;bool valid=true;
        const bool ready=sync.sync(true,r2,[&](uint32_t endpoint,uint32_t arg0,uint32_t,uint32_t){if(endpoint||arg0!=1||calls>=4){valid=false;return;}const auto offset=calls*32;const bool finish=++calls==delay;++stats.service;const std::array<uint32_t,6> fields{{arg0,get(image.data()+2032),get(image.data()+4),image[8704],get(image.data()+4812),finish?1u:0u}};for(size_t i=0;i<fields.size();++i)put(trace.data()+offset+i*4,fields[i]);if(finish){const auto bank=get(image.data()+2032),family=get(image.data()+1164+48+bank*4);valid=constructDrum(image,bank,family);}snapshot();},4);
        if(!ready||!valid||calls!=delay)return false;
        const auto bank=get(image.data()+4);
        constexpr std::array<uint32_t,7> tags{{0x019107fc,0x0191099c,0x0191092c,0x019108bc,0x0191084c,0x0191073c,0x019107ac}};
        constexpr std::array<uint32_t,7> shared{{0,0x019109bc,0x0191094c,0x019108dc,0x0191086c,0x0191075c,0x019107cc}};
        for(size_t i=0;i<count;++i){const auto voice=get(image.data()+2252+i*20+16);image[4872+voice]=0;auto m=load(image);bool known=true;
            m.core.reset(true,voice,[&](uint32_t pointer,uint32_t parameters){auto* p=m.object(pointer);if(!p||parameters!=0xff801644u+bank*16){known=false;return;}size_t family=0;for(;family<7;++family)if(get(p)==tags[family]||(shared[family]&&get(p)==shared[family]))break;if(family==7){known=false;return;}++stats.resets[family];if(pointer==0xffb00c10u)++stats.secondary;else ++stats.primary;std::array<int16_t,8> c{};for(size_t j=0;j<8;++j)c[j]=half(image.data()+1884+bank*16+j*2);
                if(family==0){std::array<uint32_t,8> display{};for(size_t j=0;j<8;++j)display[j]=get(image.data()+8724+j*4);resetObject<Tremolo246State>(p,[&](auto& s){tremolo246NoteOn(s,display,c);});for(size_t j=0;j<8;++j)put(image.data()+8724+j*4,display[j]);}
                else if(family==1)resetObject<Value246State>(p,[&](auto& s){s.noteOn(c);});
                else if(family==2)resetObject<Random246State>(p,[&](auto& s){s.noteOn(c[3]);});
            });if(!known)return false;save(image,m);
            const auto pad=get(image.data()+5580+voice*4);std::array<int16_t,8> c{};for(size_t j=0;j<8;++j)c[j]=half(image.data()+5628+bank*384+pad*16+j*2);Drum246State s;auto* p=image.data()+224+voice*20;std::memcpy(&s,p,20);s.reset(c);std::memcpy(p,&s,20);image[4872+voice]=1;++stats.held;
        }
        image[8704]=0;return true;
    }
};
}
