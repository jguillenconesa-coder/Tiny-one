// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Cluster246.h"
#include "Phase246.h"
namespace pocket {
inline constexpr std::array<const char*,4> cluster246Ids{{"cluster246Blue","cluster246Green","cluster246White","cluster246Orange"}};
inline constexpr std::array<const char*,4> cluster246EnvelopeIds{{"cluster246Attack","cluster246Decay","cluster246Sustain","cluster246Release"}};
inline constexpr std::array<int,4> cluster246Defaults{{8192,0,12000,30}};
struct Cluster246Config {
    reference::Cluster246Tables tables;
    Cluster246Config() noexcept {
        for(int count=1;count<=8;++count){const auto observed=reference::cluster246ObservedNormalization(count);tables.normalization[size_t(count-1)]=observed?*observed:int16_t(16384/count);}
        std::copy(reference::cluster246Damping.begin(),reference::cluster246Damping.end(),tables.damping.begin());
    }
};
// Authored hybrid adapter. Missing normalization counts 2/3/8 use floor(16384/n),
// not recovered L1 words. Pitch, independent RNG/workspace, clock, ADSR routing,
// DC removal and rate conversion are ours; the verified kernel stays unchanged.
struct Cluster246Voice {
    reference::Cluster246State state;
    reference::Cluster246Context context;
    std::array<int16_t,128> block{};
    Phase246Voice output;
    unsigned cursor=128;
    double previousInput=0,previousDc=0;
    bool initialized=false,releaseSent=false;
    float source(double frequency,const std::array<int16_t,4>& controls,
                 const std::array<int16_t,4>& adsr,const Cluster246Config& config,
                 const Phase246Config& rate,bool released) noexcept {
        std::array<int16_t,4> bounded{};
        for(size_t i=0;i<4;++i)bounded[i]=int16_t(std::clamp(controls[i],reference::cluster246ControlMin[i],reference::cluster246ControlMax[i]));
        if(!initialized){reference::cluster246Reset(state,context,bounded,config.tables);initialized=true;}
        if(released&&!releaseSent){reference::cluster246Release(state);releaseSent=true;}
        if(cursor==128){
            std::array<uint32_t,128> pitch{};
            pitch.fill(uint32_t(std::clamp(frequency,0.,Phase246Config::internalRate*.49)/Phase246Config::internalRate*4294967296.+.5));
            output.tremolo.pitchBlock(pitch,rate.table);
            if(!reference::cluster246Block(state,context,bounded,config.tables,pitch,rate.table,block))block.fill(0);
            reference::cluster246DisplayDecay(context);cursor=0;
        }
        const double input=block[cursor++]/32768.;
        const double dc=input-previousInput+.998932000*previousDc;
        previousInput=input;previousDc=dc;
        return output.shapeSample(dc,rate,&adsr,released);
    }
    float next(double frequency,const std::array<int16_t,4>& controls,
               const std::array<int16_t,4>& adsr,const Cluster246Config& config,
               const Phase246Config& rate,bool released=false) noexcept {
        if(!output.started){output.first=source(frequency,controls,adsr,config,rate,released);output.second=source(frequency,controls,adsr,config,rate,released);output.started=true;}
        const float result=output.first+float(output.position)*(output.second-output.first);
        output.position+=rate.step;
        while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(frequency,controls,adsr,config,rate,released);}
        return result;
    }
};
}
