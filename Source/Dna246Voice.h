// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Dna246.h"
#include "Phase246.h"
namespace pocket {
inline constexpr std::array<const char*,4> dna246Ids{{"dna246Blue","dna246Green","dna246White","dna246Orange"}};
inline constexpr std::array<const char*,4> dna246EnvelopeIds{{"dna246Attack","dna246Decay","dna246Sustain","dna246Release"}};
inline constexpr std::array<int,4> dna246Min{{-29491,4608,0,0}},dna246Max{{32767,12800,32767,32767}};
inline constexpr std::array<int16_t,4> dna246Defaults{{-8192,8192,8192,8192}};
// Own fixed-rate scheduling/ADSR/output around the original callback and an
// explicit virtual-device identity/RAM profile, not recovered physical OTP/RAM.
struct Dna246Voice {
    std::array<int16_t,128> block{};Phase246Voice output;
    unsigned cursor=128;bool initialized=false,releaseSent=false;
    double previousInput=0,previousDc=0;
    float source(double frequency,const std::array<int16_t,4>& controls,const std::array<int16_t,4>& adsr,
                 reference::Dna246Context& context,size_t slot,uint32_t& rng,const Phase246Config& rate,bool released) noexcept {
        auto& state=context.pool[slot];
        if(!initialized){state={};state.construct();state.reset();initialized=true;}
        if(released&&!releaseSent){state.release();releaseSent=true;}
        if(cursor==128){
            auto pitch=uint32_t(std::clamp(frequency,0.,Phase246Config::internalRate*.49)/Phase246Config::internalRate*4294967296.+.5);
            if(output.tremolo.enabled||output.midi.enabled){std::array<uint32_t,128> values{};values.fill(pitch);output.tremolo.pitchBlock(values,rate.table);output.midi.pitchBlock(values);pitch=values[0];}
            pitch=uint32_t(std::clamp(reference::signedPhase(pitch),int32_t(0x50000),int32_t(0x7fffffff)));
            const auto period=reference::dna246Period(pitch);state.phaseA%=period;state.phaseB%=period;
            if(state.counter>=0x7ffffe00u)state.counter=0;
            std::array<int16_t,4> bounded{};for(size_t i=0;i<4;++i)bounded[i]=int16_t(std::clamp(int(controls[i]),dna246Min[i],dna246Max[i]));
            if(!reference::dna246Block(context,slot,bounded,pitch,rng,block))block.fill(0);cursor=0;
        }
        const double input=block[cursor++]/32768.;const double dc=input-previousInput+.998932000*previousDc;previousInput=input;previousDc=dc;
        return output.shapeSample(dc,rate,&adsr,released);
    }
    float next(double frequency,const std::array<int16_t,4>& controls,const std::array<int16_t,4>& adsr,
               reference::Dna246Context& context,size_t slot,uint32_t& rng,const Phase246Config& rate,bool released=false) noexcept {
        if(!output.started){output.first=source(frequency,controls,adsr,context,slot,rng,rate,released);output.second=source(frequency,controls,adsr,context,slot,rng,rate,released);output.started=true;}
        const float result=output.first+float(output.position)*(output.second-output.first);output.position+=rate.step;
        while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(frequency,controls,adsr,context,slot,rng,rate,released);}
        return result;
    }
};
}
