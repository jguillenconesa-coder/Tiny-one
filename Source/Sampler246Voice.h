// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Sampler246Controls.h"
#include "Drum246Tables.h"
#include "Phase246.h"
#include "SampleData.h"
namespace pocket {
// Authored clock, bounded PCM view, loop gate and output adaptation.
// Reader and optional shared ADSR kernels are verified separately; their
// scheduling/composition and physical output route remain our host policy.
struct Sampler246Voice {
    reference::Sampler246State state;Phase246Voice output;std::array<int16_t,128> block{};
    unsigned cursor=128,tail=0;bool initialized=false,releaseSent=false,finished=false;
    static size_t frames(const Sample& s) noexcept {return std::min(s.drumPcm.size(),size_t(reference::sampler246Position(32767)/4096));}
    static int16_t encode(double frame) noexcept {return int16_t(std::clamp(std::lround(frame*4096.*32768./1083801600.),0l,32767l));}
    static std::array<int16_t,8> clipControls(const Sample& sample) noexcept {auto c=sampler246Defaults;c[2]=c[3]=encode(double(frames(sample)));return c;}
    float source(double ratio,const Sample& sample,const std::array<int16_t,8>& controls,bool loop,const Phase246Config& config,bool released,const std::array<int16_t,4>* adsr=nullptr) noexcept {
        if(cursor==128){
            const auto count=frames(sample);auto c=controls;
            if(!loop)c[2]=c[0]; // Explicit host LOOP gate; original loop detection is implicit.
            const bool reverse=c[4]/16384!=0;
            if(!initialized){state.construct();state.bind(1);state.reset(c);initialized=true;}
            if(released&&!releaseSent){state.release();releaseSent=true;}
            std::array<uint32_t,128> pitch{};pitch.fill(uint32_t(std::clamp(std::llround(std::max(0.,ratio)*262144.)+int64_t(ratio>0),0ll,2147483647ll)));
            reference::sampler246Block(state,c,pitch,0x7fffffff,{sample.drumPcm.data(),count,-8},reference::drum246FirCoefficients,block);
            const bool ended=!loop||c[0]>=c[2]||released;
            if(ended&&(reverse?state.position<reference::sampler246Position(c[0]):state.position>=reference::sampler246Position(c[3]))){if(tail==0)tail=512;}
            if(count==0){finished=true;block.fill(0);}cursor=0;
        }
        const auto value=block[cursor++]/32768.;if(tail>0&&--tail==0)finished=true;
        return output.shapeSample(value,config,adsr,released);
    }
    float next(double ratio,const Sample& sample,const std::array<int16_t,8>& controls,bool loop,const Phase246Config& config,bool released,const std::array<int16_t,4>* adsr=nullptr) noexcept {
        if(!output.started){output.first=source(ratio,sample,controls,loop,config,released,adsr);output.second=source(ratio,sample,controls,loop,config,released,adsr);output.started=true;}
        const auto value=output.first+float(output.position)*(output.second-output.first);output.position+=config.step;
        while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(ratio,sample,controls,loop,config,released,adsr);}return value;
    }
};
}
