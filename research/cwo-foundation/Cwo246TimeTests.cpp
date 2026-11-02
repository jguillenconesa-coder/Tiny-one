// SPDX-License-Identifier: AGPL-3.0-only
#include "Cwo246Time.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static auto read(const std::string& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing CWO time fixture");return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),{});}
static uint32_t word(const uint8_t* p){uint32_t x;std::memcpy(&x,p,4);return x;}
static int16_t half(const uint8_t* p){int16_t x;std::memcpy(&x,p,2);return x;}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string d=argv[1];const auto in=read(d+"/cwo246-time-input.bin"),out=read(d+"/cwo246-time-output.bin"),base=read(d+"/cwo246-foundation-output.bin");check(in.size()==8192*44&&out.size()==22684+8192*272+16*131072,"Incomplete CWO time oracle");check(std::memcmp(out.data(),base.data(),22684)==0,"CWO initializer differs across probes");
 Cwo246DelayState state;auto memory=std::make_unique<Cwo246DelayMemory>();size_t offset=22684,failures=0;
 for(size_t i=0;i<8192;++i){const auto* p=in.data()+i*44;const auto* e=out.data()+offset;if(word(p+24)){std::memcpy(&state,p,12);auto seed=word(p+28);for(size_t j=0;j<65536;j+=2){seed=1664525u*seed+1013904223u;(*memory)[j]=int16_t(seed&65535u);(*memory)[j+1]=int16_t(seed>>16);}}
  int16_t output;check(cwo246DelaySample(state,*memory,half(p+12),word(p+16),signedPhase(word(p+20)),output),"Valid delay state rejected");std::array<int16_t,64> sine,cosine;check(cwo246Oscillator(signedPhase(word(p+32)),signedPhase(word(p+36)),half(p+40),sine,cosine),"Valid oscillator rejected");
  if(std::memcmp(&state,e,12)||output!=half(e+12)){if(failures++<8)std::cerr<<"delay case "<<i<<" cursor "<<state.write<<" last "<<state.last<<" prev "<<state.previous<<" expected "<<half(e+4)<<"/"<<half(e+6)<<'\n';}
  if(std::memcmp(sine.data(),e+16,128)||std::memcmp(cosine.data(),e+144,128)){if(failures++<8){std::cerr<<"oscillator case "<<i;for(size_t j=0;j<64;++j)if(sine[j]!=half(e+16+j*2)||cosine[j]!=half(e+144+j*2)){std::cerr<<" sample "<<j<<" got "<<sine[j]<<"/"<<cosine[j]<<" expected "<<half(e+16+j*2)<<"/"<<half(e+144+j*2);break;}std::cerr<<'\n';}}
  offset+=272;if(i%512==511){if(std::memcmp(memory->data(),out.data()+offset,131072)&&failures++<8)std::cerr<<"workspace case "<<i<<'\n';offset+=131072;}if(failures>100)break;
 }
 check(failures==0,"CWO time components differ from original instructions");std::cout<<"PASS: CWO 8192 variable-delay calls / oscillator blocks and 16 complete circular memories match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
