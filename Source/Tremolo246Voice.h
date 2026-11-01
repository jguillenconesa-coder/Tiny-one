// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Tremolo246.h"
#include "Tremolo246Tables.h"
namespace pocket {
inline constexpr std::array<const char*,4> tremolo246Ids{{"tremolo246Speed","tremolo246Pitch","tremolo246Volume","tremolo246Envelope"}};
// Own scheduler/routing around the verified callback arithmetic. Phase/Digital
// stream each internal sample; Dr. Wave/Cluster prepare pitch before core blocks.
struct Tremolo246Voice {
    reference::Tremolo246State state;
    std::array<uint32_t,8> display{};
    std::array<int16_t,8> controls{},blockControls{};
    reference::Tremolo246Coefficients coefficients{{},0,0};
    uint32_t tempo=0;
    unsigned frame=0;
    bool enabled=false,active=false,started=false,blockPitch=false;
    void configure(bool on,const std::array<int16_t,4>& raw,int shape,uint32_t bpm,bool blocked) noexcept {
        enabled=on;tempo=bpm;blockPitch=blocked;
        for(size_t i=0;i<4;++i)controls[i]=int16_t(std::clamp(int(raw[i]),i==1?-32768:0,32767));
        controls[7]=int16_t(std::clamp(shape,0,4)*6554);
        if(!enabled){active=started=false;frame=0;}
    }
    void begin(const std::array<int16_t,8192>& sine) noexcept {
        (void)sine;blockControls=controls;
        if(!active){state={};display={};reference::tremolo246NoteOn(state,display,blockControls);active=true;}
        coefficients=reference::Tremolo246Coefficients(blockControls,tempo,state.gate);frame=0;started=true;
    }
    uint32_t pitch(uint32_t input,const std::array<int16_t,8192>& sine) noexcept {
        if(!enabled)return input;if(!started)begin(sine);
        return reference::tremolo246Pitch(state,coefficients,input,sine);
    }
    void pitchBlock(std::array<uint32_t,128>& values,const std::array<int16_t,8192>& sine) noexcept {
        if(!enabled)return;begin(sine);
        for(auto& value:values)value=reference::tremolo246Pitch(state,coefficients,value,sine);
    }
    int16_t amplitude(int16_t input,const std::array<int16_t,8192>& sine) noexcept {
        if(!enabled||(!started&&blockPitch))return input;
        if(!started)begin(sine);
        int16_t carrier=0;const auto result=reference::tremolo246Amplitude(state,coefficients,input,sine,carrier);
        if(++frame==128){reference::tremolo246Finish(state,display,coefficients,blockControls[3],carrier,sine,reference::tremolo246Decay);started=false;}
        return result;
    }
    float phase() const noexcept {return float(double(state.volumePhase^0x80000000u)/4294967296.);}
};
}
