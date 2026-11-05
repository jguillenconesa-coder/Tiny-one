// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Drum246.h"
#include "Drum246Allocator.h"
#include "Drum246Modulation.h"
#include "Drum246Tables.h"
#include "Phase246.h"
#include "SampleData.h"
namespace pocket {
inline constexpr std::array<const char*,4> drum246EnvelopeIds{{"drum246AttackLevel","drum246MidLevel","drum246ReleaseLevel","drum246Region"}};
inline constexpr std::array<int16_t,4> drum246EnvelopeDefaults{{0,32767,0,0}};
// Authored region/level model informed by manual §6.4. The original producer
// of the callback's gain buffer has not been reconstructed or numerically matched.
inline float drum246DynamicLevel(double progress,const std::array<int16_t,4>& controls) noexcept {
    progress=std::clamp(progress,0.,1.);const double region=std::clamp(double(controls[3])/65534.,0.,.5);
    const double a=std::clamp(double(controls[0])/32767.,0.,1.),m=std::clamp(double(controls[1])/32767.,0.,1.),r=std::clamp(double(controls[2])/32767.,0.,1.);
    if(region>0&&progress<region)return float(a+(m-a)*progress/region);if(region>0&&progress>1-region)return float(m+(r-m)*(progress-(1-region))/region);return float(m);
}
inline void drum246EncodePosition(double frame,int16_t& coarse,int16_t& fine) noexcept {
    const auto packed=uint32_t(std::clamp(std::llround(frame*2048.*2147483648./1083801600.),0ll,2147483647ll));coarse=int16_t(packed>>16);fine=int16_t((packed&65535u)/2);
}
// Explicit host conversion, not the recovered original pitch/control producer.
inline std::array<int16_t,8> drum246BaseControls(const PadSettings& pad,const Sample& sample) noexcept {
 std::array<int16_t,8> c{};const auto length=sample.drumPcm.size();
 c[0]=int16_t(std::clamp(std::lround(double(pad.pitch)*1024.+256),-24320l,24832l));
 drum246EncodePosition(std::clamp(double(pad.start),0.,1.)*length,c[1],c[5]);drum246EncodePosition(std::clamp(double(pad.end),0.,1.)*length,c[2],c[6]);
 c[3]=int16_t(std::clamp(pad.nativeMode,0,2)*8192+4096);c[4]=pad.reverse?24576:8192;c[7]=int16_t(std::clamp(std::lround(16384.*std::sqrt(std::clamp(double(pad.gain),0.,2.))),0l,32767l));return c;
}
struct Drum246Voice {
    reference::Drum246State state;Phase246Voice output;std::array<int16_t,128> block{},gain{};std::array<int32_t,128> derived{};
    unsigned cursor=128,tail=0;bool initialized=false,releaseSent=false,draining=false,finished=false;
    float envelope=0,progress=0;
    bool done() const noexcept {return finished;}
    float source(double bend,const PadSettings& pad,const Sample& sample,const std::array<int16_t,4>& dynamics,const Phase246Config& rate,bool released,const std::array<int16_t,8>* modulated=nullptr) noexcept {
        if(cursor==128){
            std::array<int16_t,8> controls{};const auto length=sample.drumPcm.size();
            drum246EncodePosition(std::clamp(double(pad.start),0.,1.)*length,controls[1],controls[5]);drum246EncodePosition(std::clamp(double(pad.end),0.,1.)*length,controls[2],controls[6]);
            controls[3]=int16_t(std::clamp(pad.nativeMode,0,2)*8192);controls[4]=pad.reverse?16384:0;controls[7]=int16_t(std::clamp(std::lround(16384.*std::sqrt(std::clamp(double(pad.gain),0.,2.))),0l,32767l));
            if(modulated)controls=*modulated;const bool reverse=controls[4]/16384!=0;const int playMode=controls[3]/8192;
            if(!initialized){state={};state.construct();state.bind(1);state.reset(controls);initialized=true;}
            if(released&&!releaseSent){state.release(controls);releaseSent=true;if(state.mode==0)draining=true;}
            const auto start=reference::drum246Position(controls[1],controls[5]),end=reference::drum246Position(controls[2],controls[6]);
            if(length==0||end<=start)draining=true;
            if(draining){block.fill(0);if(tail==0)tail=512;}
            else {
                const auto ratio=.5*std::exp2(std::clamp(modulated?(double(controls[0])-256)/1024.:double(pad.pitch),-24.,24.)/12.)*bend;
                std::array<uint32_t,128> pitch{};pitch.fill(uint32_t(std::clamp(std::llround(ratio*268435456.),0ll,2147483647ll)));output.tremolo.pitchBlock(pitch,rate.table);
                auto position=state.position;const bool loop=playMode==2&&state.mode==1;
                for(size_t i=0;i<128;++i){const auto fraction=std::clamp(double(position-start)/std::max(1.,double(end-start)),0.,1.);const auto level=drum246DynamicLevel(reverse?1-fraction:fraction,dynamics);gain[i]=int16_t(std::clamp(std::lround(16384.*std::sqrt(level)),0l,16384l));gain[i]=output.tremolo.amplitude(gain[i],rate.table);envelope=level;progress=float(fraction);const auto step=reference::floorDivide(reference::signedPhase(pitch[i]),131072);position=reference::saturate32(int64_t(position)+(reverse?-step:step));if(loop&&(reverse?position<start:position>=end))position=reverse?end:start;}
                // Eight-frame alignment compensates the supplied FIR window's
                // lookahead; guarded PCM addressing is an authored host policy.
                reference::drum246Block(state,controls,pitch,gain,{sample.drumPcm.data(),length,-8},reference::drum246FirCoefficients,block,derived);
                if(state.mode==0)draining=true;
            }
            cursor=0;
        }
        double value=block[cursor++]/32768.;if(tail>0&&--tail==0)finished=true;
        for(size_t i=0;i<2;++i){const auto& c=rate.filters[i];auto& m=output.memory[i];const double filtered=c.b0*value+m[0];m[0]=c.b1*value-c.a1*filtered+m[1];m[1]=c.b2*value-c.a2*filtered;value=filtered;}return float(value);
    }
    float next(double bend,const PadSettings& pad,const Sample& sample,const std::array<int16_t,4>& dynamics,const Phase246Config& rate,bool released=false,const std::array<int16_t,8>* modulated=nullptr) noexcept {
        if(!output.started){output.first=source(bend,pad,sample,dynamics,rate,released,modulated);output.second=source(bend,pad,sample,dynamics,rate,released,modulated);output.started=true;}
        const auto value=output.first+float(output.position)*(output.second-output.first);output.position+=rate.step;while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(bend,pad,sample,dynamics,rate,released,modulated);}return value;
    }
};
struct Drum246Bank {
    struct Hit {Drum246Voice voice;Drum246ModulationControl modulation;bool active=false,keyDown=false,released=false;int note=0,channel=1,pad=0;float velocity=0,last=0,stolen=0;uint64_t age=0;unsigned fade=0;};
    using Registry=std::array<std::array<int8_t,128>,17>;
    static Registry emptyRegistry() noexcept {Registry r{};for(auto& channel:r)channel.fill(-1);return r;}
    std::array<Hit,12> hits{};std::array<bool,17> pedals{};uint64_t age=0;
    // Host extension: original membership/owner checks, indexed by MIDI note and
    // channel. A stolen note stays held until its off, even after audio retires.
    Registry heldVoice=emptyRegistry();
    void reset() noexcept {*this={};}
    bool noteOn(int note,float velocity,int channel,int pad,double hostRate) noexcept {
        if(note<0||note>127||channel<1||channel>17||pad<0||pad>=24||!std::isfinite(velocity)||velocity<=0)return false;
        auto& held=heldVoice[size_t(channel-1)][size_t(note)];if(held>=0)return false;
        std::array<uint8_t,12> occupied{};for(size_t i=0;i<12;++i)occupied[i]=hits[i].active?1:0;
        int slot=reference::drum246SelectVoice(occupied,[&](unsigned v){return hits[v].voice.state.active();});
        // Defensive host policy for wrapped/negative counts; original asserts.
        if(slot<0)slot=0;
        auto* hit=&hits[size_t(slot)];const auto tail=hit->active?hit->last:0.f;const auto modulation=hit->modulation;
        *hit={};hit->modulation=modulation;hit->modulation.noteOn();hit->active=hit->keyDown=true;
        hit->note=note;hit->pad=pad;hit->channel=channel;hit->velocity=std::clamp(velocity,0.f,1.f);
        hit->age=++age;hit->stolen=tail;hit->fade=tail!=0?unsigned(hostRate*.003):0;held=int8_t(slot);return true;
    }
    void noteOff(int note,int channel) noexcept {
        if(note<0||note>127||channel<1||channel>17)return;
        auto& held=heldVoice[size_t(channel-1)][size_t(note)];const int slot=held;held=-1;if(slot<0)return;
        auto& h=hits[size_t(slot)];if(h.active&&h.note==note&&h.channel==channel){h.keyDown=false;if(!pedals[size_t(channel-1)])h.released=true;}
    }
    void pedal(int channel,bool down) noexcept {if(channel<1||channel>17)return;pedals[size_t(channel-1)]=down;if(!down)for(auto& h:hits)if(h.channel==channel&&!h.keyDown)h.released=true;}
    void allNotesOff(int channel) noexcept {
        if(channel<0||channel>17)return;
        for(auto& h:hits)if(channel==0||h.channel==channel){h.keyDown=false;h.released=true;}
        if(channel==0){pedals.fill(false);heldVoice=emptyRegistry();}else{pedals[size_t(channel-1)]=false;heldVoice[size_t(channel-1)].fill(-1);}
    }
    float next(const Kit& kit,const std::array<PadSettings,24>& pads,const std::array<int16_t,4>& dynamics,const std::array<float,17>& bends,double tune,const Phase246Config& rate,bool tremolo,const std::array<int16_t,4>& tremoloControls,int shape,uint32_t tempo,const Drum246ModulationConfig& modulation={},bool sharedSource=false) noexcept {
        float sum=0;for(auto& h:hits){if(!h.active)continue;h.voice.output.tremolo.configure(tremolo,tremoloControls,shape,tempo,true);const auto controls=h.modulation.next((modulation.element||modulation.crank||modulation.bend||modulation.midi||modulation.random||modulation.value)?drum246BaseControls(pads[size_t(h.pad)],kit[sharedSource?0:size_t(h.pad)]):std::array<int16_t,8>{},8,modulation,rate.table);auto value=h.voice.next(bends[size_t(h.channel-1)]*tune,pads[size_t(h.pad)],kit[sharedSource?0:size_t(h.pad)],dynamics,rate,h.released,h.modulation.applied?&controls:nullptr)*h.velocity*.16f;if(h.fade){const auto weight=float(h.fade)/float(std::max(1u,unsigned(rate.internalRate/rate.step*.003)));value=value*(1-weight)+h.stolen*weight;--h.fade;}h.last=value;sum+=value;if(h.voice.done())h.active=false;}return sum;
    }
};
}
