// SPDX-License-Identifier: AGPL-3.0-only
#include "Crank246.h"
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
 const auto in=read(std::filesystem::path(argv[1])/"crank246-input.bin"),out=read(std::filesystem::path(argv[1])/"crank246-output.bin");check(in.size()==18432*124&&out.size()==18432*1136,"Incomplete Crank fixtures");Crank246State state;
 for(size_t n=0;n<18432;++n){const auto* x=in.data()+n*124;const auto* y=out.data()+n*1136;const auto action=word(x+12);
  if(action&1){std::memcpy(state.words.data(),x+20,28);std::memcpy(state.history.data(),x+48,40);state.previous=int32_t(word(x+88));}if(action&2)crank246Prepare(state);if(action&4)crank246ResetInput(state);
  std::array<int16_t,8> c{},targets{};for(size_t i=0;i<8;++i){c[i]=half(x+92+i*2);targets[i]=half(x+108+i*2);}
  const uint32_t accepted=crank246Step(state,c,int(word(x)),int(word(x+8)),int(word(x+4)),targets);
  std::array<unsigned char,1136> actual{};std::memcpy(actual.data(),x+16,4);std::memcpy(actual.data()+4,state.words.data(),28);std::memcpy(actual.data()+32,state.history.data(),40);std::memcpy(actual.data()+72,&state.previous,4);std::memcpy(actual.data()+76,c.data(),16);std::memcpy(actual.data()+92,targets.data(),16);std::memcpy(actual.data()+108,&accepted,4);
  for(size_t i=0;i<256;++i){const uint32_t v=i<128?0x01234567u*uint32_t(i+1):i<192?0x04030807u*uint32_t(i-127):0x09020705u*uint32_t(i-191);std::memcpy(actual.data()+112+i*4,&v,4);}
  if(std::memcmp(actual.data(),y,1136)){for(size_t i=0;i<1136;++i)if(actual[i]!=y[i]){std::cerr<<"Crank call "<<n<<" family "<<word(x)<<" type "<<word(x+4)<<" kind "<<word(x+8)<<" byte "<<i<<" expected "<<int(y[i])<<" got "<<int(actual[i])<<'\n';break;}throw std::runtime_error("Original Crank source/controls/targets/return/buffer mismatch");}
 }
 std::cout<<"PASS: 18432 interleaved Crank callbacks, shared source/ring/flags, counters/wrap/direction/idle, all controls/targets and unchanged full buffers\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
