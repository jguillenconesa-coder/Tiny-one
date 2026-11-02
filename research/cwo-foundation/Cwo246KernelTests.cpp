// SPDX-License-Identifier: AGPL-3.0-only
#include "Cwo246Kernel.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static auto read(const std::string& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing CWO kernel fixture");return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),{});}
static uint32_t word(const uint8_t* p){uint32_t x;std::memcpy(&x,p,4);return x;}
static int16_t half(const uint8_t* p){int16_t x;std::memcpy(&x,p,2);return x;}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string d=argv[1];const auto in=read(d+"/cwo246-kernel-input.bin"),out=read(d+"/cwo246-kernel-output.bin"),base=read(d+"/cwo246-foundation-output.bin");check(in.size()==4096*460&&out.size()==22684+4096*444+8*131072,"Incomplete CWO kernel oracle");check(std::memcmp(out.data(),base.data(),22684)==0,"CWO kernel initializer mismatch");
 Cwo246State state;auto memory=std::make_unique<Cwo246DelayMemory>();std::array<uint32_t,16> meters{};size_t offset=22684,failures=0;
 for(size_t i=0;i<4096;++i){const auto* p=in.data()+i*460;const auto* e=out.data()+offset;if(word(p+452)){std::memcpy(&state,p,252);std::memcpy(meters.data(),p+388,64);auto seed=word(p+456);for(size_t j=0;j<65536;j+=2){seed=1664525u*seed+1013904223u;(*memory)[j]=int16_t(seed&65535u);(*memory)[j+1]=int16_t(seed>>16);}}
  std::array<int16_t,4> controls;std::array<int16_t,64> audio;std::memcpy(controls.data(),p+252,8);std::memcpy(audio.data(),p+260,128);if(!cwo246HybridBlock(state,controls,*memory,audio,meters)){std::cerr<<"rejected case "<<i<<'\n';throw std::runtime_error("Valid conditioned CWO case rejected");}
  for(size_t j=0;j<64;++j)if(audio[j]!=half(e+j*2)&&failures++<8)std::cerr<<"case "<<i<<" sample "<<j<<" got "<<audio[j]<<" expected "<<half(e+j*2)<<'\n';
  if(std::memcmp(&state,e+128,252)&&failures++<8){std::cerr<<"case "<<i<<" state";for(size_t j=0;j<63;++j){uint32_t v;std::memcpy(&v,reinterpret_cast<const uint8_t*>(&state)+j*4,4);if(v!=word(e+128+j*4))std::cerr<<" word "<<j<<" got "<<signedPhase(v)<<" expected "<<signedPhase(word(e+128+j*4));}std::cerr<<'\n';}
  if(std::memcmp(meters.data(),e+380,64)&&failures++<8){std::cerr<<"case "<<i<<" display got "<<meters[0]<<"/"<<meters[1]<<" expected "<<word(e+380)<<"/"<<word(e+384)<<'\n';}
  offset+=444;if(i%512==511){if(std::memcmp(memory->data(),out.data()+offset,131072)&&failures++<8)std::cerr<<"workspace case "<<i<<'\n';offset+=131072;}if(failures>100)break;
 }
 check(failures==0,"Conditioned CWO kernel differs from original instructions");std::cout<<"PASS: conditioned CWO mono 4096 blocks, complete states/displays/RNG and eight circular memories match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
