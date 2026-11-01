// SPDX-License-Identifier: AGPL-3.0-only
#include "Grid246.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static auto read(const std::string& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Grid fixture");return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),{});}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string d=argv[1];auto in=read(d+"/grid246-input.bin"),out=read(d+"/grid246-output.bin");check(in.size()==4096*248&&out.size()==4096*236+8*135200*2,"Incomplete Grid oracle");
 auto sineRaw=read(d+"/verified-sine.bin");check(sineRaw.size()>=16384,"Invalid sine fixture");std::array<int16_t,8192> sine;std::memcpy(sine.data(),sineRaw.data(),16384);auto memory=std::make_unique<Grid246Memory>();Grid246State state;std::array<uint32_t,16> meters{};size_t failures=0,offset=0;
 for(size_t i=0;i<4096;++i){const auto* p=in.data()+i*248;const auto* expected=out.data()+offset;uint32_t action;std::memcpy(&action,p+244,4);if(action){std::memcpy(&state,p,44);std::memcpy(meters.data(),p+180,64);memory->fill(0);if(action==2)state.reset();}std::array<int16_t,4> controls;std::array<int16_t,64> audio;std::memcpy(controls.data(),p+44,8);std::memcpy(audio.data(),p+52,128);check(grid246Block(state,controls,sine,*memory,audio,meters),"Valid Grid input rejected");
 for(size_t j=0;j<64;++j){int16_t e;std::memcpy(&e,expected+j*2,2);if(audio[j]!=e&&failures++<12)std::cerr<<"case "<<i<<" sample "<<j<<" got "<<audio[j]<<" expected "<<e<<'\n';}
 if(std::memcmp(&state,expected+128,44)!=0&&failures++<12){std::cerr<<"case "<<i<<" state mismatch";for(size_t w=0;w<11;++w){uint32_t got,want;std::memcpy(&got,reinterpret_cast<const uint8_t*>(&state)+w*4,4);std::memcpy(&want,expected+128+w*4,4);if(got!=want)std::cerr<<" word "<<w<<" got "<<got<<" expected "<<want;}std::cerr<<"\n";}
 if(std::memcmp(meters.data(),expected+172,64)!=0&&failures++<12)std::cerr<<"case "<<i<<" display mismatch\n";
 offset+=236;if(i%512==511){if(std::memcmp(memory->data(),out.data()+offset,135200*2)!=0&&failures++<12)std::cerr<<"segment "<<i/512<<" ring mismatch\n";offset+=135200*2;}
 if(failures>100)break;
 }
 check(failures==0,"Grid differs from instruction oracle");std::cout<<"PASS: Grid 262144 samples, 4096 states/displays, eight complete two-plane workspaces\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
