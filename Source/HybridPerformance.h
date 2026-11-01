// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace pocket {
// Authored performance policy, not the protected rev246 voice scheduler.
// Channel 17 is the internal sequencer, independent of MIDI channels 1-16.
struct HybridHeldNotes {
    struct Note {uint64_t order=0;float velocity=0;bool down=false;};
    std::array<Note,17*128> notes{};
    uint64_t order=0;
    int current=-1;
    static int index(int note,int channel) noexcept {return (channel-1)*128+note;}
    void clear() noexcept {notes={};order=0;current=-1;}
    void press(int note,int channel,float velocity) noexcept {
        current=index(note,channel);notes[size_t(current)]={++order,velocity,true};
    }
    void release(int note,int channel,bool pedal) noexcept {
        auto& n=notes[size_t(index(note,channel))];n.down=false;if(!pedal)n.order=0;
    }
    void releasePedal(int channel) noexcept {
        for(int note=0;note<128;++note){auto& n=notes[size_t(index(note,channel))];if(!n.down)n.order=0;}
    }
    void removeChannel(int channel) noexcept {
        for(int note=0;note<128;++note)notes[size_t(index(note,channel))]={};
    }
    int latest() const noexcept {
        int result=-1;uint64_t newest=0;
        for(size_t i=0;i<notes.size();++i)if(notes[i].order>newest){result=int(i);newest=notes[i].order;}
        return result;
    }
};
struct HybridGlide {
    double value=440,target=440,multiplier=1;
    unsigned remaining=0;
    void start(double from,double to,double milliseconds,double sampleRate) noexcept {
        target=to;remaining=unsigned(std::lround(milliseconds*.001*sampleRate));
        value=remaining?from:to;multiplier=remaining?std::pow(to/from,1./remaining):1;
    }
    double next() noexcept {
        // First sample retains the old pitch; snap exactly after N host samples.
        const double result=value;
        if(remaining){if(--remaining)value*=multiplier;else value=target;}
        return result;
    }
};
}
