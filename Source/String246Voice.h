// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "String246.h"
#include "Phase246.h"
namespace pocket {
inline constexpr std::array<const char*,4> string246Ids{{"string246Blue","string246Green","string246White","string246Orange"}};
inline constexpr std::array<const char*,4> string246EnvelopeIds{{"string246Attack","string246Decay","string246Sustain","string246Release"}};
inline constexpr std::array<int,4> string246Min{{64,512,0,8256}},string246Max{{8256,24064,16384,16448}};
inline constexpr std::array<int16_t,4> string246Defaults{{4096,18432,0,12288}};
// Authored six-voice scheduling, 88.2 kHz pitch/control clock, ADSR and output filter.
// State reset/release and the complete 128-sample String callback are reconstructed.
struct String246Voice {
    reference::String246State state;
    std::array<int16_t,128> block{};Phase246Voice output;
    unsigned cursor=128;bool initialized=false,releaseSent=false;
    double previousInput=0,previousDc=0;
    float source(double frequency,const std::array<int16_t,4>& controls,const std::array<int16_t,4>& adsr,
                 reference::String246Memory& memory,uint32_t& rng,const Phase246Config& rate,bool released) noexcept {
        if(!initialized){reference::string246Reset(state,rng);initialized=true;}
        if(released&&!releaseSent){state.release();releaseSent=true;}
        if(cursor==128){
            auto pitch=uint32_t(std::clamp(frequency,0.,Phase246Config::internalRate*.49)/Phase246Config::internalRate*4294967296.+.5);
            if(output.tremolo.enabled||output.midi.enabled||output.bend.enabled){std::array<uint32_t,128> values{};values.fill(pitch);output.tremolo.pitchBlock(values,rate.table);output.midi.pitchBlock(values);output.bend.pitchBlock(values);pitch=values[0];}
            // Own bounds for the original positive-divisor domain (about 5.4 Hz–44.1 kHz).
            pitch=uint32_t(std::clamp(reference::signedPhase(pitch),int32_t(0x40000),int32_t(0x7fffffff)));
            // Own cursor wrapping when the block-rate pitch shortens the loop.
            state.cursor%=reference::string246Period(pitch);
            std::array<int16_t,4> bounded{};for(size_t i=0;i<4;++i)bounded[i]=int16_t(std::clamp(int(controls[i]),string246Min[i],string246Max[i]));
            if(!reference::string246Block(state,memory,bounded,pitch,rng,block))block.fill(0);cursor=0;
        }
        const double input=block[cursor++]/32768.;const double dc=input-previousInput+.998932000*previousDc;previousInput=input;previousDc=dc;
        return output.shapeSample(dc,rate,&adsr,released);
    }
    float next(double frequency,const std::array<int16_t,4>& controls,const std::array<int16_t,4>& adsr,
               reference::String246Memory& memory,uint32_t& rng,const Phase246Config& rate,bool released=false) noexcept {
        if(!output.started){output.first=source(frequency,controls,adsr,memory,rng,rate,released);output.second=source(frequency,controls,adsr,memory,rng,rate,released);output.started=true;}
        const float result=output.first+float(output.position)*(output.second-output.first);output.position+=rate.step;
        while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(frequency,controls,adsr,memory,rng,rate,released);}
        return result;
    }
};
}
