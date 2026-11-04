// SPDX-License-Identifier: AGPL-3.0-only
#include "Bend246.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Bend fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const unsigned x=p[0]|(unsigned(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{check(argc==2,"Provide unpacked fixture directory");using namespace pocket::reference;
 const auto in=read(std::filesystem::path(argv[1])/"bend246-input.bin"),out=read(std::filesystem::path(argv[1])/"bend246-output.bin");check(in.size()==6144*96&&out.size()==6144*1108,"Incomplete Bend fixtures");Bend246State state;
 for(size_t n=0;n<6144;++n){const auto* x=in.data()+n*96;const auto* y=out.data()+n*1108;const auto action=word(x+12);
  if(action&1){std::memcpy(state.words.data(),x+20,32);std::memcpy(state.history.data(),x+52,8);state.display=word(x+60);}if(action&2)bend246Prepare(state);if(action&4)bend246ResetInput(state);
  std::array<int16_t,8> c{},targets{};for(size_t i=0;i<8;++i){c[i]=half(x+64+i*2);targets[i]=half(x+80+i*2);}
  const uint32_t accepted=bend246Step(state,c,int(word(x)),int(word(x+8)),int(word(x+4)),targets);
  std::array<unsigned char,1108> actual{};std::memcpy(actual.data(),x+16,4);std::memcpy(actual.data()+4,state.words.data(),32);std::memcpy(actual.data()+36,state.history.data(),8);std::memcpy(actual.data()+44,&state.display,4);std::memcpy(actual.data()+48,c.data(),16);std::memcpy(actual.data()+64,targets.data(),16);std::memcpy(actual.data()+80,&accepted,4);
  for(size_t i=0;i<256;++i){const uint32_t v=i<128?0x01234567u*uint32_t(i+1):i<192?0x04030807u*uint32_t(i-127):0x09020705u*uint32_t(i-191);std::memcpy(actual.data()+84+i*4,&v,4);}
  if(std::memcmp(actual.data(),y,1108)){for(size_t i=0;i<1108;++i)if(actual[i]!=y[i]){std::cerr<<"Bend call "<<n<<" family "<<word(x)<<" type "<<word(x+4)<<" kind "<<word(x+8)<<" byte "<<i<<" expected "<<int(y[i])<<" got "<<int(actual[i])<<'\n';break;}throw std::runtime_error("Original Bend source/controls/targets/return/buffer mismatch");}
 }
 std::cout<<"PASS: 6144 interleaved Bend callbacks, shared source/ring/flags, counters/wrap/direction/idle, all controls/targets and unchanged full buffers\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
