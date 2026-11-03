// SPDX-License-Identifier: AGPL-3.0-only
#include "Sampler246.h"
#include "Drum246Tables.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing drum sampler fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto v=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(v<32768?int(v):int(v)-65536);}
int main(int argc,char** argv){try{
 using namespace pocket::reference;check(argc==2,"Provide drum sampler fixtures");const std::filesystem::path root(argv[1]);const auto in=read(root/"sampler246-input.bin"),out=read(root/"sampler246-output.bin");check(in.size()==33024+4096*636&&out.size()==2052+4096*968+33024,"Wrong drum fixture sizes");
 for(size_t i=0;i<1025;++i)check(half(out.data()+i*2)==drum246FirCoefficients[i],"Original FIR conversion mismatch");
 check(std::memcmp(in.data(),out.data()+2052+4096*968,33024)==0,"Original sampler modified PCM");
 std::array<std::array<int16_t,4096>,4> pcm{};for(size_t b=0;b<4;++b)for(size_t i=0;i<4096;++i)pcm[b][i]=half(in.data()+b*8256+32+i*2);
 std::array<Sampler246State,6> pool{};uint64_t energy=0;
 for(size_t n=0;n<4096;++n){const auto* x=in.data()+33024+n*636;const auto* y=out.data()+2052+n*968;const auto flags=word(x+112),slot=word(x+116);check(slot<6,"Invalid drum fixture slot");if(flags&1)std::memcpy(pool.data(),x,96);auto& state=pool[slot];std::array<int16_t,8> controls{};for(size_t i=0;i<8;++i)controls[i]=half(x+96+i*2);
  if(flags&2){state.construct();state.bind(0x02010000);}if(flags&4)state.reset(controls);if(flags&8)state.release();
  if(std::memcmp(pool.data(),y,96)){std::cerr<<"lifecycle case "<<n<<'\n';throw std::runtime_error("Drum lifecycle mismatch");}
  std::array<uint32_t,128> pitch{};std::array<int16_t,128> audio{};for(size_t i=0;i<128;++i)pitch[i]=word(x+124+i*4);
  const auto bank=(flags>>8)&3;sampler246Block(state,controls,pitch,signedPhase(word(x+120)),{pcm[bank].data(),4096},drum246FirCoefficients,audio);
  std::array<unsigned char,968> actual{};std::memcpy(actual.data(),y,96);std::memcpy(actual.data()+96,pool.data(),96);std::memcpy(actual.data()+192,audio.data(),256);std::memcpy(actual.data()+448,pitch.data(),512);const uint32_t guards[]={0x13579bdf,0x2468ace0};std::memcpy(actual.data()+960,guards,8);
  if(std::memcmp(actual.data(),y,968)){std::cerr<<"case "<<n<<" differences:";int found=0;for(size_t i=96;i<968;++i)if(actual[i]!=y[i]){std::cerr<<" "<<i<<":"<<int(actual[i])<<"/"<<int(y[i]);if(++found==12)break;}std::cerr<<'\n';throw std::runtime_error("Drum sampler render mismatch");}for(auto v:audio)energy+=uint64_t(int64_t(v)*v);
 }
 check(energy>0,"Drum fixture is silent");check(Drum246Pcm{}.at(-1)==0,"Bounded PCM contract failed");std::cout<<"PASS: complete melodic sampler callback, 4096 blocks/524288 samples, 6 states, FIR conversion, forward/reverse/loop/release, all buffers and guards\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
