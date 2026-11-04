// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"
#include "Filter246Effect.h"
#include "Phase246Presets.h"
#include "Phase246Envelopes.h"
#include "Digital246Presets.h"
#include "Punch246Presets.h"
#include "Tape.h"
#include "NoteQueue.h"
#include "SoundLibrary.h"
#include "Sequencer.h"
#include "ScreenKnobParameter.h"
class PocketProcessor : public juce::AudioProcessor,private juce::MidiKeyboardStateListener {
public:
    explicit PocketProcessor(juce::File libraryDirectory={});
    ~PocketProcessor() override;
    void updateKeyboardDisplay();
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Pocket One"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override;
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    bool loadSample(const juce::File&,juce::String& error);
    bool exportTape(const juce::File&,juce::String& error);
    void clearTape();
    void rewindTape();
    void requestPanic(bool clearOutput=true) {if(clearOutput)panicOutputRequested.store(true);panicRequested.store(true); }
    void applyPreset(int index);
    static juce::StringArray factoryPresetNames();
    void applyPhase246Preset(int index);
    void applyDigital246Preset(int index);
    void applyDrWave246Init();
    void applyCluster246Init();
    void applyDna246Init();
    void applyVoltage246Init();void applyDsynth246Init();
    void applyDbox246Init();
    void applyDrum246Init();
    bool cluster246Enabled() const {return int(audioParameters[size_t(Param::engine)]->load())==7;}
    void applyPunch246Preset(int index);
    void applyValue246Preset(int index);
    void applyValueVoice246Preset(int index);
    bool drWave246Enabled() const {return int(audioParameters[size_t(Param::engine)]->load())==6;}
    bool cwo246Enabled() const {return phaseAudio[96]->load()>.5f;}
    bool spring246Enabled() const {return !cwo246Enabled()&& phaseAudio[91]->load()>.5f;}
    bool phone246Enabled() const {return !cwo246Enabled()&& !spring246Enabled()&& phaseAudio[86]->load()>.5f;}
    bool grid246Enabled() const {return !cwo246Enabled()&& !spring246Enabled()&& !phone246Enabled()&& phaseAudio[81]->load()>.5f;}
    bool delay246Enabled() const {return !cwo246Enabled()&& !spring246Enabled()&& !phone246Enabled()&&!grid246Enabled()&& phaseAudio[76]->load()>.5f;}
    bool nitro246Enabled() const {return !cwo246Enabled()&& !spring246Enabled()&& !phone246Enabled()&&!grid246Enabled()&&!delay246Enabled()&& phaseAudio[71]->load()>.5f;}
    bool filter246Enabled() const {return !cwo246Enabled()&& !spring246Enabled()&& !phone246Enabled()&&!grid246Enabled()&&!delay246Enabled()&&!nitro246Enabled()&&phaseAudio[66]->load()>.5f;}
    bool punch246Enabled() const {return !cwo246Enabled()&& !spring246Enabled()&& !phone246Enabled()&&!grid246Enabled()&&!delay246Enabled()&&!nitro246Enabled()&&!filter246Enabled()&&phaseAudio[20]->load()>.5f;}
    bool midi246Available() const {return valueVoice246Available()||punch246Enabled()||filter246Enabled()||nitro246Enabled()||delay246Enabled()||grid246Enabled()||phone246Enabled()||spring246Enabled()||cwo246Enabled();}
    bool element246Enabled() const {return elementAudio[0]->load()>.5f&&midi246Available();}
    int element246Source() const {return int(elementAudio[1]->load());}
    bool element246TargetAvailable() const {const int dest=int(elementAudio[3]->load()),colour=int(elementAudio[4]->load());if(dest&&colour>=4)return false;return midi246RouteAvailable(dest==0?colour+1:dest==1?colour+9:colour+13);}
    std::atomic<float> visualElement{0};
    bool crank246Enabled() const {return !element246Enabled()&&crankAudio[0]->load()>.5f&&midi246Available();}
    bool crank246TargetAvailable() const {const int dest=int(crankAudio[3]->load()),colour=int(crankAudio[4]->load());if(dest&&colour>=4)return false;return midi246RouteAvailable(dest==0?colour+1:dest==1?colour+9:colour+13);}
    std::atomic<float> visualCrank{0};
    bool bend246Enabled() const {return !element246Enabled()&&!crank246Enabled()&&bendAudio[0]->load()>.5f&&midi246Available();}
    bool bend246TargetAvailable() const {const int colour=int(bendAudio[1]->load()),dest=int(bendAudio[2]->load());if(dest&&colour>=4)return false;return midi246RouteAvailable(dest==0?colour+1:dest==1?colour+9:colour+13);}
    std::atomic<float> visualBend{0};
    bool midi246Enabled() const {return !element246Enabled()&&!crank246Enabled()&&!bend246Enabled()&&midiAudio[0]->load()>.5f&&midi246Available();}
    bool midi246RouteAvailable(int route) const {
        if(route>=1&&route<=8)return valueVoice246Available()&&(route<=4||sampler246Enabled()||dsynth246Enabled()||drum246Enabled()||dbox246Enabled());
        if(route>=9&&route<=12)return valueVoice246Available()&&!drum246Enabled()&&!dbox246Enabled()&&(!phase246Enabled()||phase246EnvelopeEnabled())&&(!digital246Enabled()||digital246EnvelopeEnabled())&&(!sampler246Enabled()||sampler246EnvelopeEnabled());
        if(route>=13&&route<=16)return punch246Enabled()||filter246Enabled()||nitro246Enabled()||delay246Enabled()||grid246Enabled()||phone246Enabled()||spring246Enabled()||cwo246Enabled();
        if(route==19||route==20)return valueVoice246Available()&&!drum246Enabled()&&!dbox246Enabled();
        if(route==17||route==18)return filter246Enabled()||nitro246Enabled()||delay246Enabled()||grid246Enabled()||phone246Enabled()||spring246Enabled()||cwo246Enabled();
        return false;
    }
    std::array<int,4> midi246Routes() const {std::array<int,4> out{};for(size_t i=0;i<4;++i)out[i]=int(midiAudio[i+1]->load());return out;}
    std::array<std::atomic<float>,4> visualMidiCC{{64,64,64,64}};
    bool random246Enabled() const {return !element246Enabled()&&!crank246Enabled()&&!bend246Enabled()&&!midi246Enabled()&&phaseAudio[61]->load()>.5f&&valueVoice246Available();}
    bool tremolo246Enabled() const {return !element246Enabled()&&!crank246Enabled()&&!bend246Enabled()&&!midi246Enabled()&&!random246Enabled()&& phaseAudio[55]->load()>.5f&&(valueVoice246Available()||dbox246Enabled()||drum246Enabled());}
    bool value246Enabled() const {return !element246Enabled()&&!crank246Enabled()&&!bend246Enabled()&&!midi246Enabled()&&!random246Enabled()&&phaseAudio[25]->load()>.5f&&!valueVoice246Enabled()&&!tremolo246Enabled();}
    bool valueVoice246Available() const {return drum246Enabled()||dbox246Enabled()||sampler246Enabled()||dsynth246Enabled()||voltage246Enabled()||dna246Enabled()||string246Enabled()||fm246Enabled()||phase246Enabled()||digital246Enabled()||drWave246Enabled()||cluster246Enabled();}
    bool valueVoice246Enabled() const {return !element246Enabled()&&!crank246Enabled()&&!bend246Enabled()&&!midi246Enabled()&&!random246Enabled()&&phaseAudio[31]->load()>.5f&&valueVoice246Available()&&!tremolo246Enabled();}
    juce::AudioProcessorValueTreeState& phaseParameters() {return *phaseState;}
    bool phase246Enabled() const {return phaseAudio[0]->load()>.5f&&int(audioParameters[size_t(Param::engine)]->load())==1;}
    bool phase246EnvelopeEnabled() const {return phase246Enabled()&&phaseAudio[5]->load()>.5f;}
    bool sampler246Enabled() const {return int(audioParameters[size_t(Param::engine)]->load())==3&&samplerAudio[0]->load()>.5f;}
    bool drum246LfoSecondary() const {return drumLfoBankAudio->load()>.5f;}
    bool sampler246Secondary() const {return samplerAudio[2]->load()>.5f;}
    bool sampler246EnvelopeEnabled() const {return sampler246Enabled()&&samplerAudio[3]->load()>.5f;}
    std::array<int16_t,8> sampler246ControlValues() const {std::array<int16_t,8> c{};for(size_t i=0;i<8;++i)c[i]=int16_t(i==4?8192+samplerAudio[4+i]->load()*16384:samplerAudio[4+i]->load());return c;}
    void resetSampler246Controls(const pocket::Sample&);
    bool drum246Enabled() const {return int(audioParameters[size_t(Param::engine)]->load())==5&&drumAudio[0]->load()>.5f;}
    int drum246SelectedPad() const {return std::clamp(int(drumAudio[5]->load()),0,23);}
    static juce::String drum246ModeId(int pad){return "drum246Pad"+juce::String(pad+1)+"Mode";}
    bool dbox246Enabled() const {return int(audioParameters[size_t(Param::engine)]->load())==11;}
    int dbox246SelectedPad() const {return std::clamp(int(dboxPadSelection->load()),0,23);}
    static juce::String dbox246ParameterId(int pad,int control) {return "dbox246Pad"+juce::String(pad+1)+"Control"+juce::String(control+1);}
    bool dsynth246Enabled() const {return int(audioParameters[size_t(Param::engine)]->load())==10;}
    bool dsynth246Secondary() const {return phaseAudio[145]->load()>.5f;}
    bool voltage246Enabled() const {return int(audioParameters[size_t(Param::engine)]->load())==9;}
    bool dna246Enabled() const {return int(audioParameters[size_t(Param::engine)]->load())==8;}
    bool string246Enabled() const {return int(audioParameters[size_t(Param::engine)]->load())==2;}
    bool fm246Enabled() const {return int(audioParameters[size_t(Param::engine)]->load())==0&&phaseAudio[10]->load()<=.5f;}
    bool digital246Enabled() const {return phaseAudio[10]->load()>.5f&&int(audioParameters[size_t(Param::engine)]->load())==0;}
    bool digital246EnvelopeEnabled() const {return digital246Enabled()&&phaseAudio[15]->load()>.5f;}
    std::atomic<int> currentFactoryPreset{-1},currentUserPreset{-1};
    juce::StringArray userPresetNames() const;
    std::vector<SoundInfo> soundCatalog() const;
    bool loadSound(const juce::String& id,juce::String& error);
    juce::String selectedSound() const;
    juce::String lastSound(bool drums) const;
    bool isFavorite(const juce::String&) const;
    void toggleFavorite(const juce::String&);
    bool assignSoundSlot(bool drums,int slot);
    bool recallSoundSlot(bool drums,int slot,juce::String& error);
    juce::String soundSlot(bool drums,int slot) const;
    bool importSamples(const juce::Array<juce::File>&,juce::String& error);
    std::atomic<int> clipboardKind{0};
    bool copySound(juce::String& error);
    bool pasteSound(juce::String& error);
    bool tapeEdit(const juce::String& action,bool shifted,juce::String& error);
    std::pair<float,float> currentTapeTake();
    bool slideTape(double delta,juce::String& error);
    void seekTape(double seconds) {pendingTapeSeek.store(float(seconds));}
    std::atomic<float> tapeSpool{0};
    std::atomic<bool> tapeBreak{false},tapeChop{false};
    void tapeMemo(int slot,bool store);
    bool startInputCapture(juce::String& error);
    bool finishInputCapture(juce::String& error);
    std::atomic<bool> inputCapturing{false};
    std::atomic<int> inputCaptureFrames{0};
    bool exportMix(const juce::File&,juce::String& error);
    std::atomic<int> kitPads{8};
    bool drum246SharedSource() const {const juce::ScopedLock lock(getCallbackLock());return engine.isSharedKit();}
    bool loadSharedDrumSource(const juce::File&,juce::String&);
    bool installSharedDrumSource(pocket::Sample, const juce::String&,juce::String&);
    void resetSharedDrumRegion(int);
    bool loadKitPad(int,const juce::File&,juce::String&);
    std::array<float,128> kitPadPreview(int);
    juce::String kitPadName(int) const;
    static juce::String padParameterId(int,const char*);
    bool importUserBank(const juce::File&,juce::String& error);
    bool purgeUserBanks(const juce::String& bank,juce::String& error);
    void setScreenPage(int page,bool tape=false);
    int screenPage() const {return visiblePage.load();}
    bool screenTapeMode() const {return visibleTape.load();}
    ScreenKnobParameter& screenKnob(int index) {return *screenKnobs[size_t(index)];}
    juce::String screenParameterId(int index) const;
    bool applyUserPreset(int index,juce::String& error);
    juce::AudioProcessorValueTreeState state;
    juce::MidiKeyboardState keyboard;
    std::array<std::atomic<float>,256> scope{};
    std::atomic<float> effectiveTempo{110};
    std::atomic<int> keyboardOctave{0};
    std::atomic<int> playhead{-1},lastNote{60},sequenceNote{-1};
    std::atomic<float> tapePosition{0};
    std::atomic<int> sampleFrames{0};
    std::array<std::atomic<float>,128> samplePreview{},samplerPreview{};std::atomic<int> samplerFrames{0};
    std::array<std::atomic<float>,4> visualRandom{};
    std::atomic<float> visualEnvelope{0},visualSamplePosition{0},visualLfoPhase{0},visualTime{0},visualLevel{0};
    std::atomic<int> visualVoices{0},visualEnvelopeStage{0},visualFmAlgorithm{0};
    std::atomic<uint64_t> visualTrigger{0};
    std::array<std::atomic<int>,16> steps{};
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    std::unique_ptr<juce::AudioProcessorValueTreeState> phaseState;
    std::array<std::atomic<float>*,5> midiAudio{};
    std::array<std::atomic<float>*,6> elementAudio{};
    std::array<std::atomic<float>*,5> crankAudio{};
    std::array<std::atomic<float>*,6> bendAudio{};
    bool captureAsSharedDrum=false;
    std::atomic<float>* drumLfoBankAudio=nullptr;std::array<std::atomic<float>*,16> samplerAudio{};std::array<std::atomic<float>*,6> drumAudio{};std::array<std::atomic<float>*,24> drumModeAudio{};
    std::array<std::array<std::atomic<float>*,8>,24> dboxAudio{};std::atomic<float>* dboxPadSelection=nullptr;
    std::array<std::atomic<float>*,146> phaseAudio{};
    juce::ValueTree normalizedPhaseState(juce::ValueTree) const;
    enum class Param {engine,tone,color,envelope,effect,attack,decay,sustain,release,lfoRate,lfoDepth,lfoDestination,lfoShape,delayTime,delayFeedback,drive,sampleLoop,sampleRoot,volume,sequencer,tempo,track1,track2,track3,track4,tapePlay,tapeRecord,tapeOverdub,tapeTrack,tapeSpeed,tapeLength,tapeLoop,tapeLoopIn,tapeLoopOut,metronome,inputMonitor,inputGain,tapeRecordGain,eqLow,eqMid,eqHigh,eqMix,masterDrive,masterEcho,masterDelayTime,masterFeedback,clickVolume,seqMode,seqLength,seqRate,seqGate,seqSwing,arpOctaves,seqVelocity,seqEuclidean,seqPulses,seqRotation,sampleClean,Count};
    std::array<std::atomic<float>*,size_t(Param::Count)> audioParameters{};
    std::array<std::array<std::atomic<float>*,6>,pocket::kitCapacity> padParameters{};
    void handleNoteOn(juce::MidiKeyboardState*,int,int,float) override;
    void handleNoteOff(juce::MidiKeyboardState*,int,int,float) override;
    pocket::NoteQueue uiToAudio,audioToUi;
    std::atomic<bool> keyboardOverflow{false},keyboardDisplayReset{false};
    bool reflectingHostNotes=false;
    struct UserPreset {juce::String name,id,bank,source;juce::File sample;std::array<juce::File,pocket::kitCapacity> kit;int kitCount=8;juce::var parameters,pads;bool isKit=false;};
    mutable juce::CriticalSection libraryStateLock;
    std::array<juce::String,pocket::kitCapacity> kitNames;
    juce::String selectedSoundId,lastSynthSound="factory:0",lastDrumSound="factory:22";
    juce::StringArray favorites;
    std::array<std::array<juce::String,8>,2> soundSlotIds;
    std::array<std::array<juce::ValueTree,8>,2> slotSettings;
    juce::ValueTree soundClipboard;juce::String soundClipboardId;
    juce::ValueTree librarySnapshot() const;
    void restoreLibrary(const juce::ValueTree&);
    void setSelectedSound(const juce::String&);
    void initializeSoundSlots();
    juce::ValueTree tapeEditSnapshot();
    void restoreTapeEdits(const juce::ValueTree&);
    std::atomic<float> pendingTapeSeek{-1};
    std::array<std::array<float,21>,2> tapeMemos{};
    std::array<bool,2> memoSaved{};
    juce::File libraryDirectory;
    std::vector<UserPreset> userPresets;
    std::array<ScreenKnobParameter*,4> screenKnobs{};
    std::atomic<int> visiblePage{0};
    std::atomic<bool> visibleTape{false};
    void refreshUserPresets();
    std::array<std::array<std::atomic<float>,128>,pocket::kitCapacity> padPreviews{},drumPreviews{};std::array<std::atomic<int>,pocket::kitCapacity> drumFrames{};
    static juce::String encodeAudio(const juce::AudioBuffer<float>&,double);
    static bool decodeAudio(const juce::String&,juce::AudioBuffer<float>&,double&,int);
    void publishSharedDrumPreview(const pocket::Sample&);
    void publishPadPreview(int,const pocket::Sample&);
    void publishSamplePreview(const pocket::Sample&);
    double processedTime=0;
    static pocket::Sample readSample(const juce::File&,juce::String& error);
    pocket::Sample factorySample(const juce::String& name) const;
    void loadFactoryKit(int index);
    pocket::Engine engine;
    pocket::Filter246Effect outputFilter;
    pocket::Phase246Config filterConfig;
    pocket::Tape tape;
    std::atomic<bool> panicRequested{false},panicOutputRequested{false};
    pocket::HeldNotes heldNotes;
    double rate=48000, clock=0;
    int previousSeqMode=-1;
    int step=0, seqNote=-1, scopeCounter=0, scopeIndex=0;
    bool wasSequencing=false;
    int64_t lastHostStep=INT64_MIN;
    float outputGain=.7f,outputSmoothing=.002f;
    std::vector<float> inputCapture;
    int captureWrite=0;
    double captureRate=48000;
    float clickDecay=.99f;
    float eqLowState=0,eqHighState=0,eqLowCoefficient=0,eqHighCoefficient=0;
    std::array<float,3> eqGains{{1,1,1}};
    std::array<float,192000> masterDelay{};size_t masterDelayWrite=0,masterDelayValid=0;
    void resetOutputHistory() noexcept {outputFilter.reset();masterDelayWrite=masterDelayValid=0;eqLowState=eqHighState=0;clickEnvelope=0;}
    float masterDriveSmooth=0,masterEchoSmooth=0,masterTimeSmooth=.281f,eqMixSmooth=0;
    double clickBeat=0,clickPhase=0;float clickEnvelope=0;int64_t lastClickBeat=-1;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PocketProcessor)
};
