// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "DrWave246Kernel.h"
#include "Phase246.h"
namespace pocket {
inline constexpr std::array<const char*,4> drWave246Ids{{"drWave246Blue","drWave246Green","drWave246White","drWave246Orange"}};
inline constexpr std::array<const char*,4> drWave246EnvelopeIds{{"drWave246Attack","drWave246Decay","drWave246Sustain","drWave246Release"}};
inline constexpr std::array<int,4> drWave246Max{{24575,16383,16383,32767}};
// Shared generated tables: prepared once off the audio thread, never per note.
struct DrWave246Config {
    reference::DrWave246WaveBank bank;
    bool ready=false;
    void prepare(const Phase246Config& output) noexcept {
        if(ready)return;
        reference::DrWave246WaveSettings settings;settings.referenceGain=32768;settings.outputGain=262144;
        reference::drWave246BuildWaveBank(bank,settings,output.table);ready=true;
    }
};
// Hybrid adapter around the unmodified verified 128-sample core. A440 pitch,
// six-block wave refresh, independent per-voice workspace, ADSR routing, DC
// removal and causal host-rate output are ours, not the missing L1 scheduler.
struct DrWave246Voice {
    reference::DrWave246State state;
    reference::DrWave246Waves waves{};
    reference::DrWave246Runtime runtime;
    std::array<int16_t,128> block{};
    Phase246Voice output;
    unsigned cursor=128,counter=0;
    double previousInput=0,previousDc=0;
    bool initialized=false;
    float source(double frequency,const std::array<int16_t,4>& controls,
                 const std::array<int16_t,4>& adsr,const DrWave246Config& config,
                 const Phase246Config& rate,bool released) noexcept {
        if(!initialized){state.construct(0);runtime.settings={0x100,0,32768,262144};initialized=true;}
        if(cursor==128){
            std::array<int16_t,4> bounded{};
            for(size_t i=0;i<4;++i)bounded[i]=int16_t(std::clamp(int(controls[i]),0,drWave246Max[i]));
            std::array<uint32_t,128> pitch{};
            pitch.fill(uint32_t(std::clamp(frequency,0.,Phase246Config::internalRate*.49)/Phase246Config::internalRate*4294967296.+.5));
            output.tremolo.pitchBlock(pitch,rate.table);output.midi.pitchBlock(pitch);
            const auto result=reference::drWave246Block(state,waves,runtime,config.bank,rate.table,bounded,pitch,int(counter),block);
            if(result!=reference::DrWave246ReadResult::rendered)block.fill(0);
            counter=(counter+1)%6;cursor=0;
        }
        const double input=block[cursor++]/32768.;
        // Our 15 Hz DC blocker removes offsets introduced by wave transforms.
        const double dc=input-previousInput+.998932000*previousDc;
        previousInput=input;previousDc=dc;
        return output.shapeSample(dc,rate,&adsr,released);
    }
    float next(double frequency,const std::array<int16_t,4>& controls,
               const std::array<int16_t,4>& adsr,const DrWave246Config& config,
               const Phase246Config& rate,bool released=false) noexcept {
        if(!output.started){output.first=source(frequency,controls,adsr,config,rate,released);output.second=source(frequency,controls,adsr,config,rate,released);output.started=true;}
        const float result=output.first+float(output.position)*(output.second-output.first);
        output.position+=rate.step;
        while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(frequency,controls,adsr,config,rate,released);}
        return result;
    }
};
}
