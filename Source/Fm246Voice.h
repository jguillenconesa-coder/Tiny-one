// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Fm246.h"
#include "Phase246.h"
namespace pocket {
inline constexpr std::array<const char*,4> fm246Ids{{"fm246Blue","fm246Green","fm246White","fm246Orange"}};
inline constexpr std::array<const char*,4> fm246EnvelopeIds{{"fm246Attack","fm246Decay","fm246Sustain","fm246Release"}};
inline constexpr std::array<int,4> fm246Min{{0,0,1024,0}},fm246Max{{32767,32767,17408,32767}};
inline constexpr std::array<int16_t,4> fm246Defaults{{8192,10485,1024,4096}};
struct Fm246Config {
    std::array<int16_t,30> table=reference::fm246ConvertTables(reference::fm246ModelBits);
};
// Own six-voice scheduling, 88.2 kHz clock/pitch adapter and output filter.
// The 128-sample numerical kernel and internal contours remain unmodified.
struct Fm246Voice {
    reference::Fm246State state;
    reference::Fm246Workspace work;
    std::array<int16_t,128> block{};
    Phase246Voice output;
    unsigned cursor=128;bool initialized=false;
    float source(double frequency,const std::array<int16_t,4>& controls,const std::array<int16_t,4>& adsr,
                 const Fm246Config& config,const Phase246Config& rate,bool released) noexcept {
        if(!initialized){state.construct();state.reset();initialized=true;}
        if(cursor==128){
            // The core reads only the high pitch word. Round that word instead
            // of truncating it; pitch resolution and block cadence are explicit
            // integration limits, not a recovered original note scheduler.
            const auto pitch=uint32_t(std::clamp(std::round(frequency/Phase246Config::internalRate*65536.),0.,32767.))<<16;
            reference::fm246Block(state,work,controls,pitch,config.table,rate.table,block);cursor=0;
        }
        return output.shapeSample(block[cursor++]/32768.,rate,&adsr,released);
    }
    float next(double frequency,const std::array<int16_t,4>& controls,const std::array<int16_t,4>& adsr,
               const Fm246Config& config,const Phase246Config& rate,bool released=false) noexcept {
        if(!output.started){output.first=source(frequency,controls,adsr,config,rate,released);output.second=source(frequency,controls,adsr,config,rate,released);output.started=true;}
        const float result=output.first+float(output.position)*(output.second-output.first);
        output.position+=rate.step;
        while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(frequency,controls,adsr,config,rate,released);}
        return result;
    }
};
}
