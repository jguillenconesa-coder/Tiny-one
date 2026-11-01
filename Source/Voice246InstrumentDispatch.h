// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "DrWave246NoteModes.h"
#include "DrWave246HeldNoteModes.h"
#include "Voice246ReleaseSelectors.h"
#include "DrWave246EmptyExplicitPitch.h"
#include "DrWave246PolyFirstFree.h"
#include "DrWave246PolySteal.h"
#include "DrWave246PolyExplicitPitch.h"
namespace pocket::reference {
enum class Voice246NoteDispatchResult { rejected, gated, duplicate, appended, disabled };
// Bounded original notification behavior with 0x01dfe03c explicitly ZERO.
// Guards are adapter policy. No packet preparation or transport emulation.
inline bool voice246QuietNotifyOn(uint32_t note,uint32_t velocity,
        std::array<uint8_t,128>& map,uint32_t outputWord) noexcept {
    if(note>=128||outputWord!=0)return false;
    if(velocity==0)map[note]=0;
    else if(map[note]==0)map[note]=1;
    return true;
}
inline bool voice246QuietNotifyOff(uint32_t note,
        std::array<uint8_t,128>& map,uint32_t outputWord) noexcept {
    if(note>=128||outputWord!=0)return false;
    map[note]=0;return true;
}
inline uint32_t voice246ArgumentWord(const Voice246NoteArgument& a,size_t p) noexcept {
    return uint32_t(a[p])|(uint32_t(a[p+1])<<8)|(uint32_t(a[p+2])<<16)|(uint32_t(a[p+3])<<24);
}
// Bounded complete instrument route: discriminator/outer mode zero, selectors
// 0..4, ready or lazy ADSR, absent auxiliary modules and output word zero.
// Raw pitch may be zero (lookup) or nonzero (explicit); metadata is preserved.
// Adapter guards reject unsupported contexts before any mutation, not asserts.
inline Voice246NoteDispatchResult voice246InstrumentNoteOn(
        Voice246NoteArgument& argument,Voice246NoteArgument& local,uint8_t& flag,
        std::array<uint8_t,128>& map,uint32_t outputWord,uint32_t discriminator,
        DrWave246Pool& waves,Voice246Registers& registers,
        std::array<Adsr246State,6>& envelopes,Voice246ReleaseList& list,uint8_t gate,
        int loaded,int selected,int16_t selectorWord,int32_t tuning,uint32_t& random,
        uint32_t* ready=nullptr) noexcept {
    using S=Voice246NoteDispatchResult;
    const auto note=voice246ArgumentWord(argument,0);
    const auto mode=floorDivide(selectorWord,4096);
    if(note>=128||discriminator||outputWord||mode<0||mode>4||list.count>128||
       loaded<0||loaded>=8||selected<0||selected>=8)return S::rejected;
    const auto listWord=[&](size_t p){return uint32_t(list.bytes[p])|(uint32_t(list.bytes[p+1])<<8)|
        (uint32_t(list.bytes[p+2])<<16)|(uint32_t(list.bytes[p+3])<<24);};
    bool duplicate=false;
    if(!gate)for(size_t i=0;i<list.count;++i)duplicate|=listWord(i*20)==note;
    if(!gate&&!duplicate&&mode!=4){
        if(list.count==128)return S::rejected;
        if(mode==0){
            bool free=false,ageCandidate=false;
            for(size_t v=0;v<6;++v){free|=!registers.active[v];ageCandidate|=signedPhase(envelopes[v].blocks)>=0;}
            if(!free&&!ageCandidate&&(!ready||*ready!=0))return S::rejected;
        }
    }
    flag=1;
    if(voice246ArgumentWord(argument,4)==0xffffffffu){argument[4]=127;argument[5]=argument[6]=argument[7]=0;}
    for(size_t i=0;i<16;++i)if(i<9||i>=12)local[i]=argument[i];
    S status=S::rejected;
    const Voice246Note n{note,voice246ArgumentWord(argument,4),voice246ArgumentWord(argument,8),signedPhase(voice246ArgumentWord(argument,12))};
    if(gate)status=S::gated;
    else if(duplicate)status=S::duplicate;
    else if(mode==4)status=S::disabled;
    else if(mode==0){
        // The ORIGINAL scan accesses each occupied ADSR before reading its age,
        // even if a later voice is free. Cold access clears ALL six ages once.
        size_t v=0;
        for(;v<6&&registers.active[v];++v)(void)adsr246Access(envelopes,v,ready);
        if(v<6)(void)adsr246Access(envelopes,v,ready);
        if(n.explicitPitch){
            const auto r=drWave246PolyExplicitPitch(waves,registers,envelopes,list,local,0,loaded,selected,selectorWord,tuning);
            using P=DrWave246PolyExplicitPitchStatus;
            if(r.status==P::freeAppended||r.status==P::busyAppended)status=S::appended;
        }else if(v<6){
            const auto r=drWave246PolyFirstFree(waves,registers,envelopes,list,n,0,loaded,selected,tuning);
            if(r.status==DrWave246PolyFirstFreeStatus::appended)status=S::appended;
        }else{
            const auto r=drWave246PolySteal(registers,envelopes,list,n,0,tuning);
            if(r.status==DrWave246PolyStealStatus::appended)status=S::appended;
        }
    }else if(list.count==0){
        if(n.explicitPitch){
            const auto r=drWave246EmptyExplicitPitch(waves,registers,envelopes,list,local,0,loaded,selected,selectorWord,tuning,random,ready);
            if(r.status==DrWave246EmptyExplicitPitchStatus::appended)status=S::appended;
        }else{
            const auto r=drWave246NoteModes(waves,registers,envelopes,list,n,0,loaded,selected,int(mode),tuning,random,ready);
            if(r.status==DrWave246NoteModesStatus::appended)status=S::appended;
        }
    }else{
        const auto r=drWave246HeldNoteModes(registers,envelopes,list,local,0,selectorWord,tuning,random,ready);
        if(r.status==DrWave246HeldNoteModesStatus::appended)status=S::appended;
    }
    if(status!=S::rejected&&argument[8])voice246QuietNotifyOn(note,voice246ArgumentWord(argument,4),map,outputWord);
    return status;
}
inline Voice246ReleaseResult voice246InstrumentNoteOff(const Voice246NoteArgument& argument,
        Voice246NoteArgument& local,std::array<uint8_t,128>& map,uint32_t outputWord,
        uint32_t discriminator,uint8_t gate,Voice246ReleaseList& list,int16_t selectorWord,
        std::array<Adsr246State,6>& envelopes,int16_t sustain,int32_t tuning,
        std::array<int32_t,6>& pitch,uint32_t& random,uint32_t* ready=nullptr) noexcept {
    const auto note=voice246ArgumentWord(argument,0);const auto mode=floorDivide(selectorWord,4096);
    if(note>=128||discriminator||gate||outputWord||mode<0||mode>4||list.count>128||sustain<0)return Voice246ReleaseResult::rejected;
    const auto r=mode==0?voice246PolyRelease(list,note,envelopes,sustain,ready):
        voice246ReleaseSelectors(list,note,selectorWord,envelopes,sustain,tuning,pitch,random,ready);
    if(r==Voice246ReleaseResult::rejected)return r;
    for(size_t i=0;i<16;++i)if(i<9||i>=12)local[i]=argument[i];
    if(argument[8])voice246QuietNotifyOff(note,map,outputWord);
    return r;
}
}
