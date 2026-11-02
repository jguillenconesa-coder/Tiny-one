// SPDX-License-Identifier: AGPL-3.0-only
#include "Fm246Pitch.h"
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static std::vector<unsigned char> read(const std::filesystem::path& path){std::ifstream f(path,std::ios::binary);check(bool(f),"Missing FM render fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto x=uint16_t(p[0])|(uint16_t(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
    check(argc==2,"Provide FM render fixture directory");const std::filesystem::path root(argv[1]);
    const auto in=read(root/"fm246-render-input.bin"),out=read(root/"fm246-render-output.bin");
    check(in.size()==4096*844&&out.size()==4096*896,"Wrong FM render fixture sizes");
    std::array<int16_t,8192> sine{};for(size_t i=0;i<sine.size();++i)sine[i]=int16_t(std::lround(std::sin(double(i)*3.14159265358979323846/4096)*32767));
    const auto table=pocket::reference::fm246ConvertTables(pocket::reference::fm246ModelBits);
    pocket::reference::Fm246State state;pocket::reference::Fm246Workspace work;
    std::array<int16_t,128> audio{};int mismatches=0;std::array<int,10> coverage{};
    for(size_t n=0;n<4096;++n){
        const auto* x=in.data()+n*844;const auto* y=out.data()+n*896;
        if(word(x+72)){std::memcpy(&state,x,60);std::memcpy(&work,x+76,512);}
        std::memcpy(audio.data(),x+588,256);
        const std::array<int16_t,4> controls{{half(x+60),half(x+62),half(x+64),half(x+66)}};
        const auto fields=pocket::reference::decodeFmFields(uint16_t(controls[1]),uint16_t(controls[2]));
        ++coverage[size_t(fields.selector>=0&&fields.selector<9?fields.selector:9)];
        auto hybridState=state;auto hybridWork=work;auto hybridAudio=audio;
        pocket::fm246HybridBlock(hybridState,hybridWork,controls,word(x+68)&0xffff0000u,table,sine,hybridAudio);
        pocket::reference::fm246Block(state,work,controls,word(x+68),table,sine,audio);
        check(std::memcmp(&state,&hybridState,sizeof state)==0&&std::memcmp(&work,&hybridWork,sizeof work)==0&&audio==hybridAudio,"Integer pitch extension diverges from original");
        std::array<unsigned char,896> actual{};
        std::memcpy(actual.data(),&state,60);std::memcpy(actual.data()+60,table.data(),60);
        std::memcpy(actual.data()+120,&work,512);std::memcpy(actual.data()+632,audio.data(),256);
        const uint32_t guards[]={0x13579bdf,0x2468ace0};std::memcpy(actual.data()+888,guards,8);
        if(std::memcmp(actual.data(),y,896)){
            if(mismatches++<20){std::cerr<<"case "<<n<<" selector "<<fields.selector;
                for(size_t k=0;k<896;++k)if(actual[k]!=y[k]){std::cerr<<" byte "<<k<<" got "<<int(actual[k])<<" expected "<<int(y[k]);break;}
                std::cerr<<'\n';}
        }
    }
    for(auto count:coverage)check(count>0,"Missing FM routing coverage");
    check(!mismatches,"FM original render/state/scratch differs");
    std::cout<<"PASS: 4096 FM blocks, 524288 samples, all 9 routings plus invalid selectors; full state/table/scratch/guards\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
