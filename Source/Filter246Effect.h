// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Filter246.h"
#include "Midi246.h"
#include "Manual246FxAudio.h"
#include "Filter246Tables.h"
#include "Filter246Controls.h"
#include "Nitro246.h"
#include "Nitro246Controls.h"
#include "Delay246.h"
#include "Delay246Controls.h"
#include "Delay246Tables.h"
#include "Grid246.h"
#include "Grid246Controls.h"
#include "Cwo246Kernel.h"
#include "Cwo246Controls.h"
#include "Spring246.h"
#include "Spring246Tables.h"
#include "Spring246Controls.h"
#include "Phone246.h"
#include "Phone246Controls.h"
#include "Phone246Tables.h"
#include <cmath>
#include <vector>
namespace pocket {
// Authored causal host adapter, 44.1 kHz internal blocks. Complete-block peak
// dependency is preserved. A fixed dry delay aligns bypass and all output paths.
struct Filter246Effect {
    static constexpr double internalRate=44100;
    reference::Filter246State state;reference::Nitro246State nitroState;bool wasNitro=false,wasDelay=false,wasGrid=false,wasPhone=false,wasSpring=false,wasCwo=false;
    reference::Delay246State delayState;std::array<int32_t,44100> delayMemory{};
    reference::Grid246State gridState;reference::Grid246Memory gridMemory{};
    reference::Cwo246State cwoState;reference::Cwo246DelayMemory cwoMemory{};
    reference::Spring246State springState;reference::Spring246Memory springMemory{};int32_t springPending=0;
    reference::Phone246State phoneState;reference::Phone246Memory phoneMemory{};
    std::array<uint32_t,16> meters{};
    std::array<int16_t,64> block{};
    std::array<int16_t,512> history{};
    std::vector<float> dry;
    struct Biquad {double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;double next(double x) noexcept {const double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return y;}};
    std::array<Biquad,2> filters{};
    double hostRate=48000,ratio=48000./44100,previous=0;
    uint64_t hostPosition=0,inputPosition=0;int64_t completed=-1;
    int latency=0;unsigned cursor=0;uint64_t underruns=0;
    float mix=0,smoothing=0;bool initialized=false,wasEnabled=false,awake=false,removeDC=true,guardSilence=true;
    uint64_t firstSignal=~uint64_t(0),firstSignalHost=~uint64_t(0);
    double dcInput=0,dcOutput=0,dcCoefficient=0;
    static int latencyFor(double rate) noexcept {return int(std::ceil(64*rate/internalRate))+1;}
    void prepare(double rate) {
        hostRate=std::max(8000.,rate);ratio=hostRate/internalRate;latency=latencyFor(hostRate);dry.resize(size_t(latency)+2);smoothing=float(1-std::exp(-1/(hostRate*.012)));dcCoefficient=std::exp(-2*3.14159265358979323846*15/hostRate);
        constexpr double pi=3.14159265358979323846;const double w=2*pi*std::min(17640.,hostRate*.4)/hostRate,cs=std::cos(w),sn=std::sin(w);const double q[]={.541196100146197,1.306562964876377};
        for(size_t i=0;i<2;++i){const double alpha=sn/(2*q[i]),a0=1+alpha;filters[i]={(1-cs)/(2*a0),(1-cs)/a0,(1-cs)/(2*a0),-2*cs/a0,(1-alpha)/a0,0,0};}
        reset();
    }
    void reset() noexcept {cwoState={};cwoMemory.fill(0);springState={};springMemory.fill(0);springPending=0;state={};nitroState={};delayState={};delayMemory.fill(0);gridState={};gridMemory.fill(0);phoneState={};phoneMemory.fill(0);wasNitro=wasDelay=wasGrid=wasPhone=wasSpring=wasCwo=false;meters.fill(0);block.fill(0);history.fill(0);std::fill(dry.begin(),dry.end(),0.f);for(auto& f:filters)f.z1=f.z2=0;hostPosition=inputPosition=0;completed=-1;previous=0;cursor=0;underruns=0;mix=0;dcInput=dcOutput=0;initialized=wasEnabled=awake=false;firstSignal=firstSignalHost=~uint64_t(0);}
    float next(float input,bool enabled,const std::array<int16_t,4>& controls,const std::array<int16_t,8192>& sine,bool nitro=false,bool delay=false,bool grid=false,bool phone=false,bool spring=false,bool cwo=false,const Midi246FxAudio* midiAudio=nullptr,const Manual246FxAudio* manualAudio=nullptr) noexcept {
        if(dry.empty())return input;
        if(!initialized){mix=enabled?1.f:0.f;initialized=true;}
        if(enabled&&(!wasEnabled||nitro!=wasNitro||delay!=wasDelay||grid!=wasGrid||phone!=wasPhone||spring!=wasSpring||cwo!=wasCwo)){cwoState={};if(cwo)cwoMemory.fill(0);springState={};springPending=0;if(spring)springMemory.fill(0);phoneState={};if(phone)phoneMemory.fill(0);state={};nitroState={};delayState={};gridState={};if(delay)delayMemory.fill(0);if(grid)gridMemory.fill(0);awake=false;firstSignal=firstSignalHost=~uint64_t(0);dcInput=dcOutput=0;}wasEnabled=enabled;wasNitro=nitro;wasDelay=delay;wasGrid=grid;wasPhone=phone;wasSpring=spring;wasCwo=cwo;if(input!=0){awake=true;if(enabled&&firstSignalHost==~uint64_t(0))firstSignalHost=hostPosition;}
        const float clean=hostPosition>=uint64_t(latency)?dry[size_t((hostPosition-uint64_t(latency))%dry.size())]:0;
        dry[size_t(hostPosition%dry.size())]=input;
        double filtered=input;if(hostRate!=internalRate)for(auto& f:filters)filtered=f.next(filtered);
        while(double(inputPosition)*ratio<=double(hostPosition)+1e-9){
            const double fraction=hostPosition==0?1:std::clamp(double(inputPosition)*ratio-(double(hostPosition)-1),0.,1.);
            const auto sample=previous+(filtered-previous)*fraction;
            block[cursor++]=int16_t(std::clamp(std::lround(sample*32768),-32768l,32767l));
            if(enabled&&manualAudio)block[cursor-1]=manualAudio->apply(block[cursor-1]);
            if(enabled&&midiAudio)block[cursor-1]=midiAudio->apply(block[cursor-1]);
            if(enabled&&block[cursor-1]!=0&&firstSignal==~uint64_t(0))firstSignal=inputPosition;
            if(cursor==64){
                if(enabled&&awake){std::array<int16_t,4> bounded;for(size_t i=0;i<4;++i)bounded[i]=int16_t(std::clamp(int(controls[i]),cwo?cwo246Min[i]:spring?spring246Min[i]:phone?phone246Min[i]:grid?grid246Min[i]:delay?delay246Min[i]:nitro?nitro246Min[i]:filter246Min[i],cwo?cwo246Max[i]:spring?spring246Max[i]:phone?phone246Max[i]:grid?grid246Max[i]:delay?delay246Max[i]:nitro?nitro246Max[i]:filter246Max[i]));if(cwo)reference::cwo246HybridBlock(cwoState,bounded,cwoMemory,block,meters);else if(spring)reference::spring246Block(springState,bounded,reference::filter246Frequency,reference::spring246Low,reference::spring246Constants,springMemory,springPending,block,meters);else if(phone)reference::phone246Block(phoneState,bounded,reference::delay246Interpolation,reference::saturate32(int64_t(2147483647)-reference::filter246Frequency[0]),reference::phone246LowCoefficient,phoneMemory,block,meters);else if(grid){if(!gridState.scanFits(2*(bounded[0]/128),2*(bounded[1]/128)))gridState.reset();reference::grid246Block(gridState,bounded,sine,gridMemory,block,meters);}else if(delay)reference::delay246Block(delayState,bounded,reference::delay246Interpolation,delayMemory,block,meters);else if(nitro)reference::nitro246HybridBlock(nitroState,bounded,reference::nitro246HybridMix,reference::filter246Frequency,sine,block,meters);else reference::filter246Block(state,bounded,reference::filter246Mix,reference::filter246Frequency,sine,block,meters);}
                const auto start=inputPosition-63;for(size_t i=0;i<64;++i)history[size_t((start+i)%history.size())]=(enabled&&guardSilence&&start+i<firstSignal)?0:block[i];completed=int64_t(inputPosition);cursor=0;
            }
            ++inputPosition;
        }
        float wet=0;
        if(hostPosition>=uint64_t(latency)){
            const double position=(double(hostPosition)-latency)/ratio;const auto first=int64_t(std::floor(position));const double fraction=position-double(first);
            if(first+(fraction>1e-9?1:0)<=completed){const float a=history[size_t(first)%history.size()]/32768.f,b=history[size_t(first+1)%history.size()]/32768.f;wet=a+float(fraction)*(b-a);}
            else ++underruns;
        }
        if(enabled&&guardSilence&&(firstSignalHost==~uint64_t(0)||hostPosition<firstSignalHost+uint64_t(latency)))wet=0;
        if(removeDC){const double filteredWet=wet-dcInput+dcCoefficient*dcOutput;dcInput=wet;dcOutput=filteredWet;wet=float(filteredWet);}
        mix+=smoothing*((enabled?1.f:0.f)-mix);if(!enabled&&mix<.000001f)mix=0;
        previous=filtered;++hostPosition;return clean+(wet-clean)*mix;
    }
};
}
