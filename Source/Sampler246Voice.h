// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Sampler246.h"
#include "Drum246Tables.h"
#include "Phase246.h"
#include "SampleData.h"
namespace pocket {
// Authored clock, trim mapping, guarded cache view and output adaptation.
// The host keeps its authored ADSR/velocity/allocator around this reader.
struct Sampler246Voice {
    reference::Sampler246State state;Phase246Voice output;std::array<int16_t,128> block{};
    unsigned cursor=128,tail=0;bool initialized=false,releaseSent=false,finished=false;
    static size_t frames(const Sample& s) noexcept {return std::min(s.drumPcm.size(),size_t(reference::sampler246Position(32767)/4096));}
    static int16_t encode(double frame) noexcept {return int16_t(std::clamp(std::lround(frame*4096.*32768./1083801600.),0l,32767l));}
    float source(double ratio,const Sample& sample,float trim,float length,bool loop,bool reverse,const Phase246Config& config,bool released) noexcept {
        if(cursor==128){
            const auto count=frames(sample);const double start=std::clamp(double(trim),0.,1.)*.9*count,end=start+(.01+std::clamp(double(length),0.,1.)*.99)*(count-start);
            std::array<int16_t,8> c{{encode(start),encode(start),encode(loop?end:start),encode(end),int16_t(reverse?16384:0),0,0,16384}};
            if(!initialized){state.construct();state.bind(1);state.reset(c);initialized=true;}
            if(released&&!releaseSent){state.release();releaseSent=true;}
            std::array<uint32_t,128> pitch{};pitch.fill(uint32_t(std::clamp(std::llround(std::max(0.,ratio)*262144.)+int64_t(ratio>0),0ll,2147483647ll)));
            reference::sampler246Block(state,c,pitch,0x7fffffff,{sample.drumPcm.data(),count,-8},reference::drum246FirCoefficients,block);
            const bool ended=!loop||released;
            if(ended&&(reverse?state.position<reference::sampler246Position(c[0]):state.position>=reference::sampler246Position(c[3]))){if(tail==0)tail=512;}
            if(count==0){finished=true;block.fill(0);}cursor=0;
        }
        const auto value=block[cursor++]/32768.;if(tail>0&&--tail==0)finished=true;
        return output.shapeSample(value,config);
    }
    float next(double ratio,const Sample& sample,float trim,float length,bool loop,bool reverse,const Phase246Config& config,bool released) noexcept {
        if(!output.started){output.first=source(ratio,sample,trim,length,loop,reverse,config,released);output.second=source(ratio,sample,trim,length,loop,reverse,config,released);output.started=true;}
        const auto value=output.first+float(output.position)*(output.second-output.first);output.position+=config.step;
        while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(ratio,sample,trim,length,loop,reverse,config,released);}return value;
    }
};
}
