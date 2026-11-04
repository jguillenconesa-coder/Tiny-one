// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <cstdint>
namespace pocket {
// Immutable, bounded UI snapshot. No audio buffers or processor locks are accessed by drawing.
struct ScreenFrame {
    bool drumLfoSecondary=false;bool samplerSecondary=false,samplerLoop=false;int samplerFrames=0;std::array<int16_t,8> samplerControls{};
    bool sampler246=false,drum246=false,dsynthSecondary=false,cluster246=false,drWave246=false,phase246=false,phaseEnvelope=false,digital246=false,cwo246=false,spring246=false,phone246=false,grid246=false,delay246=false,nitro246=false,filter246=false,punch246=false,value246=false,valueDestination=false;
    int dboxPad=0;int valueColour=0;
    bool element246=false,elementTargetAvailable=false;float elementLevel=0;
    bool crank246=false,crankTargetAvailable=false;float crankPhase=0;
    bool bend246=false,bendTargetAvailable=false;float bendLevel=0;
    bool midi246=false;std::array<float,4> midiCC{{64,64,64,64}};std::array<juce::String,4> midiTargets{};
    bool random246=false,randomEnvelope=false;std::array<float,4> random{};
    bool valueVoice=false,valueEnvelope=false,tremolo246=false;
    int kitPads=8,fmAlgorithm=0;
    int page=0,engine=0,note=60,voices=0,stage=0,step=-1,mode=0,track=0;
    float time=0,level=0,envelope=0,samplePosition=0,lfoPhase=0,tapePosition=0,tapeLength=8;
    float tone=.5f,color=.5f,attack=.02f,decay=.2f,sustain=.8f,release=.3f,lfoRate=1;
    float feedback=.42f,delay=.281f;int lfoShape=0;
    std::array<float,4> values{},trackGains{};
    std::array<juce::String,4> labels{};
    std::array<float,128> sample{};
};
void drawScreen(juce::Graphics&,const ScreenFrame&);
}
