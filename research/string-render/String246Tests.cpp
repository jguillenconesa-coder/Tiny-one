// SPDX-License-Identifier: AGPL-3.0-only
#include "String246.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing String fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto x=uint32_t(p[0])|(uint32_t(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
    check(argc==2,"Provide String fixture directory");using namespace pocket::reference;const std::filesystem::path root(argv[1]);
    const auto in=read(root/"string246-input.bin"),out=read(root/"string246-output.bin");
    check(in.size()==4096*64&&out.size()==868+4096*328+16*49152,"Wrong String fixture sizes");
    for(size_t i=0;i<129;++i)check(spring246Low[i]==half(out.data()+i*2),"String low table mismatch");
    for(size_t i=0;i<225;++i)check(delay246Interpolation[i]==half(out.data()+258+i*2),"String interpolation mismatch");
    for(size_t i=0;i<24;++i)check(uint32_t(cluster246Damping[i])==word(out.data()+708+i*4),"String excitation decay mismatch");
    String246State state;String246Memory memory{};uint32_t rng=0;size_t at=868,mismatches=0;
    for(size_t n=0;n<4096;++n){const auto* x=in.data()+n*64;const auto* y=out.data()+at;const auto action=word(x+48);
        if(action&1){std::memcpy(&state,x,32);rng=word(x+44);uint32_t seed=word(x+52);for(size_t i=0;i<memory.size();i+=2){seed=seed*1664525u+1013904223u;memory[i]=half(reinterpret_cast<const unsigned char*>(&seed));memory[i+1]=half(reinterpret_cast<const unsigned char*>(&seed)+2);}}
        if(action&8){state.construct();state.pointer=0x010225ec;}
        if(action&2)string246Reset(state,rng);
        if(action&4)state.release();
        if(std::memcmp(&state,y,32)){if(mismatches++<10)std::cerr<<"lifecycle "<<n<<'\n';}
        const auto pitch=word(x+40);const auto expected=uint32_t(std::clamp(signedPhase(word(y+324)),int32_t(0x20000),int32_t(0x60000000)));check(string246Period(pitch)==expected,"String pitch division mismatch");
        const auto period=string246Period(pitch);if(state.cursor>=0x60000000u){std::cerr<<"domain case "<<n<<" cursor "<<state.cursor<<" period "<<period<<'\n';throw std::runtime_error("Out-of-domain cursor");}
        std::array<int16_t,4> controls{};for(size_t i=0;i<4;++i)controls[i]=half(x+32+i*2);std::array<int16_t,128> audio{};
        check(string246Block(state,memory,controls,pitch,rng,audio),"String domain rejected");
        std::array<unsigned char,292> actual{};std::memcpy(actual.data(),&state,32);std::memcpy(actual.data()+32,audio.data(),256);std::memcpy(actual.data()+288,&rng,4);
        if(std::memcmp(actual.data(),y+32,292)){if(mismatches++<15){std::cerr<<"render "<<n;for(size_t i=0;i<292;++i)if(actual[i]!=y[32+i]){std::cerr<<" byte "<<i<<" got "<<int(actual[i])<<" expected "<<int(y[32+i]);break;}std::cerr<<'\n';}}
        at+=328;
        if(n%256==255){if(std::memcmp(memory.data(),out.data()+at,49152)){if(mismatches++<15)std::cerr<<"memory "<<n<<'\n';}at+=49152;}
    }
    check(!mismatches,"String lifecycle/render/memory mismatch");std::cout<<"PASS: 4096 String blocks / 524288 samples, lifecycle/full states/RNG and 16 complete workspaces\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
