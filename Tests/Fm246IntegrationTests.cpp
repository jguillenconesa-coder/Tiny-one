// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
    auto engine=std::make_unique<pocket::Engine>();
    // Fractional pitch: independently estimate the actual audio period with
    // interpolated positive crossings, across the playable bass/treble range.
    double worstCents=0;
    for(double rate:{44100.,48000.,96000.})for(int note:{24,36,48,60,69,81,93,105})for(float bend:{-2.f,0.f,2.f}){
        pocket::Parameters p;p.effect=0;p.volume=1;p.fm246Controls[0]=0;p.fm246Adsr={0,64,32767,16383};
        engine->prepare(rate);engine->next(p);engine->pitchBend(1,bend);engine->noteOn(note,1);
        double first=-1,last=-1;int crossings=0;float previous=0;
        for(int i=0;i<int(rate*.65);++i){const auto x=engine->next(p);if(i>rate*.1&&x>0&&previous<=0){const double crossing=i-double(x)/(x-previous);if(first<0)first=crossing;last=crossing;++crossings;}previous=x;}
        check(crossings>2,"Insufficient FM periods for tuning measurement");
        const double actual=(crossings-1)*rate/(last-first),expected=440*std::exp2((note-69+bend)/12.);
        const double cents=std::abs(1200*std::log2(actual/expected));worstCents=std::max(worstCents,cents);
        check(cents<.15,"Fractional FM pitch/bend deviates by more than .15 cents");
    }
    // Tremolo pitch and amplitude must share the same block epoch. Mid-block
    // activation waits for the following core block, rather than advancing just
    // the amplitude side while buffered audio still uses old pitch.
    {
        pocket::Phase246Config rate;rate.prepare(48000);pocket::Fm246Config config;
        pocket::Fm246Voice voice;const std::array<int16_t,4> env{{0,64,32767,8192}};
        for(int n=0;n<37;++n)voice.source(440,pocket::fm246Defaults,env,config,rate,false);
        voice.output.tremolo.configure(true,{12000,-12000,18000,16000},0,pocket::reference::value246Tempo(120),true);
        for(int n=37;n<128;++n)voice.source(440,pocket::fm246Defaults,env,config,rate,false);
        check(!voice.output.tremolo.active,"Tremolo began midway through a cached FM block");
        for(int n=0;n<128;++n)voice.source(440,pocket::fm246Defaults,env,config,rate,false);
        check(voice.output.tremolo.active&&voice.output.tremolo.frame==128&&voice.output.tremolo.state.pitchPhase==voice.output.tremolo.state.volumePhase,"FM Tremolo pitch/amplitude clocks diverged");
    }
    for(double rate:{44100.,48000.,96000.})for(bool envelope:{false,true})for(int colour=0;colour<4;++colour){
        auto render=[&](bool enabled,int amount){
            pocket::Parameters p;p.effect=0;p.volume=1;p.fm246Controls={12000,10485,7168,12000};p.fm246Adsr={2000,8000,16000,10000};
            p.valueVoice246=enabled;p.valueVoice246Envelope=envelope;p.valueVoice246Colour=colour;p.valueVoice246Amount=amount;p.valueVoice246Speed=12288;
            engine->prepare(rate);engine->next(p);engine->noteOn(57,.8f);std::vector<float> result;
            for(int n=0;n<int(rate*.45);++n){if(n==int(rate*.2))engine->noteOff(57);const auto x=engine->next(p);check(std::isfinite(x),"FM Value produced nonfinite audio");result.push_back(x);}
            engine->reset();for(int n=0;n<128;++n)check(engine->next(p)==0,"FM Value survived panic");return result;
        };
        const auto baseline=render(false,0),neutral=render(true,0),modulated=render(true,24000);
        check(baseline==neutral,"Zero-amount FM Value changes valid controls");
        double difference=0;for(size_t i=0;i<baseline.size();++i)difference+=std::abs(baseline[i]-modulated[i]);
        check(difference>.001,"FM Value Instrument/ADSR colour is inaudible");
    }
    for(double rate:{44100.,48000.,96000.}){
        pocket::Parameters p;p.effect=0;p.volume=1;p.fm246Adsr={0,1000,32767,16383};
        std::array<double,9> energies{};
        for(int algorithm=0;algorithm<9;++algorithm){
            engine->prepare(rate);p.fm246Controls={14000,10485,int16_t(1024+algorithm*2048),20000};
            p.fm246Controls[2]=std::min<int16_t>(17408,p.fm246Controls[2]);
            for(int i=0;i<128;++i)check(engine->next(p)==0,"Idle FM is not silent");
            engine->noteOn(69,.8f);
            for(int i=0;i<int(rate/4);++i){const auto x=engine->next(p);check(std::isfinite(x),"FM produced nonfinite audio");energies[size_t(algorithm)]+=double(x)*x;}
            check(energies[size_t(algorithm)]>.001,"FM routing produced no audio");
            engine->noteOff(69);for(int i=0;i<int(rate*2);++i)engine->next(p);
            check(engine->visualState(0).active==0,"FM release did not retire voice");
        }
        check(std::abs(energies[0]-energies[8])>.01,"FM topology has no audible effect");
        engine->prepare(rate);engine->next(p);
        for(int note=60;note<68;++note)engine->noteOn(note,1);
        check(engine->visualState(0).active==6,"FM pool must have six voices");
        for(int i=0;i<256;++i){p.fm246Controls[2]=int16_t(1024+(i%9)*2048);p.fm246Controls[2]=std::min<int16_t>(17408,p.fm246Controls[2]);check(std::isfinite(engine->next(p)),"Live topology switch failed");}
        engine->reset();for(int i=0;i<128;++i)check(engine->next(p)==0,"FM panic retained audio");
        p.hybridVoiceMode=2;p.hybridGlideMs=80;engine->next(p);engine->noteOn(60,1);
        for(int i=0;i<500;++i)engine->next(p);engine->noteOn(72,1);
        for(int i=0;i<500;++i)engine->next(p);
        check(engine->visualState(0).active==1&&engine->visualState(0).note==72,"FM legato priority failed");
        engine->noteOff(72);for(int i=0;i<200;++i)engine->next(p);
        check(engine->visualState(0).note==60,"FM legato held-note return failed");
        engine->allNotesOff();for(int i=0;i<int(rate*2);++i)engine->next(p);
        check(engine->visualState(0).active==0,"FM mono release stuck");
    }
    std::cout<<"PASS: FM tuning/bend worst error "<<worstCents<<" cents; aligned Tremolo, nine routings, three host rates, six voices, release/panic/legato/glide\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
