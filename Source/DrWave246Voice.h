// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "DrWave246Kernel.h"
#include "Phase246.h"
#include "Envelope246Transition.h"
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
    int dynamicsNote=-1;float dynamicsVelocity=-1;
    int restartNote=0;float restartVelocity=0;
    uint32_t renderedPitch=0;
    bool restartQueued=false,restartArmed=false,restartCancelled=false;
    static uint32_t hostPitch(double frequency) noexcept {
        return uint32_t((std::isfinite(frequency)?std::clamp(frequency,0.,Phase246Config::internalRate*.49):0.)/Phase246Config::internalRate*4294967296.+.5);
    }
    static int velocityCode(float velocity) noexcept {
        return std::isfinite(velocity)?int(std::clamp(velocity,0.f,1.f)*127.f+.5f):0;
    }
    // Authored boundary service. The request/ramp are recovered, but the
    // consumer timing, core reset and cancellation policy are not original L1.
    void requestRestart(int note,float velocity) noexcept {
        if(!output.envelopeStarted){setDynamics(note,velocity);return;}
        restartNote=note;restartVelocity=velocity;restartQueued=true;restartCancelled=false;
    }
    bool restartPending() const noexcept {return restartQueued||restartArmed;}
    uint32_t selectionBlocks() const noexcept {return restartQueued?0:output.envelope.blocks;}
    // Verified VoiceCtl note attenuation and signed velocity words. Mapping the
    // host float to MIDI codes and latching on our 128-frame clock are authored.
    void setDynamics(int note,float velocity) noexcept {
        if(restartPending()||restartCancelled||(note==dynamicsNote&&velocity==dynamicsVelocity))return;
        dynamicsNote=note;dynamicsVelocity=velocity;
        const int bounded=std::clamp(note,0,127);
        const auto code=velocityCode(velocity);
        output.envelopeVolume=int16_t(code*256);
        const uint32_t gain=bounded==0?32767u:32768u-uint32_t(bounded)*128u;
        output.envelope.gain=(output.envelope.gain&0xffff0000u)|gain;
    }
    float source(double frequency,const std::array<int16_t,4>& controls,
                 const std::array<int16_t,4>& adsr,const DrWave246Config& config,
                 const Phase246Config& rate,bool released) noexcept {
        if(!initialized){state.construct(0);runtime.settings={0x100,0,32768,262144};initialized=true;}
        if(released&&restartPending()){
            restartQueued=restartArmed=false;restartCancelled=true;
            output.envelopeReleased=false; // Release old pitch; do not resurrect the pending note.
        }
        if(cursor==128){
            bool committed=false;
            if(restartQueued){
                reference::adsr246RequestRestart(output.envelope,hostPitch(frequency),uint16_t(velocityCode(restartVelocity)*256));
                output.envelopeReleased=false;restartQueued=false;restartArmed=true;
            }else if(restartArmed&&output.envelope.stage==7){
                renderedPitch=output.envelope.field12;
                output.envelopeVolume=int16_t(output.envelope.field16>>16);
                dynamicsNote=restartNote;dynamicsVelocity=restartVelocity;
                // Preserve the old gain word, as the recovered busy poly helper does.
                state.construct(0);waves={};runtime={};runtime.settings={0x100,0,32768,262144};counter=0;
                restartArmed=false;committed=true;
            }
            if(!restartArmed&&!restartCancelled&&!committed)renderedPitch=hostPitch(frequency);
            std::array<int16_t,4> bounded{};
            for(size_t i=0;i<4;++i)bounded[i]=int16_t(std::clamp(int(controls[i]),0,drWave246Max[i]));
            std::array<uint32_t,128> pitch{};
            pitch.fill(renderedPitch);
            output.tremolo.pitchBlock(pitch,rate.table);output.midi.pitchBlock(pitch);output.bend.pitchBlock(pitch);
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
