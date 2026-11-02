// SPDX-License-Identifier: AGPL-3.0-only
#include "ValueVoice246.h"
#include "Random246.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing FM modulation fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const unsigned x=p[0]|(unsigned(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
    check(argc==2,"Provide fixture directory");using namespace pocket::reference;const std::filesystem::path root(argv[1]);
    const auto sineBytes=read(root/"verified-sine.bin");check(sineBytes.size()==16386,"Incomplete sine fixture");
    std::array<int16_t,8192> sine{};for(size_t i=0;i<8192;++i)sine[i]=half(sineBytes.data()+i*2);
    {
        const auto in=read(root/"value-fm246-input.bin"),out=read(root/"value-fm246-output.bin");
        check(in.size()==4096*96&&out.size()==152+4096*68,"Wrong FM Value fixtures");
        for(size_t i=0;i<8;++i)for(size_t j=0;j<4;++j)check(valueVoice246Ranges[4][i][j]==half(out.data()+i*8+j*2),"FM ranges differ from original initializer");
        check(std::memcmp(out.data(),out.data()+64,64)==0,"FM range copies differ");
        for(size_t i=0;i<4;++i)for(size_t j=0;j<3;++j)check(valueVoice246EnvelopeRanges[i][j]==half(out.data()+128+i*6+j*2),"FM ADSR range mismatch");
        Value246State state;std::array<uint32_t,8> display{};
        for(size_t n=0;n<4096;++n){const auto* x=in.data()+n*96;const auto* y=out.data()+152+n*68;
            const auto action=word(x+88);if(action){std::memcpy(&state,x,12);std::memcpy(display.data(),x+12,32);}
            check(word(x+84)==0,"Fixture is not FM");
            std::array<int16_t,8> controls{},targets{};for(size_t i=0;i<8;++i){controls[i]=half(x+44+i*2);targets[i]=half(x+60+i*2);}
            if(action==2)state.noteOn(controls);
            const uint32_t result=valueVoice246Step(state,display,controls,word(x+76),int(word(x+80)),0,targets,sine)?1u:0u;
            float bpm;std::memcpy(&bpm,x+92,4);const uint32_t tempo=value246Tempo(bpm);
            std::array<unsigned char,68> actual{};std::memcpy(actual.data(),&state,12);std::memcpy(actual.data()+12,display.data(),32);std::memcpy(actual.data()+44,targets.data(),16);std::memcpy(actual.data()+60,&result,4);std::memcpy(actual.data()+64,&tempo,4);
            if(std::memcmp(actual.data(),y,68)){std::cerr<<"Value FM case "<<n<<'\n';throw std::runtime_error("FM Value state/display/targets/return/tempo mismatch");}
        }
    }
    {
        const auto in=read(root/"random-fm246-input.bin"),out=read(root/"random-fm246-output.bin");
        check(in.size()==4096*88&&out.size()==4096*72,"Wrong FM Random fixtures");
        Random246State state;uint32_t rng=0,meter=0;
        for(size_t n=0;n<4096;++n){const auto* x=in.data()+n*88;const auto* y=out.data()+n*72;const auto meta=word(x+84);check((meta>>16)==0,"Random fixture is not FM");
            if(meta&255){std::memcpy(&state,x,44);rng=word(x+44);meter=word(x+48);}
            std::array<int16_t,8> controls{},targets{};for(size_t i=0;i<8;++i){controls[i]=half(x+52+i*2);targets[i]=half(x+68+i*2);}
            if((meta&255)==2)state.noteOn(controls[3]);
            const uint32_t result=random246VoiceStep(state,rng,meter,controls,int((meta>>8)&255),0,targets)?1u:0u;
            std::array<unsigned char,72> actual{};std::memcpy(actual.data(),&state,44);std::memcpy(actual.data()+44,&rng,4);std::memcpy(actual.data()+48,&meter,4);std::memcpy(actual.data()+52,targets.data(),16);std::memcpy(actual.data()+68,&result,4);
            if(std::memcmp(actual.data(),y,72)){std::cerr<<"Random FM case "<<n<<'\n';throw std::runtime_error("FM Random state/RNG/meter/targets/return mismatch");}
        }
    }
    std::cout<<"PASS: FM original ranges, 4096 Value and 4096 Random Instrument/ADSR callbacks, full state/display/RNG/targets/returns\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
