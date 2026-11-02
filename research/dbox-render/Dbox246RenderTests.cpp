// SPDX-License-Identifier: AGPL-3.0-only
// Complete independent render/lifecycle comparison against corrected BF524 observations.
#include "Dbox246.h"
#include <cmath>
#include "Envelope246Tables.h"
#include "Filter246Tables.h"
#include "ValueVoice246Ranges.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <vector>
static void check(bool x,const char* why){if(!x)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing D-box fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto x=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(x<35264?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
 using namespace pocket::reference;check(argc==2,"Provide D-box fixtures");const std::filesystem::path root(argv[1]);
 const auto in=read(root/"dbox246-input.bin"),out=read(root/"dbox246-output.bin");check(in.size()==4096*3296&&out.size()==1540+4096*6816,"Wrong D-box fixture size");
 for(size_t i=0;i<256;++i)check(word(out.data()+i*4)==uint32_t(i<128?adsr246Attack[i]:adsr246Decay[i-128]),"D-box envelope table mismatch");
 for(size_t i=0;i<129;++i)check(word(out.data()+1024+i*4)==uint32_t(filter246Frequency[i]),"D-box filter table mismatch");
 std::array<Dbox246State,12> pool{};Dbox246Context context{};std::array<int16_t,8192> sine{};for(size_t i=0;i<8192;++i)sine[i]=int16_t(std::lround(std::sin(double(i)*3.14159265358979323846/4096)*32767));uint64_t energy=0;unsigned captures=0;
 for(size_t n=0;n<4096;++n){const auto* x=in.data()+n*3296;const auto* y=out.data()+1540+n*6816;const auto flags=word(x+2520),slot=word(x+2524);check(slot<12,"Invalid D-box fixture slot");
  if(flags&1){std::memcpy(pool.data(),x,864);std::memcpy(&context,x+864,1640);}
  std::array<int16_t,8> controls{};for(size_t i=0;i<8;++i)controls[i]=half(x+2504+i*2);
  auto& state=pool[slot];if(flags&16)context.initialize();if(flags&2){state.construct();check(state.bind(int(slot)),"D-box bind failed");}if(flags&4)check(context.reset(state),"D-box reset failed");if(flags&8)state.release();if(flags&32)check(context.decayMeter(int(slot)),"Meter slot failed");if(flags&128)state.active();if(flags&64)check(context.resetCapture(int(slot)),"Capture slot failed");
  check(std::memcmp(pool.data(),y,864)==0,"D-box lifecycle pool mismatch");if(std::memcmp(&context,y+864,1640)){const auto* actual=reinterpret_cast<const unsigned char*>(&context);for(size_t i=0;i<1640;++i)if(actual[i]!=y[864+i]){std::cerr<<"case "<<n<<" context byte "<<i<<" got "<<int(actual[i])<<" expected "<<int(y[864+i])<<" flags "<<flags<<"\n";break;}throw std::runtime_error("D-box lifecycle context mismatch");}
  std::array<uint32_t,128> pitch{},derived{};std::array<int16_t,128> gain{},audio{},second{};
  for(size_t i=0;i<128;++i){pitch[i]=word(x+2528+i*4);gain[i]=half(x+3040+i*2);}
  check(dbox246Block(state,context,controls,pitch,gain,sine,audio,derived,second),"Render rejected valid slot");
  std::array<unsigned char,6816> actual{};std::memcpy(actual.data(),y,2504);std::memcpy(actual.data()+2504,pool.data(),864);std::memcpy(actual.data()+3368,&context,1640);std::memcpy(actual.data()+5008,audio.data(),256);std::memcpy(actual.data()+5264,pitch.data(),512);std::memcpy(actual.data()+5776,derived.data(),512);std::memcpy(actual.data()+6288,second.data(),256);std::memcpy(actual.data()+6544,gain.data(),256);const uint32_t guards[]={0x13579bdf,0x2468ace0,0x789abcde,0x89abcdef};std::memcpy(actual.data()+6800,guards,16);
  if(std::memcmp(actual.data(),y,6816)){std::cerr<<"case "<<n<<" differences:";int found=0;for(size_t i=2504;i<6816;++i)if(actual[i]!=y[i]){std::cerr<<" "<<i<<":"<<int(actual[i])<<"/"<<int(y[i]);if(++found==12)break;}std::cerr<<"\n";throw std::runtime_error("D-box render mismatch");}
  for(auto sample:audio)energy+=uint64_t(int64_t(sample)*sample);captures++;

 }
 check(energy>0&&captures>0,"Empty D-box characterization");
 {Dbox246State s;s.slot=3;check(!s.bind(-1)&&!s.bind(12)&&s.slot==3,"Invalid bind modified state");Dbox246Context c;c.counts.fill(99);c.initialize();check(c.counts[0]==99&&c.constants[0]==0x00a3d700,"Init incorrectly reset retained counters");}
 std::cout<<"PASS: D-box complete reconstructed render, lifecycle, pool/context, eight mixes and all buffers, 4096 blocks/524288 samples\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
