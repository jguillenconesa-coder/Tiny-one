// SPDX-License-Identifier: AGPL-3.0-only
// Complete independent render/lifecycle comparison against corrected BF524 observations.
#include "Dsynth246.h"
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
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing D-synth fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto x=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
 using namespace pocket::reference;check(argc==2,"Provide D-synth fixtures");const std::filesystem::path root(argv[1]);
 const auto in=read(root/"dsynth246-input.bin"),out=read(root/"dsynth246-output.bin");check(in.size()==4096*2048&&out.size()==1604+4096*4320,"Wrong D-synth fixture size");
 for(size_t i=0;i<256;++i)check(word(out.data()+i*4)==uint32_t(i<128?adsr246Attack[i]:adsr246Decay[i-128]),"D-synth envelope table mismatch");
 for(size_t i=0;i<129;++i)check(word(out.data()+1024+i*4)==uint32_t(filter246Frequency[i]),"D-synth filter table mismatch");
 for(size_t i=0;i<8;++i)for(size_t j=0;j<4;++j)check(half(out.data()+1540+i*8+j*2)==valueVoice246Ranges[8][i][j],"D-synth descriptor mismatch");
 std::array<Dsynth246State,6> pool{};Dsynth246Context context{};std::array<int16_t,8192> sine{};for(size_t i=0;i<8192;++i)sine[i]=int16_t(std::lround(std::sin(double(i)*3.14159265358979323846/4096)*32767));uint64_t energy=0;unsigned captures=0;
 for(size_t n=0;n<4096;++n){const auto* x=in.data()+n*2048;const auto* y=out.data()+1604+n*4320;const auto flags=word(x+1272),slot=word(x+1276);check(slot<6,"Invalid D-synth fixture slot");
  if(flags&1){std::memcpy(pool.data(),x,432);std::memcpy(&context,x+432,824);}
  std::array<int16_t,8> controls{};for(size_t i=0;i<8;++i)controls[i]=half(x+1256+i*2);
  auto& state=pool[slot];if(flags&16)context.initialize();if(flags&2){state.construct();check(state.bind(int(slot)),"D-synth bind failed");}if(flags&4)check(context.reset(state,controls),"D-synth reset failed");if(flags&8)state.release();if(flags&32)check(context.decayMeter(int(slot)),"Meter slot failed");if(flags&64)check(context.resetCapture(int(slot)),"Capture slot failed");
  check(std::memcmp(pool.data(),y,432)==0,"D-synth lifecycle pool mismatch");if(std::memcmp(&context,y+432,824)){const auto* actual=reinterpret_cast<const unsigned char*>(&context);for(size_t i=0;i<824;++i)if(actual[i]!=y[432+i]){std::cerr<<"case "<<n<<" context byte "<<i<<" got "<<int(actual[i])<<" expected "<<int(y[432+i])<<" flags "<<flags<<"\n";break;}throw std::runtime_error("D-synth lifecycle context mismatch");}
  const auto* post=y+1256;std::array<uint32_t,128> pitch{},derived{};std::array<int16_t,128> gain{},audio{},second{};
  for(size_t i=0;i<128;++i){pitch[i]=word(x+1280+i*4);gain[i]=half(x+1792+i*2);}
  check(dsynth246Block(state,context,controls,pitch,gain,sine,audio,derived,second),"Render rejected valid slot");
  std::array<unsigned char,4320> actual{};std::memcpy(actual.data(),y,1256);std::memcpy(actual.data()+1256,pool.data(),432);std::memcpy(actual.data()+1688,&context,824);std::memcpy(actual.data()+2512,audio.data(),256);std::memcpy(actual.data()+2768,pitch.data(),512);std::memcpy(actual.data()+3280,derived.data(),512);std::memcpy(actual.data()+3792,second.data(),256);std::memcpy(actual.data()+4048,gain.data(),256);const uint32_t guards[]={0x13579bdf,0x2468ace0,0x789abcde,0x89abcdef};std::memcpy(actual.data()+4304,guards,16);
  if(std::memcmp(actual.data(),y,4320)){std::cerr<<"case "<<n<<" differences:";int found=0;for(size_t i=1256;i<4320;++i)if(actual[i]!=y[i]){std::cerr<<" "<<i<<":"<<int(actual[i])<<"/"<<int(y[i]);if(++found==12)break;}std::cerr<<"\n";throw std::runtime_error("D-synth render mismatch");}
  for(auto sample:audio)energy+=uint64_t(int64_t(sample)*sample);captures++;

 }
 check(energy>0&&captures>0,"Empty D-synth characterization");
 {Dsynth246State s;s.slot=3;check(!s.bind(-1)&&!s.bind(6)&&s.slot==3,"Invalid bind modified state");Dsynth246Context c;c.counts.fill(99);c.initialize();check(c.counts[0]==99&&c.constants[0]==0x00a3d700,"Init incorrectly reset retained counters");}
 std::cout<<"PASS: D-synth complete reconstructed render, lifecycle, pool/context, eight mixes and all buffers, 4096 blocks/524288 samples\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
