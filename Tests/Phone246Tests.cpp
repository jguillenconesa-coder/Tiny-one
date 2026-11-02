// SPDX-License-Identifier: AGPL-3.0-only
#include "Phone246.h"
#include "Phone246Tables.h"
#include "Filter246Tables.h"
#include "Delay246Tables.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static auto read(const std::string& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Phone fixture");return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),{});}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string d=argv[1];auto in=read(d+"/phone246-input.bin"),out=read(d+"/phone246-output.bin");check(in.size()==4096*344&&out.size()==1224+4096*332+8*17642*2,"Incomplete Phone oracle");
 std::array<int32_t,129> frequency;std::array<int16_t,129> low;std::array<int16_t,225> table;std::memcpy(frequency.data(),out.data(),516);std::memcpy(low.data(),out.data()+516,258);std::memcpy(table.data(),out.data()+774,450);check(frequency==filter246Frequency&&table==delay246Interpolation&&low[127]==phone246LowCoefficient,"Runtime shared Phone coefficients differ from oracle");auto memory=std::make_unique<Phone246Memory>();Phone246State state;std::array<uint32_t,16> meters{};size_t failures=0,offset=1224;
 for(size_t i=0;i<4096;++i){const auto* p=in.data()+i*344;const auto* expected=out.data()+offset;uint32_t action;std::memcpy(&action,p+340,4);if(action){std::memcpy(&state,p,140);std::memcpy(meters.data(),p+276,64);if(action==1){memory->fill(0);(*memory)[17640]=0x1234;(*memory)[17641]=-0x2345;}if(action==2)phone246Reset(state,*memory);}std::array<int16_t,4> controls;std::array<int16_t,64> audio;std::memcpy(controls.data(),p+140,8);std::memcpy(audio.data(),p+148,128);check(phone246Block(state,controls,table,saturate32(int64_t(2147483647)-frequency[0]),low[127],*memory,audio,meters),"Valid Phone input rejected");
 for(size_t j=0;j<64;++j){int16_t e;std::memcpy(&e,expected+j*2,2);if(audio[j]!=e&&failures++<12)std::cerr<<"case "<<i<<" sample "<<j<<" got "<<audio[j]<<" expected "<<e<<'\n';}
 if(std::memcmp(&state,expected+128,140)!=0&&failures++<12){std::cerr<<"case "<<i<<" state mismatch";for(size_t w=0;w<35;++w){uint32_t got,want;std::memcpy(&got,reinterpret_cast<const uint8_t*>(&state)+w*4,4);std::memcpy(&want,expected+128+w*4,4);if(got!=want)std::cerr<<" word "<<w<<" got "<<got<<" expected "<<want;}std::cerr<<"\n";}
 if(std::memcmp(meters.data(),expected+268,64)!=0&&failures++<12)std::cerr<<"case "<<i<<" display mismatch\n";
 offset+=332;if(i%512==511){if(std::memcmp(memory->data(),out.data()+offset,17642*2)!=0&&failures++<12)std::cerr<<"segment "<<i/512<<" ring mismatch\n";offset+=17642*2;}
 if(failures>100)break;
 }
 check(failures==0,"Phone differs from instruction oracle");std::cout<<"PASS: Phone 262144 samples, 4096 states/displays, eight complete triple-buffer workspaces\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
