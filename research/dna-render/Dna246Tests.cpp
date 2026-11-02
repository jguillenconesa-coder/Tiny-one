// SPDX-License-Identifier: AGPL-3.0-only
#include "Dna246.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing DNA fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto x=uint32_t(p[0])|(uint32_t(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
    check(argc==2,"Provide DNA fixture directory");using namespace pocket::reference;const std::filesystem::path root(argv[1]);
    for(bool probe:{false,true}){const std::string prefix=probe?"dna246-ram-probe-":"dna246-";const size_t cases=probe?2u:4096u;double energy[2]{};
    const auto in=read(root/(prefix+"input.bin")),out=read(root/(prefix+"output.bin"));check(in.size()==cases*256,"Wrong DNA input size");
    for(size_t i=0;i<129;++i)check(spring246Low[i]==half(out.data()+i*2),"DNA low table mismatch");for(size_t i=0;i<225;++i)check(delay246Interpolation[i]==half(out.data()+258+i*2),"DNA interpolation mismatch");
    Dna246Context context;std::array<unsigned char,196608> ram{};uint32_t rng=0;size_t at=772,mismatches=0;
    for(size_t n=0;n<cases;++n){const auto* x=in.data()+n*256;const auto action=word(x+208);const auto slot=size_t(word(x+216));
        if(action&1){uint32_t seed=word(x+212);for(size_t i=0;i<ram.size();i+=4){seed=seed*1664525u+1013904223u;const auto value=word(x+220)?seed:0;for(unsigned b=0;b<4;++b)ram[i+b]=static_cast<unsigned char>(value>>(b*8));}std::memcpy(context.pool.data(),x,192);rng=word(x+204);std::array<char,32> identity{};std::memcpy(identity.data(),x+224,32);dna246Identity(context.wave,identity);context.feedback=0x40000000;
            check(at+512<=out.size(),"Truncated identity result");check(std::memcmp(context.wave.data(),out.data()+at,512)==0,"DNA original identity fragment mismatch");at+=512;}
        auto& state=context.pool[slot];if(action&8)state.construct();if(action&2)state.reset();if(action&4)state.release();
        check(at+492<=out.size(),"Truncated DNA result");const auto* y=out.data()+at;
        if(std::memcmp(&state,y,32)){if(mismatches++<12)std::cerr<<"lifecycle "<<n<<'\n';}
        std::array<int16_t,4> controls{};for(size_t k=0;k<4;++k)controls[k]=half(x+192+k*2);std::array<int16_t,128> audio{};
        const auto pitch=word(x+200);check(dna246Period(pitch)==uint32_t(std::clamp(signedPhase(word(y+488)),int32_t(0x20000),int32_t(0x30000000))),"DNA pitch division mismatch");
        bool range=true;auto reader=[&](const Dna246Pool& pool,size_t index){if(index>=ram.size()){range=false;return uint8_t(0);}return index<192?dna246PoolByte(pool,index):uint8_t(ram[index]);};
        if(!dna246Block(context,slot,controls,pitch,rng,audio,reader)){std::cerr<<"domain "<<n<<" A "<<state.phaseA<<" B "<<state.phaseB<<" period "<<dna246Period(pitch)<<'\n';throw std::runtime_error("DNA domain rejected");}check(range,"DNA source RAM range exceeded");
        std::array<unsigned char,456> actual{};std::memcpy(actual.data(),context.pool.data(),192);std::memcpy(actual.data()+192,&context.feedback,4);std::memcpy(actual.data()+196,&rng,4);std::memcpy(actual.data()+200,audio.data(),256);
        if(std::memcmp(actual.data(),y+32,456)){if(mismatches++<20){std::cerr<<"render "<<n;for(size_t k=0;k<456;++k)if(actual[k]!=y[32+k]){std::cerr<<" byte "<<k<<" got "<<int(actual[k])<<" expected "<<int(y[32+k]);break;}std::cerr<<'\n';}}
        if(probe)for(auto sample:audio)energy[n]+=double(sample)*sample;
        at+=492;
        if(n%256==255){check(at+ram.size()+24576<=out.size(),"Truncated DNA memory");std::memcpy(ram.data(),context.pool.data(),192);if(std::memcmp(ram.data(),out.data()+at,ram.size())){if(mismatches++<20)std::cerr<<"RAM "<<n<<'\n';}at+=ram.size();check(std::memcmp(context.wave.data(),out.data()+at,24576)==0,"DNA waveform changed");at+=24576;}
    }
    check(at==out.size(),"Unexpected DNA trailing data");check(!mismatches,"DNA lifecycle/render/live RAM mismatch");if(probe)check(energy[0]==0&&energy[1]>0,"DNA paired RAM dependency lost");}
    std::cout<<"PASS: DNA identity conversion, 4096 blocks / 524288 samples, six live state slots, shared/RNG and 16 full RAM/wave snapshots and isolated RAM-dependence probe\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
