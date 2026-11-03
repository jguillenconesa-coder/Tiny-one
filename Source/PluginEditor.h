// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "PluginProcessor.h"
#include "SoundBrowser.h"
#include "KitEditor.h"
class PocketLook : public juce::LookAndFeel_V4 {
public:
    void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};
class HardwareKeyboard : public juce::MidiKeyboardComponent {
public:
    explicit HardwareKeyboard(juce::MidiKeyboardState& s) : MidiKeyboardComponent(s,horizontalKeyboard) {}
    void drawWhiteNote(int,juce::Graphics&,juce::Rectangle<float>,bool,bool,juce::Colour,juce::Colour) override;
    void drawBlackNote(int,juce::Graphics&,juce::Rectangle<float>,bool,bool,juce::Colour) override;
    bool mouseDownOnKey(int,const juce::MouseEvent&) override;
    bool mouseDraggedToKey(int,const juce::MouseEvent&) override;
};
class PocketEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit PocketEditor(PocketProcessor&);
    ~PocketEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress&) override;
    int getControlParameterIndex(juce::Component&) override;
private:
    void timerCallback() override;
    void selectPage(int);
    void chooseSample();
    void chooseSamples();
    void showBrowser(int kind=1);
    void showError(const juce::String&);
    void doTapeEdit(const juce::String&);
    void showComMenu();
    void showSequenceMenu();
    void applyTapeTrick(int);
    bool shifted() const;
    void setParameter(const char*,float);
    void chooseBank();
    void refreshPresetMenu();
    void chooseExport();
    void chooseSession(bool save);
    void setOctave(int);
    void selectMode(int);
    void applySoundSlot(int);
    PocketProcessor& processor;
    PocketLook look;
    SoundBrowser browser;
    KitEditor kitEditor;
    juce::TextButton kitEditKey{"KIT EDIT"};
    bool kitOpen=false;
    juce::String lastSelectedSound;
    bool browserOpen=true,showSequence=false;
    juce::TooltipWindow tooltip{this,650};
    int page=0,octave=0,mainMode=0;

    std::array<juce::TextButton,4> modeButtons;
    std::array<juce::TextButton,8> slotButtons;
    juce::TextButton browseKey{"BROWSE"},splitKey{"SPLIT"},liftKey{"LIFT"},dropKey{"DROP"},undoKey{"UNDO"},sequencerKey{"SEQ"},comKey{"COM"},micKey{"IN"},metroKey{"CLICK"};
    juce::TextButton phase246{"PHASE 246 CORE"};bool lastDsynth246=false,lastDsynthBank=false;juce::TextButton dsynthBank{"OSC II"};std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dsynthBankAttachment;bool lastVoltage246=false,lastDna246=false,lastString246=false;bool lastCluster246=false,lastDrWave246=false,lastPhase246=false,lastPhaseEnvelope=false,lastDigital246=false,lastDigitalEnvelope=false,lastCwo246=false,lastSpring246=false,lastPhone246=false,lastGrid246=false,lastDelay246=false,lastNitro246=false,lastFilter246=false,lastPunch246=false,lastValue246=false,lastValueVoice246=false;
    juce::TextButton phaseEnvelope{"246 ENVELOPE"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseAttachment,envelopeAttachment,digitalAttachment,digitalEnvelopeAttachment,valueAttachment,valueFreeAttachment,valueVoiceAttachment,valueVoiceFreeAttachment;
    juce::TextButton sampler246{"SAMPLER 246"},samplerReverse{"REVERSE"},drumLfoBank{"I / II"},samplerSecondary{"FINE / GAIN"},samplerEnvelope{"246 ENVELOPE"};bool lastSampler246=false,lastSamplerSecondary=false,lastSamplerEnvelope=false;std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> samplerAttachment,samplerReverseAttachment,drumLfoBankAttachment,samplerSecondaryAttachment,samplerEnvelopeAttachment;
    juce::TextButton drum246{"SAMPLER 246"};juce::ComboBox drumPad;bool lastDrum246=false;int lastDrumPad=-1;std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> drumAttachment;std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> drumPadAttachment;
    juce::ComboBox dboxPad;bool lastDbox246=false;int lastDboxPad=-1;std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> dboxPadAttachment;
    juce::ComboBox outputFx;
    void syncOutputFx();
    juce::TextButton value246{"VALUE 246 FX"},valueFree{"FREE PHASE"};
    bool lastRandom246=false;juce::TextButton random246{"RANDOM 246"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> randomAttachment;
    bool lastTremolo246=false;
    juce::TextButton tremolo246{"TREMOLO 246"};juce::ComboBox tremoloShape;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> tremoloAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> tremoloShapeAttachment;
    juce::TextButton valueVoice246{"VALUE 246 VOICE"},valueVoiceFree{"FREE PHASE"};
    juce::TextButton digital246{"DIGITAL 246 CORE"},digitalEnvelope{"246 ENVELOPE"};
    juce::TextButton cleanSample{"CLEAN SAMPLE"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> cleanAttachment;
    juce::ComboBox hybridVoiceMode;
    juce::Slider hybridGlide;
    juce::Label hybridGlideLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> hybridModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hybridGlideAttachment;
    juce::Label status;
    juce::TextButton sequenceSettings{"SEQ SETTINGS"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> metroAttachment;
    juce::TextButton shiftKey{"shift"},helpKey{"?"},tapTempoKey{"TAP"};
    double lastTempoTap=0,scrubStart=0;bool slidingTake=false;
    std::array<juce::String,4> knobLabels;
    std::array<juce::TextButton,5> pageButtons;
    juce::ComboBox presets,tapeTrack,tapeLength;
    juce::TextButton importBank{"BANK"};
    juce::TextButton saveSession{"SAVE"},loadSession{"OPEN"};
    juce::TextButton loadSample{"LOAD"},loopSample{"LOOP"},tapePlay{"PLAY"},tapeRecord{"REC"},overdub{"OVER"},rewind{"|<"},clear{"CLEAR"},exportTape{"EXPORT"},octaveDown{"OCT -"},octaveUp{"OCT +"};
    juce::Slider tapeSpeed;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> trackAttachment,lengthAttachment;
    std::unique_ptr<juce::FileChooser> chooser;
    std::array<juce::Slider,4> knobs;
    std::array<std::unique_ptr<juce::SliderParameterAttachment>,4> attachments;
    juce::Slider volume, tempo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment, tempoAttachment;
    juce::ComboBox engine;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> engineAttachment;
    juce::TextButton sequence{"PLAY"}, panic{"STOP"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sequenceAttachment;
    std::array<juce::TextButton,16> stepButtons;
    HardwareKeyboard keyboard;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PocketEditor)
};
