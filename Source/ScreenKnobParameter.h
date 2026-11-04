// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Stable host identity, with the range and value of the currently visible control.
// The parameter is owned by AudioProcessor, after the existing APVTS parameters.
class ScreenKnobParameter final : public juce::RangedAudioParameter,
                                  private juce::AudioProcessorParameter::Listener {
    struct PadRoute {
        const int expectedEngine;std::array<std::atomic<juce::RangedAudioParameter*>,24> targets{};
        std::atomic<bool> active{false};std::atomic<std::atomic<float>*> engine{nullptr},pad{nullptr},enabled{nullptr},guard{nullptr};
        explicit PadRoute(int e):expectedEngine(e){}
        void attach(const std::array<juce::RangedAudioParameter*,24>& values,std::atomic<float>& e,std::atomic<float>& p,std::atomic<float>* flag,juce::AudioProcessorParameter::Listener* listener,std::atomic<float>* extra=nullptr){engine=&e;pad=&p;enabled=flag;guard=extra;for(size_t i=0;i<24;++i){targets[i]=values[i];if(std::find(values.begin(),values.begin()+int(i),values[i])==values.begin()+int(i))values[i]->addListener(listener);}active=true;}
        void detach(juce::AudioProcessorParameter::Listener* listener){if(!active.exchange(false))return;for(size_t i=0;i<24;++i){auto* target=targets[i].load();bool seen=false;for(size_t j=0;j<i;++j)seen|=targets[j].load()==target;if(!seen)target->removeListener(listener);}}
        juce::RangedAudioParameter* get() const {if(!active.load())return nullptr;auto* prerequisite=enabled.load();auto* g=guard.load();if((g&&g->load()<=.5f)||int(engine.load()->load())!=expectedEngine||(prerequisite&&prerequisite->load()<=.5f))return nullptr;return targets[size_t(std::clamp(int(pad.load()->load()),0,23))].load();}
    };
    std::atomic<std::atomic<float>*> drumTremoloEnabled{nullptr},samplerModulationEnabled{nullptr};
    PadRoute dboxRoute{11},drumRoute{5},samplerRoute{3};
public:
    ScreenKnobParameter(const juce::String& id,const juce::String& name)
        : RangedAudioParameter(juce::ParameterID{id,1},name,juce::AudioProcessorParameterWithIDAttributes{}) {}
    void disconnect() {
        if(auto* p=crankAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=bendAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=midiAlternative.exchange(nullptr))p->removeListener(this);
        dboxRoute.detach(this);drumRoute.detach(this);samplerRoute.detach(this);
        if(auto* p=dsynthAlternative.exchange(nullptr))p->removeListener(this);if(auto* p=dsynthSecondary.exchange(nullptr))p->removeListener(this);
        if(auto* p=voltageAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=dnaAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=stringAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=fmAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=randomAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=tremoloAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=cwoAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=springAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=phoneAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=gridAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=delayAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=nitroAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=filterAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=clusterAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=drWaveAlternative.exchange(nullptr))p->removeListener(this);
        if(trackMode.exchange(false))for(auto* p:tracks)p->removeListener(this);
        if(auto* p=alternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=digitalAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=voiceAlternative.exchange(nullptr))p->removeListener(this);
        if(auto* p=target.exchange(nullptr))p->removeListener(this);
    }
    void follow(juce::RangedAudioParameter* next) {
        if(!samplerRoute.active.load()&&!dboxRoute.active.load()&&!drumRoute.active.load()&&!dsynthAlternative.load()&&!voltageAlternative.load()&&!dnaAlternative.load()&&!stringAlternative.load()&&!fmAlternative.load()&&!trackMode.load()&&!alternative.load()&&!digitalAlternative.load()&&!drWaveAlternative.load()&&!clusterAlternative.load()&&!tremoloAlternative.load()&&!randomAlternative.load()&&!midiAlternative.load()&&!bendAlternative.load()&&!crankAlternative.load()&&!filterAlternative.load()&&!nitroAlternative.load()&&!delayAlternative.load()&&!gridAlternative.load()&&!phoneAlternative.load()&&!springAlternative.load()&&!cwoAlternative.load()&&currentTarget()==next)return;
        disconnect();target.store(next);
        if(next){next->addListener(this);sendValueChangedMessageToListeners(next->getValue());}
    }
    void followAlternative(juce::RangedAudioParameter* base,juce::RangedAudioParameter* other,std::atomic<float>& enabled,std::atomic<float>& engine,std::atomic<float>* prerequisite=nullptr) {
        disconnect();selection=&enabled;engineSelection=&engine;guard.store(prerequisite);target.store(base);alternative.store(other);
        base->addListener(this);other->addListener(this);sendValueChangedMessageToListeners(getValue());
    }
    void followSwitch(juce::RangedAudioParameter* base,juce::RangedAudioParameter* other,std::atomic<float>& enabled) {
        disconnect();selection=&enabled;engineSelection=nullptr;guard=nullptr;target=base;alternative=other;
        base->addListener(this);other->addListener(this);sendValueChangedMessageToListeners(getValue());
    }
    void followNative(juce::RangedAudioParameter* base,juce::RangedAudioParameter* phase,juce::RangedAudioParameter* digital,
                      std::atomic<float>& phaseEnabled,std::atomic<float>& digitalEnabled,std::atomic<float>& engine,
                      std::atomic<float>* phaseGuard=nullptr,std::atomic<float>* digitalGuard=nullptr) {
        followAlternative(base,phase,phaseEnabled,engine,phaseGuard);
        digitalSelection=&digitalEnabled;digitalPrerequisite.store(digitalGuard);digitalAlternative.store(digital);
        digital->addListener(this);sendValueChangedMessageToListeners(getValue());
    }
    void followSampler(const std::array<juce::RangedAudioParameter*,24>& targets,std::atomic<float>& engine,std::atomic<float>& bank,std::atomic<float>& enabled,std::atomic<float>* envelope=nullptr){samplerRoute.attach(targets,engine,bank,&enabled,this,envelope);sendValueChangedMessageToListeners(getValue());}
    void followSamplerModulationAvailability(std::atomic<float>& enabled){samplerModulationEnabled=&enabled;}
    void followDrumTremoloAvailability(std::atomic<float>& enabled){drumTremoloEnabled=&enabled;}
    void followDbox(const std::array<juce::RangedAudioParameter*,24>& targets,std::atomic<float>& engine,std::atomic<float>& pad){dboxRoute.attach(targets,engine,pad,nullptr,this);sendValueChangedMessageToListeners(getValue());}
    void followDrum(const std::array<juce::RangedAudioParameter*,24>& targets,std::atomic<float>& engine,std::atomic<float>& pad,std::atomic<float>& enabled){drumRoute.attach(targets,engine,pad,&enabled,this);sendValueChangedMessageToListeners(getValue());}
    void followDsynth(juce::RangedAudioParameter* primary,juce::RangedAudioParameter* secondary,std::atomic<float>& engine,std::atomic<float>& bank){dsynthEngine=&engine;dsynthBank=&bank;dsynthAlternative=primary;primary->addListener(this);if(secondary!=primary){dsynthSecondary=secondary;secondary->addListener(this);}sendValueChangedMessageToListeners(getValue());}
    void followVoltage(juce::RangedAudioParameter* parameter,std::atomic<float>& engine){voltageEngine=&engine;voltageAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followDna(juce::RangedAudioParameter* parameter,std::atomic<float>& engine){dnaEngine=&engine;dnaAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followString(juce::RangedAudioParameter* parameter,std::atomic<float>& engine){stringEngine=&engine;stringAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followFm(juce::RangedAudioParameter* parameter,std::atomic<float>& engine,std::atomic<float>& digital){fmEngine=&engine;fmDigital=&digital;fmAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followCwo(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled){cwoSelection=&enabled;cwoAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followSpring(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled){springSelection=&enabled;springAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followPhone(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled){phoneSelection=&enabled;phoneAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followGrid(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled){gridSelection=&enabled;gridAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followDelay(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled){delaySelection=&enabled;delayAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followNitro(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled){nitroSelection=&enabled;nitroAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followFilter(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled){filterSelection=&enabled;filterAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followDrWave(juce::RangedAudioParameter* parameter,std::atomic<float>& engine){drWaveEngine=&engine;drWaveAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followCluster(juce::RangedAudioParameter* parameter,std::atomic<float>& engine){clusterEngine=&engine;clusterAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followValue(juce::RangedAudioParameter* base,juce::RangedAudioParameter* fx,juce::RangedAudioParameter* voice,
                     std::atomic<float>& fxEnabled,std::atomic<float>& voiceEnabled,std::atomic<float>& engine,
                     std::atomic<float>& phaseEnabled,std::atomic<float>& digitalEnabled) {
        followSwitch(base,fx,fxEnabled);voiceSelection=&voiceEnabled;voiceEngine=&engine;
        voicePhase=&phaseEnabled;voiceDigital=&digitalEnabled;voiceAlternative=voice;
        voice->addListener(this);sendValueChangedMessageToListeners(getValue());
    }
    void followTremolo(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled,std::atomic<float>& engine,std::atomic<float>& phase,std::atomic<float>& digital){
        tremoloSelection=&enabled;tremoloEngine=&engine;tremoloPhase=&phase;tremoloDigital=&digital;tremoloAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());
    }
    void followCrank(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled,bool(*available)(const void*),const void* context){crankSelection=&enabled;crankAvailable=available;crankContext=context;crankAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followBend(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled,bool(*available)(const void*),const void* context){bendSelection=&enabled;bendAvailable=available;bendContext=context;bendAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followMidi(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled,bool(*available)(const void*),const void* context){midiSelection=&enabled;midiAvailable=available;midiContext=context;midiAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());}
    void followRandom(juce::RangedAudioParameter* parameter,std::atomic<float>& enabled,std::atomic<float>& engine,std::atomic<float>& phase,std::atomic<float>& digital){
        randomSelection=&enabled;randomEngine=&engine;randomPhase=&phase;randomDigital=&digital;randomAlternative=parameter;parameter->addListener(this);sendValueChangedMessageToListeners(getValue());
    }
    void enableTapeTracks(const std::array<juce::RangedAudioParameter*,4>& parameters,std::atomic<float>& selection) {tracks=parameters;trackSelection=&selection;}
    void followTapeTrack() {
        disconnect();trackMode.store(true);for(auto* p:tracks)p->addListener(this);
        sendValueChangedMessageToListeners(getValue());
    }
    juce::RangedAudioParameter* followedParameter() const {return currentTarget();}
    void enableTapeCursor(std::atomic<float>& position,std::atomic<float>& pending,std::atomic<float>& length) {cursor=&position;seek=&pending;tapeLength=&length;}
    float getValue() const override {if(auto* p=currentTarget())return p->getValue();return cursor?std::clamp(cursor->load()/duration(),0.f,1.f):0;}
    void setValue(float value) override {if(auto* p=currentTarget())p->setValueNotifyingHost(std::clamp(value,0.f,1.f));else if(seek)seek->store(std::clamp(value,0.f,1.f)*duration());}
    float getDefaultValue() const override {if(auto* p=currentTarget())return p->getDefaultValue();return 0;}
    juce::String getText(float value,int length) const override {if(auto* p=currentTarget())return p->getText(value,length);return juce::String(value*duration(),2)+" s";}
    float getValueForText(const juce::String& text) const override {if(auto* p=currentTarget())return p->getValueForText(text);return std::clamp(text.getFloatValue()/duration(),0.f,1.f);}
    int getNumSteps() const override {return juce::AudioProcessor::getDefaultNumParameterSteps();}
    const juce::NormalisableRange<float>& getNormalisableRange() const override {
        if(auto* p=currentTarget())return p->getNormalisableRange();return fallback;
    }
private:
    void parameterValueChanged(int index,float value) override {
        if(auto* p=currentTarget())if(p->getParameterIndex()==index)sendValueChangedMessageToListeners(value);
    }
    void parameterGestureChanged(int,bool) override {}
    std::atomic<juce::RangedAudioParameter*> crankAlternative{nullptr};std::atomic<std::atomic<float>*> crankSelection{nullptr};std::atomic<bool(*)(const void*)> crankAvailable{nullptr};std::atomic<const void*> crankContext{nullptr};
    std::atomic<juce::RangedAudioParameter*> bendAlternative{nullptr};std::atomic<std::atomic<float>*> bendSelection{nullptr};std::atomic<bool(*)(const void*)> bendAvailable{nullptr};std::atomic<const void*> bendContext{nullptr};
    std::atomic<juce::RangedAudioParameter*> midiAlternative{nullptr};std::atomic<std::atomic<float>*> midiSelection{nullptr};std::atomic<bool(*)(const void*)> midiAvailable{nullptr};std::atomic<const void*> midiContext{nullptr};
    std::atomic<juce::RangedAudioParameter*> randomAlternative{nullptr};
    std::atomic<std::atomic<float>*> randomSelection{nullptr},randomEngine{nullptr},randomPhase{nullptr},randomDigital{nullptr};
    std::atomic<juce::RangedAudioParameter*> tremoloAlternative{nullptr};
    std::atomic<std::atomic<float>*> tremoloSelection{nullptr},tremoloEngine{nullptr},tremoloPhase{nullptr},tremoloDigital{nullptr};
    std::atomic<juce::RangedAudioParameter*> cwoAlternative{nullptr};std::atomic<std::atomic<float>*> cwoSelection{nullptr};
    std::atomic<juce::RangedAudioParameter*> springAlternative{nullptr};std::atomic<std::atomic<float>*> springSelection{nullptr};
    std::atomic<juce::RangedAudioParameter*> phoneAlternative{nullptr};std::atomic<std::atomic<float>*> phoneSelection{nullptr};
    std::atomic<juce::RangedAudioParameter*> gridAlternative{nullptr};std::atomic<std::atomic<float>*> gridSelection{nullptr};
    std::atomic<juce::RangedAudioParameter*> delayAlternative{nullptr};std::atomic<std::atomic<float>*> delaySelection{nullptr};
    std::atomic<juce::RangedAudioParameter*> nitroAlternative{nullptr};std::atomic<std::atomic<float>*> nitroSelection{nullptr};
    std::atomic<juce::RangedAudioParameter*> filterAlternative{nullptr};std::atomic<std::atomic<float>*> filterSelection{nullptr};
    std::atomic<juce::RangedAudioParameter*> clusterAlternative{nullptr};std::atomic<std::atomic<float>*> clusterEngine{nullptr};
    std::atomic<juce::RangedAudioParameter*> drWaveAlternative{nullptr};std::atomic<std::atomic<float>*> drWaveEngine{nullptr};
    std::atomic<juce::RangedAudioParameter*> dsynthAlternative{nullptr},dsynthSecondary{nullptr};std::atomic<std::atomic<float>*> dsynthEngine{nullptr},dsynthBank{nullptr};
    std::atomic<juce::RangedAudioParameter*> voltageAlternative{nullptr};std::atomic<std::atomic<float>*> voltageEngine{nullptr};
    std::atomic<juce::RangedAudioParameter*> dnaAlternative{nullptr};std::atomic<std::atomic<float>*> dnaEngine{nullptr};
    std::atomic<juce::RangedAudioParameter*> stringAlternative{nullptr};std::atomic<std::atomic<float>*> stringEngine{nullptr};
    std::atomic<juce::RangedAudioParameter*> fmAlternative{nullptr};std::atomic<std::atomic<float>*> fmEngine{nullptr},fmDigital{nullptr};
    std::atomic<juce::RangedAudioParameter*> target{nullptr};
    std::atomic<juce::RangedAudioParameter*> alternative{nullptr};
    std::atomic<juce::RangedAudioParameter*> digitalAlternative{nullptr};
    std::atomic<juce::RangedAudioParameter*> voiceAlternative{nullptr};
    std::atomic<std::atomic<float>*> voiceSelection{nullptr},voiceEngine{nullptr},voicePhase{nullptr},voiceDigital{nullptr};
    std::atomic<std::atomic<float>*> digitalSelection{nullptr},digitalPrerequisite{nullptr};
    std::atomic<std::atomic<float>*> selection{nullptr},engineSelection{nullptr},guard{nullptr};
    std::array<juce::RangedAudioParameter*,4> tracks{};
    std::atomic<float>* trackSelection=nullptr;
    std::atomic<bool> trackMode{false};
    juce::RangedAudioParameter* currentTarget() const {if(auto* p=crankAlternative.load())if(crankSelection.load()->load()>.5f){const auto available=crankAvailable.load();if(available&&available(crankContext.load()))return p;}if(auto* p=bendAlternative.load())if(bendSelection.load()->load()>.5f){const auto available=bendAvailable.load();if(available&&available(bendContext.load()))return p;}if(auto* p=midiAlternative.load())if(midiSelection.load()->load()>.5f){const auto available=midiAvailable.load();if(available&&available(midiContext.load()))return p;}if(auto* p=samplerRoute.get())return p;if(auto* p=drumRoute.get())return p;if(auto* p=dboxRoute.get())return p;if(auto* p=dsynthAlternative.load())if(int(dsynthEngine.load()->load())==10){auto* q=dsynthSecondary.load();return q&&dsynthBank.load()->load()>.5f?q:p;}if(auto* p=voltageAlternative.load())if(int(voltageEngine.load()->load())==9)return p;if(auto* p=dnaAlternative.load())if(int(dnaEngine.load()->load())==8)return p;if(auto* p=stringAlternative.load())if(int(stringEngine.load()->load())==2)return p;if(auto* p=fmAlternative.load())if(int(fmEngine.load()->load())==0&&fmDigital.load()->load()<=.5f)return p;if(auto* p=cwoAlternative.load())if(cwoSelection.load()->load()>.5f)return p;if(auto* p=springAlternative.load())if(springSelection.load()->load()>.5f)return p;if(auto* p=phoneAlternative.load())if(phoneSelection.load()->load()>.5f)return p;if(auto* p=gridAlternative.load())if(gridSelection.load()->load()>.5f)return p;if(auto* p=delayAlternative.load())if(delaySelection.load()->load()>.5f)return p;if(auto* p=nitroAlternative.load())if(nitroSelection.load()->load()>.5f)return p;if(auto* p=filterAlternative.load())if(filterSelection.load()->load()>.5f)return p;if(auto* p=randomAlternative.load())if(randomSelection.load()->load()>.5f){const auto e=int(randomEngine.load()->load());if(e==6||e==7||(e==1&&randomPhase.load()->load()>.5f)||e==0||e==2||e==8||e==9||e==10||e==11||(e==5&&drumTremoloEnabled.load()&&drumTremoloEnabled.load()->load()>.5f)||(e==3&&samplerModulationEnabled.load()&&samplerModulationEnabled.load()->load()>.5f))return p;}if(auto* p=tremoloAlternative.load())if(tremoloSelection.load()->load()>.5f){const auto e=int(tremoloEngine.load()->load());if(e==6||e==7||(e==1&&tremoloPhase.load()->load()>.5f)||e==0||e==2||e==8||e==9||e==10||e==11||(e==5&&drumTremoloEnabled.load()&&drumTremoloEnabled.load()->load()>.5f)||(e==3&&samplerModulationEnabled.load()&&samplerModulationEnabled.load()->load()>.5f)||e==11||(e==5&&drumTremoloEnabled.load()&&drumTremoloEnabled.load()->load()>.5f))return p;}if(auto* p=clusterAlternative.load())if(int(clusterEngine.load()->load())==7)return p;if(auto* p=drWaveAlternative.load())if(int(drWaveEngine.load()->load())==6)return p;if(trackMode.load())return tracks[size_t(std::clamp(int(trackSelection->load()),0,3))];
        if(auto* p=voiceAlternative.load())if(voiceSelection.load()->load()>.5f){const auto e=int(voiceEngine.load()->load());if(e==6||e==7||(e==1&&voicePhase.load()->load()>.5f)||e==0||e==2||e==8||e==9||e==10||e==11||(e==5&&drumTremoloEnabled.load()&&drumTremoloEnabled.load()->load()>.5f)||(e==3&&samplerModulationEnabled.load()&&samplerModulationEnabled.load()->load()>.5f))return p;}
        if(auto* p=digitalAlternative.load())if(digitalSelection.load()->load()>.5f&&int(engineSelection.load()->load())==0){auto* condition=digitalPrerequisite.load();if(!condition||condition->load()>.5f)return p;}
        if(auto* p=alternative.load())if(selection.load()->load()>.5f){auto* engine=engineSelection.load();if(!engine||int(engine->load())==1){auto* condition=guard.load();if(!condition||condition->load()>.5f)return p;}}
        return target.load();}
    std::atomic<float>* cursor=nullptr,*seek=nullptr,*tapeLength=nullptr;
    float duration() const {return tapeLength?float(4<<std::clamp(int(tapeLength->load()),0,2)):1.f;}
    const juce::NormalisableRange<float> fallback{0.f,1.f};
};
