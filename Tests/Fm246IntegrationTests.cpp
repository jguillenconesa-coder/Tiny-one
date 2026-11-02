// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
    auto engine=std::make_unique<pocket::Engine>();
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
    std::cout<<"PASS: nine FM routings at three host rates, six voices, live switching, release, panic, legato/glide\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
