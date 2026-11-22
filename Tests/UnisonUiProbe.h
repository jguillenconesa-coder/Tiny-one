// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <functional>
#include <iostream>
// Wait for the actual posted click and parameter update, not a fixed delay.
// AppKit uses a native peer so this is a real window-backed event loop on macOS.
inline bool checkUnisonUi(juce::AudioProcessorEditor& editor,juce::TextButton& button,
                         juce::AudioProcessorValueTreeState& state,const char* id,
                         const char* capturePath=nullptr) {
    editor.setVisible(true);
   #if JUCE_MAC
    constexpr bool nativePeerRequired=true;
   #else
    constexpr bool nativePeerRequired=false;
   #endif
    if(capturePath!=nullptr||nativePeerRequired){editor.addToDesktop(juce::ComponentPeer::windowHasTitleBar);editor.toFront(true);}
    auto* mode=editor.findChildWithID("hybrid-voice-mode");auto* glide=editor.findChildWithID("hybrid-glide");
    bool clicked=false,passed=false;auto previous=button.onClick;
    button.onClick=[&]{clicked=true;if(previous)previous();};
    struct Poll:juce::Timer {std::function<void()> tick;void timerCallback()override{tick();}} poll;
    const auto start=juce::Time::getMillisecondCounterHiRes();
    poll.tick=[&]{
        const bool ready=mode&&glide&&!mode->isEnabled()&&glide->isEnabled()&&clicked&&button.isVisible()&&button.getToggleState()&&state.getRawParameterValue(id)->load()>.5f;
        if(ready){passed=true;if(capturePath){juce::FileOutputStream stream(juce::File(juce::String::fromUTF8(capturePath)));juce::PNGImageFormat png;passed=png.writeImageToStream(editor.createComponentSnapshot(editor.getLocalBounds()),stream);}}
        const bool expired=juce::Time::getMillisecondCounterHiRes()-start>5000;
        if(ready||expired){if(!passed)std::cerr<<"Unison UI: clicked="<<clicked<<" visible="<<button.isVisible()<<" toggle="<<button.getToggleState()<<" modeEnabled="<<(mode?mode->isEnabled():false)<<" glideEnabled="<<(glide?glide->isEnabled():false)<<" value="<<state.getRawParameterValue(id)->load()<<'\n';poll.stopTimer();juce::MessageManager::getInstance()->stopDispatchLoop();}
    };
    poll.startTimer(20);juce::Component::SafePointer<juce::TextButton> safe(&button);juce::MessageManager::callAsync([safe]{if(safe)safe->triggerClick();});
    juce::MessageManager::getInstance()->runDispatchLoop();poll.stopTimer();button.onClick=std::move(previous);return passed;
}
