// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Stable host identity, with the range and value of the currently visible control.
// The parameter is owned by AudioProcessor, after the existing APVTS parameters.
class ScreenKnobParameter final : public juce::RangedAudioParameter,
                                  private juce::AudioProcessorParameter::Listener {
public:
    ScreenKnobParameter(const juce::String& id,const juce::String& name)
        : RangedAudioParameter(juce::ParameterID{id,1},name,juce::AudioProcessorParameterWithIDAttributes{}) {}
    void disconnect() {
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
        if(!fmAlternative.load()&&!trackMode.load()&&!alternative.load()&&!digitalAlternative.load()&&!drWaveAlternative.load()&&!clusterAlternative.load()&&!tremoloAlternative.load()&&!randomAlternative.load()&&!filterAlternative.load()&&!nitroAlternative.load()&&!delayAlternative.load()&&!gridAlternative.load()&&!phoneAlternative.load()&&!springAlternative.load()&&!cwoAlternative.load()&&currentTarget()==next)return;
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
    juce::RangedAudioParameter* currentTarget() const {if(auto* p=fmAlternative.load())if(int(fmEngine.load()->load())==0&&fmDigital.load()->load()<=.5f)return p;if(auto* p=cwoAlternative.load())if(cwoSelection.load()->load()>.5f)return p;if(auto* p=springAlternative.load())if(springSelection.load()->load()>.5f)return p;if(auto* p=phoneAlternative.load())if(phoneSelection.load()->load()>.5f)return p;if(auto* p=gridAlternative.load())if(gridSelection.load()->load()>.5f)return p;if(auto* p=delayAlternative.load())if(delaySelection.load()->load()>.5f)return p;if(auto* p=nitroAlternative.load())if(nitroSelection.load()->load()>.5f)return p;if(auto* p=filterAlternative.load())if(filterSelection.load()->load()>.5f)return p;if(auto* p=randomAlternative.load())if(randomSelection.load()->load()>.5f){const auto e=int(randomEngine.load()->load());if(e==6||e==7||(e==1&&randomPhase.load()->load()>.5f)||(e==0&&randomDigital.load()->load()>.5f))return p;}if(auto* p=tremoloAlternative.load())if(tremoloSelection.load()->load()>.5f){const auto e=int(tremoloEngine.load()->load());if(e==6||e==7||(e==1&&tremoloPhase.load()->load()>.5f)||(e==0&&tremoloDigital.load()->load()>.5f))return p;}if(auto* p=clusterAlternative.load())if(int(clusterEngine.load()->load())==7)return p;if(auto* p=drWaveAlternative.load())if(int(drWaveEngine.load()->load())==6)return p;if(trackMode.load())return tracks[size_t(std::clamp(int(trackSelection->load()),0,3))];
        if(auto* p=voiceAlternative.load())if(voiceSelection.load()->load()>.5f){const auto e=int(voiceEngine.load()->load());if(e==6||e==7||(e==1&&voicePhase.load()->load()>.5f)||(e==0&&voiceDigital.load()->load()>.5f))return p;}
        if(auto* p=digitalAlternative.load())if(digitalSelection.load()->load()>.5f&&int(engineSelection.load()->load())==0){auto* condition=digitalPrerequisite.load();if(!condition||condition->load()>.5f)return p;}
        if(auto* p=alternative.load())if(selection.load()->load()>.5f){auto* engine=engineSelection.load();if(!engine||int(engine->load())==1){auto* condition=guard.load();if(!condition||condition->load()>.5f)return p;}}
        return target.load();}
    std::atomic<float>* cursor=nullptr,*seek=nullptr,*tapeLength=nullptr;
    float duration() const {return tapeLength?float(4<<std::clamp(int(tapeLength->load()),0,2)):1.f;}
    const juce::NormalisableRange<float> fallback{0.f,1.f};
};
