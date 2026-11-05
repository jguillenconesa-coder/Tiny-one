// SPDX-License-Identifier: AGPL-3.0-only
#include "Crank246Special.h"
#include "Bend246Voice.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Crank fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const unsigned x=p[0]|(unsigned(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{check(argc==2,"Provide unpacked fixture directory");using namespace pocket::reference;
 const auto in=read(std::filesystem::path(argv[1])/"crank-voice246-input.bin"),out=read(std::filesystem::path(argv[1])/"crank-voice246-output.bin");check(in.size()==18432*128&&out.size()==18432*1140,"Incomplete Crank fixtures");Crank246State state;uint32_t gainState=0;
 for(size_t n=0;n<18432;++n){const auto* x=in.data()+n*128;const auto* y=out.data()+n*1140;const auto action=word(x+12);
  if(action&1){std::memcpy(state.words.data(),x+20,28);std::memcpy(state.history.data(),x+48,40);state.previous=int32_t(word(x+88));gainState=word(x+92);}if(action&2)crank246Prepare(state);if(action&4)crank246ResetInput(state);
  std::array<int16_t,8> c{},targets{};for(size_t i=0;i<8;++i){c[i]=half(x+96+i*2);targets[i]=half(x+112+i*2);}
  std::array<uint32_t,128> pitch{};std::array<int16_t,128> gain{};for(size_t i=0;i<128;++i){pitch[i]=0x01234567u*uint32_t(i+1);const auto v=0x04030807u*uint32_t(i/2+1);gain[i]=half(reinterpret_cast<const unsigned char*>(&v)+(i%2)*2);}const auto inputPitch=pitch;const auto inputGain=gain;
  const uint32_t accepted=crank246VoiceStep(state,gainState,c,int(word(x)),int(word(x+8)),pitch,gain);
  if(accepted){pocket::Bend246Voice bridge;bridge.configure(true,c[3]/4096,int16_t(gainState&65535u),crank246PitchCoefficient(crank246Level(state),c[1]),true,2);auto values=inputPitch;bridge.pitchBlock(values);check(values==pitch,"Crank pitch bridge differs from original");for(size_t i=0;i<128;++i)check(bridge.amplitude(inputGain[i])==gain[i],"Crank gain bridge differs from original");}
  std::array<unsigned char,1140> actual{};std::memcpy(actual.data(),x+16,4);std::memcpy(actual.data()+4,state.words.data(),28);std::memcpy(actual.data()+32,state.history.data(),40);std::memcpy(actual.data()+72,&state.previous,4);std::memcpy(actual.data()+80,c.data(),16);std::memcpy(actual.data()+96,targets.data(),16);std::memcpy(actual.data()+112,&accepted,4);
  std::memcpy(actual.data()+76,&gainState,4);
  for(size_t i=0;i<256;++i){const uint32_t v=i<128?0x01234567u*uint32_t(i+1):i<192?0x04030807u*uint32_t(i-127):0x09020705u*uint32_t(i-191);std::memcpy(actual.data()+116+i*4,&v,4);}
  std::memcpy(actual.data()+116,pitch.data(),512);std::memcpy(actual.data()+628,gain.data(),256);
  if(std::memcmp(actual.data(),y,1140)){for(size_t i=0;i<1140;++i)if(actual[i]!=y[i]){std::cerr<<"Crank call "<<n<<" family "<<word(x)<<" type "<<word(x+4)<<" kind "<<word(x+8)<<" byte "<<i<<" expected "<<int(y[i])<<" got "<<int(actual[i])<<'\n';break;}throw std::runtime_error("Original Crank source/controls/targets/return/buffer mismatch");}
 }
 std::cout<<"PASS: 18432 Crank voice callbacks, shared source/ring/flags, counters/wrap/direction/idle, all controls/targets and exact pitch/gain/FX buffers\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
