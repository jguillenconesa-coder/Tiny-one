// SPDX-License-Identifier: AGPL-3.0-only
#include "ValueVoice246.h"
#include "Value246Fx.h"
#include "ValueVoice246Control.h"
#include "Engine.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
std::vector<uint8_t> read(const std::string& path){std::ifstream f(path,std::ios::binary);check(bool(f),"Missing Value fixture");return {std::istreambuf_iterator<char>(f),{}};}
uint32_t u32(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
int16_t s16(const uint8_t* p){const auto x=uint32_t(p[0])|(uint32_t(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
    check(argc==2,"Expected vector directory");const std::string dir=argv[1];
    const auto in=read(dir+"/value-hybrid246-input.bin"),expected=read(dir+"/value-hybrid246-output.bin"),table=read(dir+"/verified-sine.bin");
    check(in.size()==4096*96&&expected.size()==152+4096*68,"Incomplete Value oracle");
    for(size_t t=0;t<2;++t)for(size_t c=0;c<8;++c)for(size_t j=0;j<4;++j)
        check(valueVoice246Ranges[t+2][c][j]==s16(expected.data()+t*64+c*8+j*2),"Voice range differs from original initializer");
    for(size_t c=0;c<4;++c)for(size_t j=0;j<3;++j)check(valueVoice246EnvelopeRanges[c][j]==s16(expected.data()+128+c*6+j*2),"ENV range differs from original initializer");
    std::array<int16_t,8192> sine{};for(size_t i=0;i<sine.size();++i)sine[i]=s16(table.data()+i*2);
    Value246State state;std::array<uint32_t,8> display{};size_t failures=0;
    for(size_t i=0;i<4096;++i){const auto* p=in.data()+i*96;const auto* e=expected.data()+152+i*68;
        const auto action=u32(p+88);
        if(action){state={u32(p),u32(p+4),uint16_t(s16(p+8)),uint16_t(s16(p+10))};for(size_t j=0;j<8;++j)display[j]=u32(p+12+j*4);}
        std::array<int16_t,8> controls{},targets{};for(size_t j=0;j<8;++j){controls[j]=s16(p+44+j*2);targets[j]=s16(p+60+j*2);}
        if(action==2)state.noteOn(controls);
        const auto result=valueVoice246Step(state,display,controls,u32(p+76),int(u32(p+80)),int(u32(p+84)),targets,sine);
        std::array<uint32_t,17> actual{};actual[0]=state.tag;actual[1]=state.phase;actual[2]=state.word|(uint32_t(state.padding)<<16);
        for(size_t j=0;j<8;++j)actual[j+3]=display[j];
        for(size_t j=0;j<4;++j)actual[j+11]=uint16_t(targets[j*2])|(uint32_t(uint16_t(targets[j*2+1]))<<16);
        actual[15]=result?1:0;float bpm=0;std::memcpy(&bpm,p+92,4);actual[16]=value246Tempo(bpm);
        for(size_t j=0;j<actual.size();++j)if(actual[j]!=u32(e+j*4)){
            if(failures++<12)std::cerr<<"case "<<i<<" word "<<j<<" got "<<std::hex<<actual[j]<<" expected "<<u32(e+j*4)<<std::dec<<'\n';
        }
        if(failures>100)break;
    }
    check(failures==0,"Value instrument/ENV reconstruction differs from original instruction oracle");
    for(size_t c=0;c<4;++c)check(pocket::drWave246Max[c]==valueVoice246Ranges[2][c][1],"Dr. Wave host maximum differs from observed range");
    // Clock/retrigger/FREE and inactive behavior for both new families at 7 rates.
    for(int type:{2,4})for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
        pocket::ValueVoice246Control control;const std::array<int16_t,4> base=type==2?std::array<int16_t,4>{{12000,8000,6000,16000}}:std::array<int16_t,4>{{8192,14000,12000,400}};const auto step=44100./(64*rate);
        for(int i=0;i<int(rate);++i)control.next(true,false,true,12288,24000,0,type,value246Tempo(120),step,base,sine);
        check(std::abs(double(control.ticks)-44100./64)<=1,"Hybrid Voice clock drift");const auto phase=control.state.phase;control.noteOn();control.next(true,false,true,12288,24000,0,type,value246Tempo(120),step,base,sine);check(control.state.phase==phase||control.state.phase==phase+value246Increment(12288,value246Tempo(120)),"Hybrid FREE restarted phase");control.noteOn();control.next(true,false,false,12288,24000,0,type,value246Tempo(120),step,base,sine);check(control.state.phase==0x80000000u||control.state.phase==0x80000000u+value246Increment(12288,value246Tempo(120)),"Hybrid note did not reset phase");check(control.next(false,false,false,0,0,0,type,value246Tempo(120),step,base,sine)==base,"Disabled hybrid Value changed base");
    }
    // Per-voice integration: modulation reaches every colour/ADSR without writing base.
    for(int mode:{6,7})for(bool envelope:{false,true})for(int colour=0;colour<4;++colour){
        auto render=[&](int amount){auto engine=std::make_unique<pocket::Engine>();pocket::Parameters p;p.engine=mode;p.effect=0;p.volume=1;p.valueVoice246=true;p.valueVoice246Envelope=envelope;p.valueVoice246Speed=12288;p.valueVoice246Amount=amount;p.valueVoice246Colour=colour;p.value246Tempo=value246Tempo(120);p.drWave246Controls={12000,8000,6000,16000};p.cluster246Controls={8192,14000,12000,400};p.drWave246Adsr=p.cluster246Adsr={4000,8000,16000,12000};engine->prepare(48000);engine->selectEngine(mode);for(int note:{48,52,55,59,60,64,67})engine->noteOn(note,.8f);std::vector<float> result;for(int i=0;i<24000;++i){if(i==12000)engine->allNotesOff();auto x=engine->next(p);check(std::isfinite(x)&&std::abs(x)<2,"Hybrid Voice unstable");result.push_back(x);}engine->reset();for(int i=0;i<256;++i)check(engine->next(p)==0,"Hybrid Voice survived panic");return result;};
        const auto base=render(0);for(int amount:{-32767,32767}){auto altered=render(amount);double delta=0;for(size_t i=0;i<base.size();++i)delta+=std::abs(base[i]-altered[i]);check(delta>.01,"Hybrid Voice target is inaudible");}
    }
    std::cout<<"PASS: 4096 Dr. Wave/Cluster Value instrument/ENV states, displays, buffers, ranges, returns and tempo; 14 clock cases and 48 audio target workloads\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
