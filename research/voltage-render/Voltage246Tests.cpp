// SPDX-License-Identifier: AGPL-3.0-only
#include "Voltage246.h"
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Voltage fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const unsigned x=p[0]|(unsigned(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
    check(argc==2,"Provide Voltage fixture directory");using namespace pocket::reference;const std::filesystem::path root(argv[1]);
    const auto in=read(root/"voltage246-input.bin"),out=read(root/"voltage246-output.bin");
    check(in.size()==4096*840&&out.size()==324+4096*1472,"Wrong Voltage fixture size");
    for(size_t i=0;i<129;++i)check(spring246Low[i]==half(out.data()+i*2),"Voltage filter table differs from original conversion");
    std::array<int16_t,8192> sine{};for(size_t i=0;i<8192;++i)sine[i]=int16_t(std::lround(std::sin(double(i)*3.14159265358979323846/4096)*32767));
    Voltage246Pool pool{};int errors=0;
    for(size_t n=0;n<4096;++n){const auto* x=in.data()+n*840;const auto* y=out.data()+324+n*1472;
        const auto action=word(x+320),slot=word(x+324);check(slot<6,"Invalid test slot");
        if(action&1)std::memcpy(pool.data(),x,312);
        auto& s=pool[slot];if(action&8)s.construct();if(action&2)s.reset();if(action&4)s.release();
        std::array<unsigned char,1472> actual{};std::memcpy(actual.data(),pool.data(),312);
        std::array<int16_t,4> c{};for(size_t i=0;i<4;++i)c[i]=half(x+312+i*2);
        std::array<uint32_t,128> pitch{};for(size_t i=0;i<128;++i)pitch[i]=word(x+328+i*4);
        std::array<int16_t,128> audio{};std::array<int16_t,32> display{};
        voltage246Block(s,c,pitch,sine,audio,display);
        std::memcpy(actual.data()+312,pool.data(),312);std::memcpy(actual.data()+624,audio.data(),256);std::memcpy(actual.data()+880,display.data(),64);std::memcpy(actual.data()+944,pitch.data(),512);
        const uint32_t guards[]={0x13579bdf,0x2468ace0,0x789abcde,0x89abcdef};std::memcpy(actual.data()+1456,guards,16);
        if(std::memcmp(actual.data(),y,1472)){if(errors++<15){std::cerr<<"case "<<n<<" slot "<<slot;for(size_t i=0;i<1472;++i)if(actual[i]!=y[i]){std::cerr<<" byte "<<i<<" got "<<int(actual[i])<<" expected "<<int(y[i]);break;}std::cerr<<'\n';}}
    }
    check(!errors,"Voltage pool/audio/display/pitch/guards differ");
    std::cout<<"PASS: Voltage 4096 blocks / 524288 samples, six complete states before/after lifecycle, display/pitch/guards and 129 coefficients\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
