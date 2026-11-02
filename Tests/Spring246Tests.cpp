// SPDX-License-Identifier: AGPL-3.0-only
#include "Spring246.h"
#include "Spring246Tables.h"
#include "Filter246Tables.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static auto read(const std::string& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Spring fixture");return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),{});}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string d=argv[1];auto in=read(d+"/spring246-input.bin"),out=read(d+"/spring246-output.bin");check(in.size()==4096*312&&out.size()==790+4096*300+8*58272,"Incomplete Spring oracle");
 check(std::memcmp(out.data(),filter246Frequency.data(),516)==0&&std::memcmp(out.data()+516,spring246Low.data(),258)==0&&std::memcmp(out.data()+778,spring246Constants.data(),8)==0,"Runtime Spring coefficients differ from initializer oracle");
 auto memory=std::make_unique<Spring246Memory>();Spring246State state;int32_t pending=0;std::array<uint32_t,16> meters{};size_t failures=0,offset=790;
 for(size_t i=0;i<4096;++i){const auto* p=in.data()+i*312;const auto* expected=out.data()+offset;uint32_t action;std::memcpy(&action,p+304,4);
  if(action){std::memcpy(&state,p,104);std::memcpy(meters.data(),p+240,64);pending=0;if(action==1){uint32_t seed;std::memcpy(&seed,p+308,4);for(size_t j=0;j<memory->size();j+=2){seed=1664525u*seed+1013904223u;(*memory)[j]=int16_t(seed&65535u);(*memory)[j+1]=int16_t(seed>>16);}}if(action==2)spring246Reset(state,*memory,0,pending);}
  std::array<int16_t,4> c;std::array<int16_t,64> audio;std::memcpy(c.data(),p+104,8);std::memcpy(audio.data(),p+112,128);check(spring246Block(state,c,filter246Frequency,spring246Low,spring246Constants,*memory,pending,audio,meters),"Valid Spring input rejected");
  for(size_t j=0;j<64;++j){int16_t e;std::memcpy(&e,expected+j*2,2);if(audio[j]!=e&&failures++<8)std::cerr<<"case "<<i<<" sample "<<j<<" got "<<audio[j]<<" expected "<<e<<'\n';}
  if(std::memcmp(&state,expected+128,104)!=0&&failures++<8){std::cerr<<"case "<<i<<" state mismatch";for(size_t w=0;w<26;++w){int32_t got,want;std::memcpy(&got,reinterpret_cast<const uint8_t*>(&state)+w*4,4);std::memcpy(&want,expected+128+w*4,4);if(got!=want)std::cerr<<" word "<<w<<" got "<<got<<" expected "<<want;}std::cerr<<'\n';}
  if(std::memcmp(meters.data(),expected+232,64)||std::memcmp(&pending,expected+296,4)){if(failures++<8)std::cerr<<"case "<<i<<" display/counter mismatch\n";}
  offset+=300;if(i%512==511){if(std::memcmp(memory->data(),out.data()+offset,58272)!=0&&failures++<8)std::cerr<<"segment "<<i/512<<" workspace mismatch\n";offset+=58272;}
  if(failures>100)break;
 }
 check(failures==0,"Spring differs from instruction oracle");std::cout<<"PASS: Spring 4096 blocks, complete states/displays/counters and eight workspaces match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
