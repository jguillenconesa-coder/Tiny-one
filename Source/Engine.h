// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include "Phase246.h"
#include "Manual246FxAudio.h"
#include "Element246FxAudio.h"
#include "Element246Gain.h"
#include "Element246VoiceInput.h"
#include "SharedDrum246.h"
#include "Fm246Voice.h"
#include "Digital246Voice.h"
#include "DrWave246Voice.h"
#include "Cluster246Voice.h"
#include "Punch246Effect.h"
#include "Filter246Controls.h"
#include "Nitro246Controls.h"
#include "Delay246Controls.h"
#include "Grid246Controls.h"
#include "Phone246Controls.h"
#include "Spring246Controls.h"
#include "Cwo246Controls.h"
#include "Value246Fx.h"
#include "ValueVoice246Control.h"
#include "Random246Control.h"
#include "Random246FxControl.h"
#include "String246Voice.h"
#include "Dna246Voice.h"
#include "Voltage246Voice.h"
#include "Dsynth246Voice.h"
#include "Dbox246Voice.h"
#include "Drum246Voice.h"
#include "Sampler246Voice.h"
#include "HybridPerformance.h"

namespace pocket {
constexpr double pi = 3.14159265358979323846;
inline constexpr bool isHybridEngine(int mode) noexcept {return mode==2||mode==6||mode==7||mode==8||mode==9||mode==10;}
inline constexpr bool isDrumEngine(int mode) noexcept {return mode==4||mode==5||mode==11;}
struct Parameters {
    int engine=0;bool drum246=false;std::array<int16_t,4> drum246Dynamics=drum246EnvelopeDefaults;
    std::array<std::array<int16_t,8>,dbox246Pads> dbox246Controls=dbox246Defaults();
    std::array<int16_t,4> fm246Controls=fm246Defaults,fm246Adsr{{64,8064,32767,8192}};
    std::array<int16_t,4> string246Controls=string246Defaults,string246Adsr{{64,8064,32767,8192}};
    std::array<int16_t,4> dna246Controls=dna246Defaults,dna246Adsr{{64,8064,32767,8192}};
    std::array<int16_t,4> voltage246Controls=voltage246Defaults,voltage246Adsr{{64,8064,32767,8192}};
    std::array<int16_t,8> dsynth246Controls=dsynth246Defaults;std::array<int16_t,4> dsynth246Adsr{{64,8064,32767,8192}};bool dsynth246Secondary=false;
    bool element246=false;int element246Source=0,element246Amount=16000,element246Destination=0,element246Colour=0,element246Sensor=0;
    bool crank246=false;int crank246Angle=0,crank246Amount=16000,crank246Destination=0,crank246Colour=0;
    bool bend246=false,bend246Reverse=false;int bend246Colour=0,bend246Destination=0,bend246Amount=16000,bend246Angle=8192;
    bool midi246=false;std::array<int,4> midi246Routes{{1,2,3,4}};
    int hybridVoiceMode=0;float hybridGlideMs=0;
    bool random246=false;std::array<int16_t,4> random246Controls{{2000,4000,0,24000}};
    bool tremolo246=false;int tremolo246Shape=0;std::array<int16_t,4> tremolo246Controls{{16000,0,0,16000}};
    bool phase246=false,phase246Envelope=false;std::array<int16_t,4> phase246Controls{{0,29491,0,0}},phase246Adsr{{64,8064,32767,8192}};
    bool digital246=false,digital246Envelope=false;std::array<int16_t,4> digital246Controls{{0,16384,0,0}},digital246Adsr{{64,8064,32767,8192}};
    std::array<int16_t,4> drWave246Controls{{0,0,0,0}},drWave246Adsr{{64,8064,32767,8192}};
    std::array<int16_t,4> cluster246Controls{{8192,0,12000,30}},cluster246Adsr{{64,8064,32767,8192}};
    bool cwo246=false;std::array<int16_t,4> cwo246Controls=cwo246Defaults;
    bool spring246=false;std::array<int16_t,4> spring246Controls=spring246Defaults;
    bool phone246=false;std::array<int16_t,4> phone246Controls{{12000,8192,6144,20000}};
    bool grid246=false;std::array<int16_t,4> grid246Controls{{6144,8192,27000,16000}};
    bool delay246=false;std::array<int16_t,4> delay246Controls{{6144,24000,9000,16000}};
    bool nitro246=false;std::array<int16_t,4> nitro246Controls{{512,0,8000,14000}};
    bool filter246=false;std::array<int16_t,4> filter246Controls{{8000,0,0,24576}};
    bool punch246=false;std::array<int16_t,4> punch246Controls{{6000,15000,20000,28000}};
    bool value246=false,value246Free=false,value246Destination=true;
    int value246Speed=1000,value246Amount=4000,value246Colour=0;uint32_t value246Tempo=199457888;
    bool valueVoice246=false,valueVoice246Envelope=false,valueVoice246Free=false;
    int valueVoice246Speed=1000,valueVoice246Amount=4000,valueVoice246Colour=0;
    std::array<PadSettings,kitCapacity> pads{};
    float tone=.45f,color=.35f,envelope=.35f,effect=.2f,volume=.7f;
    float attack=.02f,decay=.2f,sustain=.8f,release=.3f;
    float lfoRate=1,lfoDepth=0; int lfoDestination=0,lfoShape=0;
    float delayTime=.281f,feedback=.42f,drive=0;
    bool drum246LfoBank=false;bool sampler246=false,sampler246Envelope=true;bool sampler246Secondary=false;std::array<int16_t,8> sampler246Controls=sampler246Defaults;std::array<int16_t,4> sampler246Adsr=sampler246EnvelopeDefaults;bool sampleClean=false;bool sampleLoop=false; float sampleRoot=60;
};
class Engine {
public:
    static constexpr int polyphony=8;
    void prepare(double rate) { sampleRate=std::max(8000.0,rate); smoothing=float(1-std::exp(-1.0/(sampleRate*.012))); cachedCutoff=-1;phaseConfig.prepare(sampleRate);drWaveConfig.prepare(phaseConfig);punchEffect.prepare(sampleRate);valueEffect.prepare(sampleRate);elementVoiceInput.prepare(sampleRate);reset(); }
    const std::array<uint32_t,4>& midiControllers() const noexcept {return midiCC;}
    void midiController(int number,int value) noexcept {if(number>=1&&number<=4)midiCC[size_t(number-1)]=uint32_t(std::clamp(value,0,127));}
    float element246Level() const noexcept {return elementControl.level();}
    float crank246Phase() const noexcept {return crankControl.phase();}
    Element246FxAudio elementFxAudio() const noexcept {
        if(elementControl.active&&elementControl.destination==3&&(elementControl.colour==2||elementControl.colour==3))return {elementControl.inputs,elementControl.source,int16_t(elementControl.amount),elementControl.colour};
        return {};
    }
    Manual246FxAudio manualFxAudio() const noexcept {
        if(crankControl.active&&crankControl.destination==3&&crankControl.colour>=2&&crankControl.colour<=3)return {reference::saturate16((reference::crank246Level(crankControl.state)-64)*512),crankControl.controls[1],crankControl.colour};
        if(bendControl.active&&bendControl.destination==3&&bendControl.colour>=2&&bendControl.colour<=3)return {reference::saturate16(int(int16_t(bendControl.state.words[0]))*256),bendControl.controls[2],bendControl.colour};
        return {};
    }
    float bend246Level() const noexcept {return bendControl.level();}
    void reset() {elementVoiceInput.reset();elementControl={};elementFeedback=0;crankControl={};bendControl={};midiCC.fill(64);drumBank.reset();selectedDrum=false;selectedSampler=false;dboxBank.reset();dsynthContext={};dsynthContext.initialize();dsynthControlsInitialized=nativeDsynthActive=false;voltageControlsInitialized=false;nativeVoltageActive=false;dnaContext.prepare();dnaControlsInitialized=false;nativeDnaActive=false;selectionInitialized=false;stringControlsInitialized=false;nativeStringActive=false;fmControlsInitialized=false;nativeFmActive=false;held.clear(); voices={};randomRng=0x52414e44u; tremoloActive=false;delayValid=0; write=0; filter=0; current={}; lfoPhase=0; bends.fill(1); pedals.fill(false); cachedAttack=cachedDecay=cachedRelease=cachedMacro=-1;cleanMix=0;modulatedFilterMix=0;cleanInitialized=false;phaseControlsInitialized=false;digitalControlsInitialized=false;nativeEnvelopeActive=false;nativeSamplerEnvelopeActive=false;nativeDigitalActive=false;nativeDrWaveActive=false;nativeClusterActive=false;drWaveControlsInitialized=false;clusterControlsInitialized=false;filterControlsInitialized=false;punchEffect.reset();valueEffect.reset();randomFx.reset();valueActive=valueVoiceActive=false;punchMix=0;punchActive=false; }
    void selectEngine(int mode,bool digital=false,bool drum=false,bool sampler=false) noexcept {
        sampler=mode==3&&sampler;
        if(selectionInitialized&&sampler!=selectedSampler){voices={};held.clear();}
        selectedSampler=sampler;
        drum=mode==5&&drum;
        if(selectionInitialized&&(selectedDrum||drum)&&(mode!=selectedEngine||selectedDrum!=drum)){voices={};drumBank.reset();held.clear();}
        selectedDrum=drum;
        if(selectionInitialized&&mode!=selectedEngine&&(mode==11||selectedEngine==11)){voices={};dboxBank.reset();held.clear();}
        if(selectionInitialized&&(mode!=selectedEngine||(mode==0&&digital!=selectedDigital))&&(hybridPerformance()||isHybridEngine(mode)||mode==0)){voices={};held.clear();}
        if(mode==10&&(!selectionInitialized||selectedEngine!=10)){dsynthContext={};dsynthContext.initialize();}
        if(mode==8&&(!selectionInitialized||selectedEngine!=8))dnaContext.prepare();
        selectedEngine=mode;selectedDigital=digital;selectionInitialized=true;
    }
    void configureHybridPerformance(int mode,float milliseconds) noexcept {
        mode=std::clamp(mode,0,2);
        if(mode!=hybridMode){if(hybridPerformance()){voices={};held.clear();}hybridMode=mode;}
        glideMs=std::isfinite(milliseconds)?std::clamp(milliseconds,0.f,2000.f):0.f;
    }
    void setSample(Sample data) {if(data.drumPcm.empty()&&!data.frames.empty())data.prepareDrumPcm(); sample=std::move(data); voices={}; }
    void swapSample(Sample& data) {std::swap(sample,data);voices={};}
    bool isSharedKit() const noexcept {return sharedKit;}
    void swapKit(Kit& data,int count=8,bool shared=false) {sharedKit=shared;drumBank.reset();kit.swap(data);kitCount=sharedKit||count==24?24:8;voices={};}
    void swapKitPad(int pad,Sample& data){drumBank.reset();if(pad>=0&&pad<kitCapacity){std::swap(kit[size_t(pad)],data);if(pad>=kitCount)kitCount=24;voices={};}}
    int getKitCount() const{return kitCount;}
    const Sample& getSample() const { return sample; }
    void setKit(Kit data,int count=8,bool shared=false) {sharedKit=shared;for(auto& s:data)if(s.drumPcm.empty()&&!s.frames.empty())s.prepareDrumPcm();drumBank.reset();kit=std::move(data);kitCount=sharedKit||count==24?24:8;voices={};}
    const Kit& getKit() const { return kit; }
    static int kitSlot(int note,int count=8) {
        if(note>=60) return (note-60)%(count==24?24:8);
        switch(note) {case 35:case 36:return 0;case 38:case 40:return 1;case 42:case 44:return 2;case 46:return 3;case 41:case 43:case 45:case 47:case 48:case 50:return 4;case 39:return 5;case 49:case 57:return 6;case 37:case 51:return 7;default:return note%8;}
    }
    void pitchBend(int channel,float semitones) { bends[channelIndex(channel)]=std::exp2(std::clamp(semitones,-2.f,2.f)/12.f); }
    void sustainPedal(int channel,bool down) {
        if(channel<1||channel>17)return;
        if(selectedDrum){drumBank.pedal(channel,down);return;}
        if(selectedEngine==11){dboxBank.pedal(channel,down);return;}
        pedals[channelIndex(channel)]=down;
        if(monophonic()&&!down){held.releasePedal(channel);updateHeldVoice();return;}
        if(!down) for(auto& v:voices) if(v.channel==channel&&!v.keyDown) v.released=true;
    }
    void noteOn(int note,float velocity,int channel=1) {
        if(note<0||note>127||channel<1||channel>17)return;
        if(velocity<=0){noteOff(note,channel);return;}
        if(selectedDrum){valueEffect.noteOn();randomFx.noteOn();drumBank.noteOn(note,velocity,channel,kitSlot(note,kitCount),sampleRate);return;}
        if(selectedEngine==11){valueEffect.noteOn();randomFx.noteOn();dboxBank.noteOn(note,velocity,channel,sampleRate);return;}
        valueEffect.noteOn();randomFx.noteOn();
        if(monophonic()){
            const bool overlap=held.latest()>=0;
            held.press(note,channel,std::clamp(velocity,0.f,1.f));
            playHeldVoice(overlap);return;
        }
        Voice* v=nullptr;
        for(size_t i=0;i<((hybridPerformance()||selectedSampler)?6u:voices.size());++i) if(auto& candidate=voices[i];!candidate.active) { v=&candidate; break; }
        if(!v) v=&*std::min_element(voices.begin(),voices.begin()+((hybridPerformance()||selectedSampler)?6:polyphony),[](auto& a,auto& b){return a.age<b.age;});
        const auto stolen=(hybridPerformance()||selectedSampler)&&v->active?v->lastOutput:0.f;
        const auto randomState=v->randomLfo;const auto valueState=v->valueVoice;*v={};v->randomLfo=randomState;v->randomLfo.noteOn();v->stealTail=stolen;v->stealRemaining=stolen!=0?unsigned(sampleRate*.003):0;v->valueVoice=valueState;v->valueVoice.noteOn();v->active=true; v->keyDown=true; v->note=note; v->channel=channel;
        v->velocity=std::clamp(velocity,0.f,1.f); v->age=++age;v->cluster246.context.random=uint32_t(v->age)*2654435761u;
        v->frequency=440.0*std::pow(2.0,(note-69)/12.0);

    }
    void noteOff(int note,int channel=1) {
        if(note<0||note>127||channel<1||channel>17)return;
        if(selectedDrum){drumBank.noteOff(note,channel);return;}
        if(selectedEngine==11){dboxBank.noteOff(note,channel);return;}
        if(monophonic()){held.release(note,channel,pedals[channelIndex(channel)]);updateHeldVoice();return;}
        for(auto& v:voices) if(v.active&&v.note==note&&v.channel==channel) { v.keyDown=false; if(!pedals[channelIndex(channel)]) v.released=true; }
    }
    void allNotesOff(int channel=0) {
        if(channel<0||channel>17)return;
        if(selectedDrum){drumBank.allNotesOff(channel);return;}
        if(selectedEngine==11){dboxBank.allNotesOff(channel);return;}
        if(monophonic()){
            if(channel==0){held.clear();voices[0].keyDown=false;voices[0].released=true;pedals.fill(false);}
            else {held.removeChannel(channel);pedals[channelIndex(channel)]=false;updateHeldVoice();}
            return;
        }
        for(auto& v:voices) if(channel==0||v.channel==channel) { v.keyDown=false; v.released=true; }
        if(channel==0) pedals.fill(false); else pedals[channelIndex(channel)]=false;
    }
    struct VisualState {std::array<float,4> random{};float envelope=0,progress=0,lfo=0;int active=0,stage=0,note=-1,channel=0,fmAlgorithm=0;double frequency=0;uint64_t trigger=0;};
    VisualState visualState(int mode) const noexcept {
        VisualState out;if(randomFx.active)for(size_t i=0;i<4;++i)out.random[i]=randomFx.modulation[i]/32768.f;if(mode==5&&selectedDrum){out.trigger=drumBank.age;uint64_t latest=0;for(const auto& h:drumBank.hits)if(h.active){++out.active;out.envelope=std::max(out.envelope,h.voice.envelope);if(h.age>=latest){latest=h.age;out.note=h.note;out.channel=h.channel;out.stage=h.released?4:2;out.progress=h.voice.progress;out.lfo=h.modulation.mode==1?h.modulation.phase():h.voice.output.tremolo.phase();for(size_t i=0;i<4;++i)if(!randomFx.active)out.random[i]=h.modulation.mode==2&&h.modulation.applied?h.modulation.signals[i]/32768.f:0;}}return out;}if(mode==11){out.trigger=dboxBank.age;uint64_t latest=0;for(const auto& h:dboxBank.hits)if(h.active){++out.active;out.envelope=std::max(out.envelope,h.voice.level());if(h.age>=latest){latest=h.age;out.note=h.note;out.channel=h.channel;out.stage=h.released?4:2;out.lfo=h.modulation.mode==1?h.modulation.phase():h.voice.output.tremolo.phase();for(size_t i=0;i<4;++i)if(!randomFx.active)out.random[i]=h.modulation.mode==2&&h.modulation.applied?h.modulation.signals[i]/32768.f:0;}}return out;}out.trigger=age;out.lfo=valueActive?valueEffect.phase():float(lfoPhase);uint64_t latest=0;
        for(const auto& v:voices) if(v.active) {
            ++out.active;out.envelope=std::max(out.envelope,v.level);
            if(v.age>=latest) {latest=v.age;out.fmAlgorithm=v.fm246.algorithm;out.note=v.note;out.channel=v.channel;out.frequency=(monophonic()?v.glide.value:v.frequency)*bends[channelIndex(v.channel)];out.stage=nativeEnvelopeActive?(nativeSamplerEnvelopeActive?v.sampler246.output.envelopeStage():nativeDsynthActive?v.dsynth246.output.envelopeStage():nativeVoltageActive?v.voltage246.output.envelopeStage():nativeDnaActive?v.dna246.output.envelopeStage():nativeStringActive?v.string246.output.envelopeStage():nativeFmActive?v.fm246.output.envelopeStage():nativeClusterActive?v.cluster246.output.envelopeStage():nativeDrWaveActive?v.drWave246.output.envelopeStage():nativeDigitalActive?v.digital246.output.envelopeStage():v.phase246.envelopeStage()):v.released?4:v.stage==0?1:std::abs(v.level-currentSustain)>.005f?2:3;
                for(size_t i=0;i<4;++i)if(!randomFx.active)out.random[i]=v.randomLfo.active?v.randomLfo.modulation[i]/32768.f:0;
                if(valueVoiceActive)out.lfo=v.valueVoice.phase();
                if(tremoloActive)out.lfo=(selectedSampler?v.sampler246.output:nativeDsynthActive?v.dsynth246.output:nativeVoltageActive?v.voltage246.output:nativeDnaActive?v.dna246.output:nativeStringActive?v.string246.output:nativeFmActive?v.fm246.output:nativeClusterActive?v.cluster246.output:nativeDrWaveActive?v.drWave246.output:nativeDigitalActive?v.digital246.output:v.phase246).tremolo.phase();
                const size_t length=mode==5?kit[sharedKit?0:size_t(kitSlot(v.note,kitCount))].frames.size():sample.frames.size();
                if(mode==3&&selectedSampler){const auto frames=Sampler246Voice::frames(sample);out.progress=frames?std::clamp(float(v.sampler246.state.position/(4096.*frames)),0.f,1.f):0;}
                else if(length) out.progress=std::clamp(float(v.samplePosition/double(length))+(mode==3?current.tone*.9f:0.f),0.f,1.f);
            }
        }
        return out;
    }
    const std::array<int16_t,4>& filterControls() const noexcept {return filterModulated;}
    float next(const Parameters& target,float elementExternal=0) {
        selectEngine(target.engine,target.digital246,target.drum246,target.sampler246);
        configureHybridPerformance(target.hybridVoiceMode,target.hybridGlideMs);
        const bool nativeDrum=target.engine==5&&target.drum246;
        const bool nativeDbox=target.engine==11;
        const bool nativeFm=target.engine==0&&!target.digital246;
        const bool nativeString=target.engine==2;
        const bool nativeDsynth=target.engine==10;
        const bool nativeVoltage=target.engine==9;
        const bool nativeDna=target.engine==8;
        const bool nativeCluster=target.engine==7;
        const bool nativeDrWave=target.engine==6;
        const bool nativePhase=target.engine==1&&target.phase246;
        const bool nativeDigital=target.engine==0&&target.digital246;
        const bool nativeSampler=target.engine==3&&target.sampler246;
        const bool nativeCore=nativeSampler||nativeDrum||nativeDbox||nativeDsynth||nativeVoltage||nativeDna||nativeString||nativeFm||nativePhase||nativeDigital||nativeDrWave||nativeCluster;
        const bool nativeElement=target.element246;
        elementVoiceInput.next(nativeElement,elementExternal);
        const double elementStep=44100./(64*sampleRate);
        const float elementEnvelope=nativeElement&&elementControl.willTick(elementStep)?visualState(target.engine).envelope:0;
        elementControl.next(nativeElement,target.element246Source,target.element246Amount,target.element246Destination,target.element246Colour,target.element246Sensor,elementExternal,elementEnvelope,elementFeedback,elementStep,nativeDrum||nativeDbox);
        const bool nativeCrank=target.crank246&&!nativeElement;
        crankControl.next(nativeCrank,target.crank246Angle,target.crank246Amount,target.crank246Destination,target.crank246Colour,44100./(64*sampleRate));
        const bool nativeBend=target.bend246&&!nativeCrank&&!nativeElement;
        bendControl.next(nativeBend,target.bend246Colour,target.bend246Destination,target.bend246Amount,target.bend246Angle,target.bend246Reverse,44100./(64*sampleRate));
        const bool nativeMidi=target.midi246&&!nativeBend&&!nativeCrank&&!nativeElement;
        const bool randomSelected=target.random246&&(nativeCore||target.random246Controls[2]==4096)&&!nativeMidi&&!nativeBend&&!nativeCrank&&!nativeElement;
        const bool nativeRandomFx=randomSelected&&target.random246Controls[2]==4096;
        const bool nativeRandom=randomSelected&&nativeCore&&!nativeRandomFx;
        const bool modEnvelope=nativeRandom?target.random246Controls[2]>=2048:target.valueVoice246Envelope;
        const bool nativeTremolo=target.tremolo246&&nativeCore&&!randomSelected&&!nativeMidi&&!nativeBend&&!nativeCrank&&!nativeElement;tremoloActive=nativeTremolo;
        const bool nativeValueVoice=target.valueVoice246&&nativeCore&&!nativeTremolo&&!randomSelected&&!nativeMidi&&!nativeBend&&!nativeCrank&&!nativeElement;
        valueVoiceActive=nativeValueVoice;nativeFmActive=nativeFm;
        nativeClusterActive=nativeCluster;nativeDrWaveActive=nativeDrWave;nativeDigitalActive=nativeDigital;nativeVoltageActive=nativeVoltage;nativeDnaActive=nativeDna;nativeStringActive=nativeString;nativeDsynthActive=nativeDsynth;nativeSamplerEnvelopeActive=nativeSampler&&target.sampler246Envelope;nativeEnvelopeActive=nativeSamplerEnvelopeActive||nativeDsynth||nativeVoltage||nativeDna||nativeString||nativeFm||nativeCluster||nativeDrWave||(nativePhase&&target.phase246Envelope)||(nativeDigital&&target.digital246Envelope);
        if(nativePhase){for(size_t i=0;i<4;++i){if(!phaseControlsInitialized)phaseWords[i]=target.phase246Controls[i];else phaseWords[i]+=smoothing*(target.phase246Controls[i]-phaseWords[i]);phaseRaw[i]=int16_t(std::clamp(phaseWords[i],0.f,32767.f)+.5f);}phaseControlsInitialized=true;}
        else phaseControlsInitialized=false;
        if(nativeDigital){for(size_t i=0;i<4;++i){if(!digitalControlsInitialized)digitalWords[i]=target.digital246Controls[i];else digitalWords[i]+=smoothing*(target.digital246Controls[i]-digitalWords[i]);digitalRaw[i]=int16_t(std::lround(std::clamp(digitalWords[i],i==2?-32768.f:0.f,32767.f)));}digitalControlsInitialized=true;}
        else digitalControlsInitialized=false;
        if(nativeDrWave){for(size_t i=0;i<4;++i){if(!drWaveControlsInitialized)drWaveWords[i]=target.drWave246Controls[i];else drWaveWords[i]+=smoothing*(target.drWave246Controls[i]-drWaveWords[i]);drWaveRaw[i]=int16_t(std::lround(std::clamp(drWaveWords[i],0.f,float(drWave246Max[i]))));}drWaveControlsInitialized=true;}else drWaveControlsInitialized=false;
        if(nativeCluster){for(size_t i=0;i<4;++i){if(!clusterControlsInitialized)clusterWords[i]=target.cluster246Controls[i];else clusterWords[i]+=smoothing*(target.cluster246Controls[i]-clusterWords[i]);clusterRaw[i]=int16_t(std::lround(std::clamp(clusterWords[i],float(reference::cluster246ControlMin[i]),float(reference::cluster246ControlMax[i]))));}clusterControlsInitialized=true;}else clusterControlsInitialized=false;
        if(nativeFm){for(size_t i=0;i<4;++i){if(!fmControlsInitialized)fmWords[i]=target.fm246Controls[i];else fmWords[i]+=smoothing*(target.fm246Controls[i]-fmWords[i]);fmRaw[i]=int16_t(std::lround(std::clamp(fmWords[i],float(fm246Min[i]),float(fm246Max[i]))));}fmControlsInitialized=true;}else fmControlsInitialized=false;
        if(nativeString){for(size_t i=0;i<4;++i){if(!stringControlsInitialized)stringWords[i]=target.string246Controls[i];else stringWords[i]+=smoothing*(target.string246Controls[i]-stringWords[i]);stringRaw[i]=int16_t(std::lround(std::clamp(stringWords[i],float(string246Min[i]),float(string246Max[i]))));}stringControlsInitialized=true;}else stringControlsInitialized=false;
        if(nativeDsynth){for(size_t i=0;i<8;++i){if(!dsynthControlsInitialized)dsynthWords[i]=target.dsynth246Controls[i];else dsynthWords[i]+=smoothing*(target.dsynth246Controls[i]-dsynthWords[i]);dsynthRaw[i]=int16_t(std::lround(std::clamp(dsynthWords[i],0.f,32767.f)));}dsynthControlsInitialized=true;}else dsynthControlsInitialized=false;
        if(nativeVoltage){for(size_t i=0;i<4;++i){if(!voltageControlsInitialized)voltageWords[i]=target.voltage246Controls[i];else voltageWords[i]+=smoothing*(target.voltage246Controls[i]-voltageWords[i]);voltageRaw[i]=int16_t(std::lround(std::clamp(voltageWords[i],float(voltage246Min[i]),float(voltage246Max[i]))));}voltageControlsInitialized=true;}else voltageControlsInitialized=false;
        if(nativeDna){for(size_t i=0;i<4;++i){if(!dnaControlsInitialized)dnaWords[i]=target.dna246Controls[i];else dnaWords[i]+=smoothing*(target.dna246Controls[i]-dnaWords[i]);dnaRaw[i]=int16_t(std::lround(std::clamp(dnaWords[i],float(dna246Min[i]),float(dna246Max[i]))));}dnaControlsInitialized=true;}else dnaControlsInitialized=false;
        const int midiType=nativeSampler?6:nativeDsynth?12:nativeVoltage?13:nativeDna?11:nativeString?3:nativeFm?0:nativeCluster?4:nativeDrWave?2:nativeDigital?1:7;
        auto midiSampler=target.sampler246Controls;
        const auto& adsrBase=nativeSampler?target.sampler246Adsr:nativeDsynth?target.dsynth246Adsr:nativeVoltage?target.voltage246Adsr:nativeDna?target.dna246Adsr:nativeString?target.string246Adsr:nativeFm?target.fm246Adsr:nativeCluster?target.cluster246Adsr:nativeDrWave?target.drWave246Adsr:nativeDigital?target.digital246Adsr:target.phase246Adsr;
        const auto midiAdsr=nativeElement&&nativeCore?elementControl.apply(adsrBase,1,midiType):nativeCrank&&nativeCore?crankControl.apply(adsrBase,1,midiType):nativeBend&&nativeCore?bendControl.apply(adsrBase,1,midiType):nativeMidi&&nativeCore?midi246Apply(adsrBase,1,midiType,target.midi246Routes,midiCC):adsrBase;
        if(nativeMidi||nativeBend||nativeCrank||nativeElement){auto apply=[&](auto& raw){if(nativeElement){raw=elementControl.apply(raw,0,midiType);return;}if(nativeCrank){raw=crankControl.apply(raw,0,midiType);return;}if(nativeBend){raw=bendControl.apply(raw,0,midiType);return;}raw=midi246Apply(raw,0,midiType,target.midi246Routes,midiCC);};
         if(nativeSampler)apply(midiSampler);else if(nativeDsynth)apply(dsynthRaw);else if(nativeVoltage)apply(voltageRaw);else if(nativeDna)apply(dnaRaw);else if(nativeString)apply(stringRaw);else if(nativeFm)apply(fmRaw);else if(nativeCluster)apply(clusterRaw);else if(nativeDrWave)apply(drWaveRaw);else if(nativeDigital)apply(digitalRaw);else if(nativePhase)apply(phaseRaw);
        }
        const reference::PdCoefficients phaseCoefficients(phaseRaw);
        const reference::Digital246Coefficients digitalCoefficients(digitalRaw);
        if(!cleanInitialized){const bool clean=(target.sampleClean&&(target.engine==3||target.engine==5))||nativeCore;cleanMix=clean?1.f:0.f;if(clean)current=target;cleanInitialized=true;}
        auto follow=[this,&target](float& v,float t){v+=smoothing*(t-v);if(target.sampleClean&&std::abs(v-t)<.00002f)v=t;};
        follow(current.tone,target.tone); follow(current.color,target.color); follow(current.envelope,nativeCore?.5f:target.envelope);
        follow(current.effect,target.effect); follow(current.volume,target.volume); follow(current.drive,target.drive);
        follow(current.delayTime,target.delayTime); follow(current.feedback,target.feedback);
        const auto& p=current;currentSustain=target.sustain;
        lfoPhase+=std::clamp(double(target.lfoRate),.05,20.0)/sampleRate; lfoPhase-=std::floor(lfoPhase);
        const double lfo=(target.value246||nativeValueVoice||nativeTremolo||randomSelected||nativeMidi||nativeBend||nativeCrank||nativeElement?0:target.lfoDepth)*(target.lfoShape==1 ? 1-4*std::abs(lfoPhase-.5) : target.lfoShape==2 ? (lfoPhase<.5?1:-1) : std::sin(2*pi*lfoPhase));
        const double pitchMod=target.lfoDestination==0?lfo*2:0;
        const double pitchFactor=pitchMod==0?1:std::exp2(pitchMod/12);
        const double kitPitch=target.engine==5?std::exp2((p.tone-.5)*2):1;
        if(cachedAttack!=target.attack||cachedMacro!=p.envelope) attackStep=float(1/(sampleRate*(.001+target.attack*target.attack*4*(.5+p.envelope))));
        if(cachedDecay!=target.decay) decayFactor=float(std::exp(-1/(sampleRate*(.005+target.decay*target.decay*3))));
        if(cachedRelease!=target.release||cachedMacro!=p.envelope) releaseFactor=float(std::exp(-1/(sampleRate*(.008+target.release*target.release*5*(.5+p.envelope)))));
        cachedAttack=target.attack;cachedDecay=target.decay;cachedRelease=target.release;cachedMacro=p.envelope;
        if(cachedRoot!=target.sampleRoot) {cachedRoot=target.sampleRoot;rootFrequency=440*std::exp2((cachedRoot-69)/12);}

        if(target.engine==5)for(size_t i=0;i<padPitch.size();++i)if(target.pads[i].pitch!=cachedPadPitch[i]){cachedPadPitch[i]=target.pads[i].pitch;padPitch[i]=std::exp2(std::clamp(double(target.pads[i].pitch),-24.,24.)/12);}
        Drum246ModulationConfig drumModulation;drumModulation.element=nativeElement?&elementControl:nullptr;drumModulation.crank=nativeCrank?&crankControl:nullptr;drumModulation.bend=nativeBend?&bendControl:nullptr;drumModulation.midi=nativeMidi;drumModulation.midiRoutes=target.midi246Routes;drumModulation.midiCC=midiCC;drumModulation.value=nativeValueVoice;drumModulation.random=nativeRandom;drumModulation.free=target.valueVoice246Free;drumModulation.secondary=target.drum246LfoBank;drumModulation.speed=target.valueVoice246Speed;drumModulation.amount=target.valueVoice246Amount;drumModulation.colour=target.valueVoice246Colour;drumModulation.envelope=target.valueVoice246Envelope;drumModulation.randomControls=target.random246Controls;drumModulation.tempo=target.value246Tempo;drumModulation.step=44100./(64*sampleRate);drumModulation.rng=&randomRng;
        float sum=nativeDbox?dboxBank.next(target.dbox246Controls,bends,smoothing,phaseConfig,nativeTremolo,target.tremolo246Controls,target.tremolo246Shape,target.value246Tempo,drumModulation):nativeDrum?drumBank.next(kit,target.pads,target.drum246Dynamics,bends,kitPitch*pitchFactor,phaseConfig,nativeTremolo,target.tremolo246Controls,target.tremolo246Shape,target.value246Tempo,drumModulation,sharedKit):0;
        for(auto& v:voices) {
            if(!v.active) continue;
            if(nativeEnvelopeActive){}
            else if(v.released) v.level*=releaseFactor;
            else if(v.stage==0) {v.level=std::min(1.f,v.level+attackStep);if(v.level>=1) v.stage=1;}
            else v.level=target.sustain+(v.level-target.sustain)*decayFactor;
            if(!nativeEnvelopeActive&&target.engine!=4&&target.engine!=5&&v.released&&v.level<.0001f) { v.active=false; continue; }
            const double bend=bends[channelIndex(v.channel)]*pitchFactor;
            const double frequency=(monophonic()?v.glide.next():v.frequency)*bend;
            v.phase+=frequency/sampleRate; v.phase-=std::floor(v.phase);
            v.modPhase+=frequency*(1+std::round(p.color*7))/sampleRate; v.modPhase-=std::floor(v.modPhase);
            float s=0, amplitude=v.level;
            std::array<int16_t,4> modulated{};
            auto dsynthModulated=dsynthRaw;auto samplerModulated=midiSampler;
            if(nativeSampler&&!modEnvelope&&(nativeValueVoice||nativeRandom)){
                if(nativeRandom)samplerModulated=v.randomLfo.next(true,target.random246Controls,6,44100./(64*sampleRate),randomRng,target.sampler246Controls);
                else samplerModulated=v.valueVoice.next(true,false,target.valueVoice246Free,target.valueVoice246Speed,target.valueVoice246Amount,target.valueVoice246Colour+(target.sampler246Secondary?4:0),6,target.value246Tempo,44100./(64*sampleRate),target.sampler246Controls,phaseConfig.table);
            }else if(nativeDsynth&&!modEnvelope&&(nativeValueVoice||nativeRandom)){
                if(nativeRandom)dsynthModulated=v.randomLfo.next(true,target.random246Controls,12,44100./(64*sampleRate),randomRng,dsynthRaw);
                else dsynthModulated=v.valueVoice.next(true,false,target.valueVoice246Free,target.valueVoice246Speed,target.valueVoice246Amount,target.valueVoice246Colour+(target.dsynth246Secondary?4:0),12,target.value246Tempo,44100./(64*sampleRate),dsynthRaw,phaseConfig.table);
            }else if(nativeValueVoice||nativeRandom){
                const auto& base=modEnvelope?(nativeSampler?target.sampler246Adsr:nativeDsynth?target.dsynth246Adsr:nativeVoltage?target.voltage246Adsr:nativeDna?target.dna246Adsr:nativeString?target.string246Adsr:nativeFm?target.fm246Adsr:nativeCluster?target.cluster246Adsr:nativeDrWave?target.drWave246Adsr:nativeDigital?target.digital246Adsr:target.phase246Adsr):(nativeVoltage?voltageRaw:nativeDna?dnaRaw:nativeString?stringRaw:nativeFm?fmRaw:nativeCluster?clusterRaw:nativeDrWave?drWaveRaw:nativeDigital?digitalRaw:phaseRaw);
                if(nativeRandom)modulated=v.randomLfo.next(true,target.random246Controls,nativeSampler?6:nativeDsynth?12:nativeVoltage?13:nativeDna?11:nativeString?3:nativeFm?0:nativeCluster?4:nativeDrWave?2:nativeDigital?1:7,44100./(64*sampleRate),randomRng,base);
                else modulated=v.valueVoice.next(true,target.valueVoice246Envelope,target.valueVoice246Free,target.valueVoice246Speed,target.valueVoice246Amount,target.valueVoice246Colour,nativeSampler?6:nativeDsynth?12:nativeVoltage?13:nativeDna?11:nativeString?3:nativeFm?0:nativeCluster?4:nativeDrWave?2:nativeDigital?1:7,target.value246Tempo,44100./(64*sampleRate),base,phaseConfig.table);
            }
            if(!nativeValueVoice&&v.valueVoice.active)v.valueVoice.reset();
            if(!nativeRandom&&v.randomLfo.active)v.randomLfo.reset();
            auto& nativeOutput=nativeSampler?v.sampler246.output:nativeDsynth?v.dsynth246.output:nativeVoltage?v.voltage246.output:nativeDna?v.dna246.output:nativeString?v.string246.output:nativeFm?v.fm246.output:nativeCluster?v.cluster246.output:nativeDrWave?v.drWave246.output:nativeDigital?v.digital246.output:v.phase246;
            nativeOutput.tremolo.configure(nativeTremolo,target.tremolo246Controls,target.tremolo246Shape,target.value246Tempo,nativeSampler||nativeDsynth||nativeVoltage||nativeDna||nativeString||nativeFm||nativeCluster||nativeDrWave);
            nativeOutput.midi.configure(nativeMidi&&nativeCore&&!nativeDrum&&!nativeDbox,target.midi246Routes,midiCC,nativeSampler||nativeDsynth||nativeVoltage||nativeDna||nativeString||nativeFm||nativeCluster||nativeDrWave);
            if(nativeElement&&target.element246Destination==3&&target.element246Colour==0){const int source=target.element246Source;nativeOutput.bend.configure(nativeCore&&!nativeDrum&&!nativeDbox,0,reference::element246GainScalar(elementControl.inputs,source,int16_t(target.element246Amount)),0,nativeSampler||nativeDsynth||nativeVoltage||nativeDna||nativeString||nativeFm||nativeCluster||nativeDrWave,3,source==1?3:source==2?2:1,&elementVoiceInput.words,elementVoiceInput.head);}
            else if(nativeElement&&target.element246Destination==3&&target.element246Colour==1){
                const int source=target.element246Source;const auto amount=int16_t(target.element246Amount);
                const bool constant=(nativePhase&&!target.phase246Envelope)||(nativeDigital&&!target.digital246Envelope)||(nativeSampler&&!target.sampler246Envelope);
                const auto held=constant?int16_t(16384):reference::saturate16(int(std::lround(nativeOutput.envelopeLevel()*16384.f)));
                const auto carrier=source==2?held:source==3?elementControl.inputs.level:reference::saturate16(-int(elementControl.inputs.sensor));
                nativeOutput.bend.configure(nativeCore&&!nativeDrum&&!nativeDbox,1,amount,reference::multiplyQ15(carrier,amount),nativeSampler||nativeDsynth||nativeVoltage||nativeDna||nativeString||nativeFm||nativeCluster||nativeDrWave,4,source==1?6:source==2?5:4,&elementVoiceInput.words,elementVoiceInput.head);
            }
            else nativeOutput.bend.configure(nativeCore&&!nativeDrum&&!nativeDbox&&((nativeCrank&&target.crank246Destination==3)||(nativeBend&&target.bend246Destination==3)),nativeCrank?target.crank246Colour:target.bend246Colour,nativeCrank?int16_t(crankControl.gainState&65535u):int16_t(bendControl.state.display&65535u),nativeCrank?reference::crank246PitchCoefficient(reference::crank246Level(crankControl.state),int16_t(target.crank246Amount)):reference::bend246PitchCoefficient(int(int16_t(bendControl.state.words[0])),int16_t(target.bend246Amount)),nativeSampler||nativeDsynth||nativeVoltage||nativeDna||nativeString||nativeFm||nativeCluster||nativeDrWave,nativeCrank?2:1);
            if(nativeDsynth){
                s=v.dsynth246.next(frequency,dsynthModulated,(nativeValueVoice||nativeRandom)&&modEnvelope?modulated:midiAdsr,dsynthContext,int(&v-voices.data()),phaseConfig,v.released);
                amplitude=1;v.level=v.dsynth246.output.envelopeLevel();if(v.dsynth246.done())v.active=false;
            }else if(nativeVoltage){
                s=v.voltage246.next(frequency,(nativeValueVoice||nativeRandom)&&!modEnvelope?modulated:voltageRaw,(nativeValueVoice||nativeRandom)&&modEnvelope?modulated:midiAdsr,phaseConfig,v.released);
                amplitude=1;v.level=v.voltage246.output.envelopeLevel();if(v.voltage246.output.envelopeDone())v.active=false;
            }
            else if(nativeDna){
                s=v.dna246.next(frequency,(nativeValueVoice||nativeRandom)&&!modEnvelope?modulated:dnaRaw,(nativeValueVoice||nativeRandom)&&modEnvelope?modulated:midiAdsr,dnaContext,size_t(&v-voices.data()),randomRng,phaseConfig,v.released);
                amplitude=1;v.level=v.dna246.output.envelopeLevel();if(v.dna246.output.envelopeDone())v.active=false;
            }
            else if(nativeString){
                s=v.string246.next(frequency,(nativeValueVoice||nativeRandom)&&!modEnvelope?modulated:stringRaw,(nativeValueVoice||nativeRandom)&&modEnvelope?modulated:midiAdsr,stringBuffers[size_t(&v-voices.data())],randomRng,phaseConfig,v.released);
                amplitude=1;v.level=v.string246.output.envelopeLevel();if(v.string246.output.envelopeDone())v.active=false;
            }
            else if(nativeFm){
                s=v.fm246.next(frequency,(nativeValueVoice||nativeRandom)&&!modEnvelope?modulated:fmRaw,(nativeValueVoice||nativeRandom)&&modEnvelope?modulated:midiAdsr,fmConfig,phaseConfig,v.released);
                amplitude=1;v.level=v.fm246.output.envelopeLevel();if(v.fm246.output.envelopeDone())v.active=false;
            }
            else if(nativeCluster){
                s=v.cluster246.next(frequency,(nativeValueVoice||nativeRandom)&&!modEnvelope?modulated:clusterRaw,(nativeValueVoice||nativeRandom)&&modEnvelope?modulated:midiAdsr,clusterConfig,phaseConfig,v.released);
                amplitude=1;v.level=v.cluster246.output.envelopeLevel();if(v.cluster246.output.envelopeDone())v.active=false;
            }
            else if(nativeDrWave){
                s=v.drWave246.next(frequency,(nativeValueVoice||nativeRandom)&&!modEnvelope?modulated:drWaveRaw,(nativeValueVoice||nativeRandom)&&modEnvelope?modulated:midiAdsr,drWaveConfig,phaseConfig,v.released);
                amplitude=1;v.level=v.drWave246.output.envelopeLevel();if(v.drWave246.output.envelopeDone())v.active=false;
            }
            else if(nativeDigital){
                s=v.digital246.next(frequency,(nativeValueVoice||nativeRandom)&&!modEnvelope?reference::Digital246Coefficients(modulated):digitalCoefficients,phaseConfig,nativeEnvelopeActive?((nativeValueVoice||nativeRandom)&&modEnvelope?&modulated:&midiAdsr):nullptr,v.released);
                if(nativeEnvelopeActive){amplitude=1;v.level=v.digital246.output.envelopeLevel();if(v.digital246.output.envelopeDone())v.active=false;}
            }
            else if(nativePhase){
                s=v.phase246.next(frequency,(nativeValueVoice||nativeRandom)&&!modEnvelope?reference::PdCoefficients(modulated):phaseCoefficients,phaseConfig,nativeEnvelopeActive?((nativeValueVoice||nativeRandom)&&modEnvelope?&modulated:&midiAdsr):nullptr,v.released);
                if(nativeEnvelopeActive){amplitude=1;v.level=v.phase246.envelopeLevel();if(v.phase246.envelopeDone())v.active=false;}
            }
            else if(target.engine==1) {
                const double bendPoint=.08+p.tone*.84;
                const double phase=v.phase<bendPoint?.5*v.phase/bendPoint:.5+.5*(v.phase-bendPoint)/(1-bendPoint);
                s=float(std::cos(2*pi*phase)*(1-p.color*.45)+std::sin(4*pi*v.phase)*p.color*.45);
            } else if(nativeSampler) {
                s=v.sampler246.next(frequency/rootFrequency,sample,samplerModulated,target.sampleLoop,phaseConfig,v.released,target.sampler246Envelope?((nativeValueVoice||nativeRandom)&&modEnvelope?&modulated:&midiAdsr):nullptr);
                if(target.sampler246Envelope){amplitude=1;v.level=v.sampler246.output.envelopeLevel();}
                if(v.sampler246.finished||(target.sampler246Envelope&&v.sampler246.output.envelopeDone()))v.active=false;
            } else if(target.engine==3&&!sample.frames.empty()) {
                const auto start=size_t(p.tone*.9*double(sample.frames.size()-1));
                const auto length=std::max(size_t(1),size_t((.01+p.color*.99)*double(sample.frames.size()-start)));
                if(v.samplePosition>=double(length)) {
                    if(target.sampleLoop&&!v.released) v.samplePosition=std::fmod(v.samplePosition,double(length));
                    else { v.active=false; continue; }
                }
                const double position=double(start)+v.samplePosition;
                const auto a=std::min(size_t(position),sample.frames.size()-1),b=std::min(a+1,sample.frames.size()-1);
                const float fraction=float(position-std::floor(position));
                s=sample.frames[a]+fraction*(sample.frames[b]-sample.frames[a]);
                v.samplePosition+=sample.rate/sampleRate*v.frequency/rootFrequency*bend;
            } else if(target.engine==5) {
                const auto slot=size_t(kitSlot(v.note,kitCount));const auto& hit=kit[sharedKit?0:slot];const auto& pad=target.pads[slot];
                if(hit.frames.empty()){v.active=false;continue;}
                const double start=std::clamp(double(pad.start),0.,1.)*double(hit.frames.size());
                const double end=std::clamp(double(pad.end),0.,1.)*double(hit.frames.size());
                const double length=std::max(0.,end-start)*std::clamp(double(p.color),.05,1.0);
                if(length<1||v.samplePosition>=length||(pad.gated&&v.released&&v.level<.0001f)){v.active=false;continue;}
                const double position=std::clamp(pad.reverse?start+length-1-v.samplePosition:start+v.samplePosition,0.,double(hit.frames.size()-1));
                const auto a=size_t(position),b=std::min(a+1,hit.frames.size()-1);
                const float fraction=float(position-double(a));
                s=(hit.frames[a]+fraction*(hit.frames[b]-hit.frames[a]))*pad.gain;
                v.samplePosition+=hit.rate/sampleRate*kitPitch*bend*padPitch[slot];
                amplitude=pad.gated?v.level:1; // One-shots retain legacy note-off behavior.
            } else if(target.engine==4) {
                const int kind=(v.note%12==0||v.note%12==7)?0:(v.note%12==2||v.note%12==4)?1:2;
                const double duration=kind==0?.08+p.envelope*.7:kind==1?.04+p.envelope*.4:.015+p.envelope*.16;
                amplitude=float(std::exp(-v.time/duration));
                const float n=noise(),high=n-v.lastNoise; v.lastNoise=n;
                if(kind==0) { v.drumPhase+=(35+110*p.tone+150*std::exp(-v.time*35))/sampleRate; s=float(std::sin(2*pi*v.drumPhase))*(1-p.color*.35f)+high*p.color*.15f; }
                else if(kind==1) s=float(std::sin(2*pi*v.time*(150+p.tone*180)))*.35f+n*(.35f+p.color*.5f);
                else s=high*(.3f+p.color*.5f);
                if(amplitude<.0001f) v.active=false;
            }
            v.time+=1/sampleRate;
            float contribution=s*amplitude*v.velocity*.16f;
            if((nativeDsynth||nativeVoltage||nativeDna||nativeString||nativeFm||nativeDrWave||nativeCluster)&&v.stealRemaining){const float weight=float(v.stealRemaining)/float(std::max(1u,unsigned(sampleRate*.003)));contribution=contribution*(1-weight)+v.stealTail*weight;--v.stealRemaining;}
            v.lastOutput=contribution;sum+=contribution;
        }
        elementFeedback=nativeElement?sum:0;
        const double cutoff=(900+p.tone*14500)*(target.lfoDestination==1&&lfo!=0?std::exp2(lfo*3):1);
        if(cutoff!=cachedCutoff) {cachedCutoff=cutoff;filterCoefficient=float(1-std::exp(-2*pi*std::min(cutoff,sampleRate*.42)/sampleRate));}
        const float coefficient=filterCoefficient;
        filter+=coefficient*(sum-filter);
        follow(cleanMix,((target.sampleClean&&(target.engine==3||target.engine==5))||nativeCore)?1.f:0.f);
        // CLEAN bypasses the implicit tone filter, but an explicitly selected
        // FILTER LFO must still be audible. Ramp its insertion to avoid a jump.
        follow(modulatedFilterMix,!target.value246&&!nativeValueVoice&&!nativeTremolo&&!randomSelected&&!nativeMidi&&!nativeBend&&!nativeCrank&&!nativeElement&&target.lfoDestination==1&&target.lfoDepth>0?1.f:0.f);
        const float cleanInput=sum+(filter-sum)*modulatedFilterMix;
        const float legacyDriven=std::tanh(filter*(1+p.drive*12))/(1+p.drive*2);
        const float cleanDriven=p.drive<.000001f?cleanInput:std::tanh(cleanInput*(1+p.drive*12))/(1+p.drive*2);
        float driven=legacyDriven+(cleanDriven-legacyDriven)*cleanMix;
        valueActive=target.value246&&!nativeValueVoice&&!nativeTremolo&&!randomSelected&&!nativeMidi&&!nativeBend&&!nativeCrank&&!nativeElement;
        const bool outputFx=target.filter246||target.nitro246||target.delay246||target.grid246||target.phone246||target.spring246||target.cwo246;const auto& outputControls=target.cwo246?target.cwo246Controls:target.spring246?target.spring246Controls:target.phone246?target.phone246Controls:target.grid246?target.grid246Controls:target.delay246?target.delay246Controls:target.nitro246?target.nitro246Controls:target.filter246Controls;const auto& outputMin=target.cwo246?cwo246Min:target.spring246?spring246Min:target.phone246?phone246Min:target.grid246?grid246Min:target.delay246?delay246Min:target.nitro246?nitro246Min:filter246Min;const auto& outputMax=target.cwo246?cwo246Max:target.spring246?spring246Max:target.phone246?phone246Max:target.grid246?grid246Max:target.delay246?delay246Max:target.nitro246?nitro246Max:filter246Max;
        const bool punchSelected=target.punch246&&!outputFx;
        if(punchSelected||punchActive){for(size_t i=0;i<4;++i){if(!punchActive)punchWords[i]=target.punch246Controls[i];else punchWords[i]+=smoothing*(target.punch246Controls[i]-punchWords[i]);punchRaw[i]=int16_t(std::lround(std::clamp(punchWords[i],0.f,32767.f)));}punchActive=true;follow(punchMix,punchSelected?1.f:0.f);}
        if(outputFx){for(size_t i=0;i<4;++i){if(!filterControlsInitialized)filterWords[i]=outputControls[i];else filterWords[i]+=smoothing*(outputControls[i]-filterWords[i]);filterRaw[i]=int16_t(std::lround(std::clamp(filterWords[i],float(outputMin[i]),float(outputMax[i]))));}filterControlsInitialized=true;}else filterControlsInitialized=false;
        auto modulated=valueEffect.next(valueActive,target.value246Destination&&(outputFx||punchSelected),target.value246Free,target.value246Speed,target.value246Amount,target.value246Colour,target.value246Tempo,outputFx?filterRaw:punchActive?punchRaw:target.punch246Controls,phaseConfig.table,target.cwo246?7:target.spring246?4:target.phone246?3:target.grid246?1:target.delay246?0:target.nitro246?6:target.filter246?5:2);
        modulated=randomFx.next(nativeRandomFx,target.random246Controls,(outputFx||punchSelected)?(target.cwo246?7:target.spring246?4:target.phone246?3:target.grid246?1:target.delay246?0:target.nitro246?6:target.filter246?5:2):-1,44100./(64*sampleRate),randomRng,modulated);
        if(nativeElement&&(outputFx||punchSelected))modulated=elementControl.apply(modulated,2,target.cwo246?7:target.spring246?4:target.phone246?3:target.grid246?1:target.delay246?0:target.nitro246?6:target.filter246?5:2);
        if(nativeCrank&&(outputFx||punchSelected))modulated=crankControl.apply(modulated,2,target.cwo246?7:target.spring246?4:target.phone246?3:target.grid246?1:target.delay246?0:target.nitro246?6:target.filter246?5:2);
        if(nativeBend&&(outputFx||punchSelected))modulated=bendControl.apply(modulated,2,target.cwo246?7:target.spring246?4:target.phone246?3:target.grid246?1:target.delay246?0:target.nitro246?6:target.filter246?5:2);
        if(nativeMidi&&(outputFx||punchSelected))modulated=midi246Apply(modulated,2,target.cwo246?7:target.spring246?4:target.phone246?3:target.grid246?1:target.delay246?0:target.nitro246?6:target.filter246?5:2,target.midi246Routes,midiCC);
        filterModulated=outputFx?modulated:outputControls;
        if(punchActive){const auto processed=punchEffect.next(driven,outputFx?punchRaw:modulated,phaseConfig.table);driven+=punchMix*(processed-driven);if(!punchSelected&&punchMix<.000001f){punchMix=0;punchActive=false;punchEffect.reset();}}
        const auto lag=std::clamp(size_t(sampleRate*p.delayTime),size_t(1),delay.size()-1);
        const float echo=lag<=delayValid?delay[(write+delay.size()-lag)%delay.size()]:0;
        delay[write]=std::tanh(driven+echo*std::clamp(p.feedback,0.f,.85f)); write=(write+1)%delay.size();if(delayValid<delay.size())++delayValid;
        const float output=(driven+echo*p.effect*.7f)*p.volume,limited=std::tanh(output);
        return limited+(output-limited)*cleanMix;
    }
private:
    static size_t channelIndex(int ch) { return size_t(std::clamp(ch,1,17)-1); }
    struct Voice {
        bool active=false,released=false,keyDown=false; int note=0,channel=1,stage=0;
        Dsynth246Voice dsynth246;Voltage246Voice voltage246;Dna246Voice dna246;String246Voice string246;
        Phase246Voice phase246;Sampler246Voice sampler246;
        Fm246Voice fm246;
        Digital246Voice digital246;
        DrWave246Voice drWave246;
        Cluster246Voice cluster246;
        float lastOutput=0,stealTail=0;unsigned stealRemaining=0;
        ValueVoice246Control valueVoice;Random246Control randomLfo;
        uint64_t age=0; float velocity=0,level=0,lastNoise=0;
        HybridGlide glide;
        double phase=0,modPhase=0,frequency=440,samplePosition=0,time=0,drumPhase=0;
    };
    bool hybridPerformance() const noexcept {return selectedSampler||isHybridEngine(selectedEngine)||(selectedEngine==0&&!selectedDigital);}
    bool monophonic() const noexcept {return hybridPerformance()&&hybridMode!=0;}
    void playHeldVoice(bool overlap) noexcept {
        auto& v=voices[0];const int id=held.current;
        const auto& n=held.notes[size_t(id)];
        const bool continuing=overlap&&v.active&&!v.released;
        const double from=v.glide.value;
        if(hybridMode==1||!continuing){
            const auto tail=v.active?v.lastOutput:0.f;const auto randomState=v.randomLfo;const auto valueState=v.valueVoice;
            v={};v.randomLfo=randomState;v.randomLfo.noteOn();v.valueVoice=valueState;v.valueVoice.noteOn();v.age=++age;
            v.cluster246.context.random=uint32_t(v.age)*2654435761u;
            v.stealTail=tail;v.stealRemaining=tail!=0?unsigned(sampleRate*.003):0;
        }
        v.active=true;v.released=false;v.keyDown=n.down;v.note=id%128;v.channel=id/128+1;v.velocity=n.velocity;
        v.frequency=440.*std::exp2((v.note-69)/12.);
        v.glide.start(continuing?from:v.frequency,v.frequency,continuing?glideMs:0,sampleRate);
    }
    void updateHeldVoice() noexcept {
        const int latest=held.latest();
        if(latest<0){held.current=-1;voices[0].keyDown=false;voices[0].released=true;return;}
        if(latest!=held.current){held.current=latest;playHeldVoice(true);}
        else voices[0].keyDown=held.notes[size_t(latest)].down;
    }
    HybridHeldNotes held;
    int hybridMode=0;float glideMs=0;
    Cluster246Config clusterConfig;bool nativeClusterActive=false,clusterControlsInitialized=false;
    std::array<float,4> clusterWords{};std::array<int16_t,4> clusterRaw{};
    DrWave246Config drWaveConfig;bool nativeDrWaveActive=false,drWaveControlsInitialized=false;int selectedEngine=0;bool selectedDigital=false,selectionInitialized=false;
    reference::Dsynth246Context dsynthContext;bool nativeDsynthActive=false,dsynthControlsInitialized=false;std::array<float,8> dsynthWords{};std::array<int16_t,8> dsynthRaw=dsynth246Defaults;
    bool nativeVoltageActive=false,voltageControlsInitialized=false;std::array<float,4> voltageWords{};std::array<int16_t,4> voltageRaw=voltage246Defaults;
    reference::Dna246Context dnaContext;bool nativeDnaActive=false,dnaControlsInitialized=false;std::array<float,4> dnaWords{};std::array<int16_t,4> dnaRaw=dna246Defaults;
    bool nativeStringActive=false,stringControlsInitialized=false;std::array<float,4> stringWords{};std::array<int16_t,4> stringRaw=string246Defaults;
    Fm246Config fmConfig;bool nativeFmActive=false,fmControlsInitialized=false;
    std::array<float,4> fmWords{};std::array<int16_t,4> fmRaw{};
    std::array<float,4> drWaveWords{};std::array<int16_t,4> drWaveRaw{};
    Drum246Bank drumBank;bool selectedDrum=false,selectedSampler=false;
    Dbox246Bank dboxBank;
    Phase246Config phaseConfig;bool phaseControlsInitialized=false,digitalControlsInitialized=false,nativeEnvelopeActive=false,nativeSamplerEnvelopeActive=false,nativeDigitalActive=false;
    std::array<float,4> filterWords{};std::array<int16_t,4> filterRaw{},filterModulated{{8000,0,0,24576}};bool filterControlsInitialized=false;
    Punch246Effect punchEffect;float punchMix=0;bool punchActive=false;
    uint32_t randomRng=0x52414e44u;
    Element246VoiceInput elementVoiceInput;Element246Control elementControl;float elementFeedback=0;
    Crank246Control crankControl;
    Bend246Control bendControl;
    std::array<uint32_t,4> midiCC{{64,64,64,64}};
    Value246Fx valueEffect;Random246FxControl randomFx;bool valueActive=false,valueVoiceActive=false;
    std::array<float,4> punchWords{};std::array<int16_t,4> punchRaw{};
    std::array<float,4> phaseWords{};std::array<int16_t,4> phaseRaw{};
    std::array<float,4> digitalWords{};std::array<int16_t,4> digitalRaw{};
    float noise() { random^=random<<13; random^=random>>17; random^=random<<5; return float(double(random)/4294967295.0*2-1); }
    bool tremoloActive=false;
    bool cleanInitialized=false;float cleanMix=0,modulatedFilterMix=0,currentSustain=.8f;
    float cachedAttack=-1,cachedDecay=-1,cachedRelease=-1,cachedMacro=-1,attackStep=0,decayFactor=0,releaseFactor=0;
    double cachedRoot=-1,rootFrequency=261.625565,cachedCutoff=-1;float filterCoefficient=0;
    double sampleRate=48000,lfoPhase=0; float smoothing=.0017f; Parameters current;
    std::array<reference::String246Memory,polyphony> stringBuffers{};
    std::array<Voice,polyphony> voices{}; std::array<float,192000> delay{};
    std::array<float,17> bends{}; std::array<bool,17> pedals{};
    Sample sample;Kit kit;bool sharedKit=false;int kitCount=8;
    std::array<float,kitCapacity> cachedPadPitch{};
    std::array<double,kitCapacity> padPitch{{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};
    size_t write=0,delayValid=0; uint64_t age=0; uint32_t random=0x12345678; float filter=0;
};
}
