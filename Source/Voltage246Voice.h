// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Voltage246.h"
#include "Phase246.h"
namespace pocket {
inline constexpr std::array<const char*,4> voltage246Ids{{"voltage246Blue","voltage246Green","voltage246White","voltage246Orange"}};
inline constexpr std::array<const char*,4> voltage246EnvelopeIds{{"voltage246Attack","voltage246Decay","voltage246Sustain","voltage246Release"}};
inline constexpr std::array<int,4> voltage246Min{{0,0,0,0}},voltage246Max{{32767,32767,32767,32767}};
inline constexpr std::array<int16_t,4> voltage246Defaults{{12000,4000,16000,8000}};
// Own clock, voice policy, ADSR composition, DC/output filter and resampling.
// Core pitch is a full 128-word stream; no FM fractional extension is used.
struct Voltage246Voice {
    reference::Voltage246State state;
    std::array<int16_t,128> block{};std::array<int16_t,32> display{};Phase246Voice output;
    unsigned cursor=128;bool initialized=false,releaseSent=false;
    double previousInput=0,previousDc=0;
    float source(double frequency,const std::array<int16_t,4>& controls,const std::array<int16_t,4>& adsr,
                 const Phase246Config& rate,bool released) noexcept {
        if(!initialized){state={};state.construct();state.reset();initialized=true;}
        if(released&&!releaseSent){state.release();releaseSent=true;}
        if(cursor==128){
            std::array<uint32_t,128> pitch{};
            pitch.fill(uint32_t(std::clamp(frequency,0.,Phase246Config::internalRate*.49)/Phase246Config::internalRate*4294967296.+.5));
            output.tremolo.pitchBlock(pitch,rate.table);
            std::array<int16_t,4> bounded{};for(size_t i=0;i<4;++i)bounded[i]=int16_t(std::clamp(int(controls[i]),voltage246Min[i],voltage246Max[i]));
            reference::voltage246Block(state,bounded,pitch,rate.table,block,display);cursor=0;
        }
        const double input=block[cursor++]/32768.;const double dc=input-previousInput+.998932000*previousDc;previousInput=input;previousDc=dc;
        return output.shapeSample(dc,rate,&adsr,released);
    }
    float next(double frequency,const std::array<int16_t,4>& controls,const std::array<int16_t,4>& adsr,
               const Phase246Config& rate,bool released=false) noexcept {
        if(!output.started){output.first=source(frequency,controls,adsr,rate,released);output.second=source(frequency,controls,adsr,rate,released);output.started=true;}
        const float result=output.first+float(output.position)*(output.second-output.first);output.position+=rate.step;
        while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(frequency,controls,adsr,rate,released);}
        return result;
    }
};
}
