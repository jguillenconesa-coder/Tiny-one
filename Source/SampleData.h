// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
namespace pocket {
inline constexpr int kitCapacity=24;
struct PadSettings {float start=0,end=1,gain=1,pitch=0;bool reverse=false,gated=false;int nativeMode=1;};
struct Sample {
    std::vector<float> frames;double rate=48000;std::vector<int16_t> drumPcm;
    // Authored offline import conversion. Never call from the audio callback.
    // Keep the original float audio for the legacy player, export and projects.
    void prepareDrumPcm() {
        if(frames.empty()||!std::isfinite(rate)||rate<=0){drumPcm.clear();return;}
        const auto count=size_t(std::min(529200.,std::ceil(double(frames.size())*44100./rate)));drumPcm.resize(count);
        const auto bounded=[](float x){return std::isfinite(x)?std::clamp(double(x),-1.,1.):0.;};
        for(size_t i=0;i<count;++i){const auto position=double(i)*rate/44100.;const auto a=std::min(size_t(position),frames.size()-1),b=std::min(a+1,frames.size()-1);const auto fraction=position-std::floor(position);const auto value=bounded(frames[a])+(bounded(frames[b])-bounded(frames[a]))*fraction;drumPcm[i]=int16_t(std::clamp(std::lround(value*32768.),-32768l,32767l));}
    }
};
using Kit=std::array<Sample,kitCapacity>;
}
