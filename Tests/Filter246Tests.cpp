// SPDX-License-Identifier: AGPL-3.0-only
#include "Filter246.h"
#include "Filter246Tables.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static auto read(const std::string& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Filter fixture");return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),{});}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string d=argv[1];auto in=read(d+"/filter246-input.bin"),out=read(d+"/filter246-output.bin"),table=read(d+"/verified-sine.bin");check(in.size()==4096*240&&out.size()==776+4096*228,"Incomplete Filter oracle");
 std::array<int32_t,65> mix;std::array<int32_t,129> frequency;std::array<int16_t,8192> sine;std::memcpy(mix.data(),out.data(),260);std::memcpy(frequency.data(),out.data()+260,516);std::memcpy(sine.data(),table.data(),16384);
 check(mix==filter246Mix&&frequency==filter246Frequency,"Generated runtime tables differ from initializer");
 Filter246State state;std::array<uint32_t,16> meters{};size_t failures=0;
 for(size_t i=0;i<4096;++i){const auto* p=in.data()+i*240;const auto* expected=out.data()+776+i*228;uint32_t action;std::memcpy(&action,p+236,4);if(action){std::memcpy(&state,p,36);std::memcpy(meters.data(),p+172,64);}std::array<int16_t,4> controls;std::array<int16_t,64> audio;std::memcpy(controls.data(),p+36,8);std::memcpy(audio.data(),p+44,128);check(filter246Block(state,controls,mix,frequency,sine,audio,meters),"Valid Filter input rejected");
 for(size_t j=0;j<64;++j){int16_t e;std::memcpy(&e,expected+j*2,2);if(audio[j]!=e&&failures++<12)std::cerr<<"case "<<i<<" sample "<<j<<" got "<<audio[j]<<" expected "<<e<<'\n';}

 if(std::memcmp(&state,expected+128,36)!=0&&failures++<12)std::cerr<<"case "<<i<<" state mismatch\n";
 if(std::memcmp(meters.data(),expected+164,64)!=0&&failures++<12)std::cerr<<"case "<<i<<" meter mismatch\n";
 if(failures>100)break;
 }
 check(failures==0,"Filter differs from original instruction oracle");std::cout<<"PASS: Filter 262144 samples, 4096 complete states/displays and 194 generated coefficients\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
