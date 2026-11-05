// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "../drum-core/CoreDrumSampler246.h"
namespace pocket::research {
// Research domain: type-1 drum mode, no modulation objects/FX and supplied
// octave transposition only. Does not provide a scheduler or gain producer.
struct DrumVoice246 {
    CoreDrumSampler246 audio;
    std::array<unsigned char,812> globals{};
    std::array<unsigned char,2564> held{};
    using Controls=std::array<std::array<std::array<int16_t,8>,24>,8>;
    static uint32_t get(const unsigned char* p) noexcept {return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
    static void put(unsigned char* p,uint32_t v,unsigned bytes=4) noexcept {for(unsigned i=0;i<bytes;++i)p[i]=static_cast<unsigned char>(v>>(i*8));}
    unsigned char* at(unsigned address) noexcept {return globals.data()+address-0x1038;}
    uint32_t value(unsigned address) noexcept {return get(at(address));}
    void reset(unsigned sound) noexcept {globals.fill(0);held.fill(0x5a);put(held.data()+2560,0);put(at(0x1038),1);audio.factory(sound);}
    uint32_t query(unsigned voice) noexcept {return audio.pool[voice].active();}
    // Returns selected voice for research diagnostics; original callback is void.
    int on(const std::array<uint32_t,4>& event,int transpose,int preview,unsigned sound,const Controls& controls) noexcept {
        const auto note=reference::signedPhase(event[0]);if(note<53||note>76)return -1;
        const auto count=get(held.data()+2560);
        for(unsigned i=0;i<count;++i)if(get(held.data()+i*20)==event[0])return -1;
        if(count>=128||sound>=8||transpose<-24||transpose>24||transpose%12)return -1;
        int selected=-1,best=-1;
        for(unsigned v=0;v<12;++v){if(!*at(0x1070+v)){selected=int(v);break;}const auto age=reference::signedPhase(query(v));if(age>best){best=age;selected=int(v);}}
        if(selected<0)return -1;
        const auto voice=unsigned(selected),pad=unsigned(note-53);
        *at(0x1070+voice)=0;
        if(preview==-1){put(at(0x1044),pad);put(at(0x1048),voice);}else if(preview==int(pad))put(at(0x1048),voice);
        put(at(0x10f4+voice*4),pad);*at(0x1124+pad)=1;
        const auto low=event[1]&65535;const auto velocity=low<32768?int(low):int(low)-65536;
        put(at(0x12ec+voice*2),uint16_t(reference::saturate16(velocity*256)),2);
        put(at(0x1304+voice*4),uint32_t(1)<<(26+transpose/12));
        put(at(0x1334+voice*4),pad);
        audio.dispatch(voice,sound,pad,false,controls[sound][pad]);
        auto* row=held.data()+count*20;
        put(row,event[0]);put(row+4,event[1]);row[8]=static_cast<unsigned char>(event[2]);put(row+12,event[3]);put(row+16,voice);
        put(held.data()+2560,count+1);*at(0x1070+voice)=1;return selected;
    }
    bool off(uint32_t note,unsigned sound,const Controls& controls) noexcept {
        const auto signedNote=reference::signedPhase(note);if(signedNote<53||signedNote>76||sound>=8)return false;
        const auto count=get(held.data()+2560);
        for(unsigned i=0;i<count;++i){auto* row=held.data()+i*20;if(get(row)!=note)continue;
            const auto voice=get(row+16),pad=note-53;
            if(voice>=12)return false;
            if(value(0x10f4+voice*4)==pad&&*at(0x1070+voice))audio.dispatch(voice,sound,value(0x1334+voice*4),true,controls[sound][value(0x1334+voice*4)]);
            for(unsigned j=i;j+1<count;++j){auto* dst=held.data()+j*20;const auto* src=dst+20;put(dst,get(src));put(dst+4,get(src+4));dst[8]=src[8];put(dst+12,get(src+12));put(dst+16,get(src+16));}
            put(held.data()+2560,count-1);return true;
        }
        return false;
    }
};
}
