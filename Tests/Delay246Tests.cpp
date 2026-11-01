// SPDX-License-Identifier: AGPL-3.0-only
#include "Delay246.h"
#include "Delay246Tables.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static auto read(const std::string& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Delay fixture");return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),{});}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string d=argv[1];auto in=read(d+"/delay246-input.bin"),out=read(d+"/delay246-output.bin");check(in.size()==4096*224&&out.size()==450+4096*212+8*44100*4,"Incomplete Delay oracle");
 std::array<int16_t,225> table;std::memcpy(table.data(),out.data(),450);check(table==delay246Interpolation,"Runtime Delay coefficients differ from initializer");auto memory=std::make_unique<std::array<int32_t,44100>>();Delay246State state;std::array<uint32_t,16> meters{};size_t failures=0,offset=450;
 for(size_t i=0;i<4096;++i){const auto* p=in.data()+i*224;const auto* expected=out.data()+offset;uint32_t action;std::memcpy(&action,p+220,4);if(action){std::memcpy(&state,p,20);std::memcpy(meters.data(),p+156,64);memory->fill(0);if(action==2)state.reset();}std::array<int16_t,4> controls;std::array<int16_t,64> audio;std::memcpy(controls.data(),p+20,8);std::memcpy(audio.data(),p+28,128);check(delay246Block(state,controls,table,*memory,audio,meters),"Valid Delay input rejected");
 for(size_t j=0;j<64;++j){int16_t e;std::memcpy(&e,expected+j*2,2);if(audio[j]!=e&&failures++<12)std::cerr<<"case "<<i<<" sample "<<j<<" got "<<audio[j]<<" expected "<<e<<'\n';}
 if(std::memcmp(&state,expected+128,20)!=0&&failures++<12){std::cerr<<"case "<<i<<" state mismatch";for(size_t w=0;w<5;++w){uint32_t got,want;std::memcpy(&got,reinterpret_cast<const uint8_t*>(&state)+w*4,4);std::memcpy(&want,expected+128+w*4,4);if(got!=want)std::cerr<<" word "<<w<<" got "<<got<<" expected "<<want;}std::cerr<<"\n";}
 if(std::memcmp(meters.data(),expected+148,64)!=0&&failures++<12)std::cerr<<"case "<<i<<" display mismatch\n";
 offset+=212;if(i%512==511){if(std::memcmp(memory->data(),out.data()+offset,44100*4)!=0&&failures++<12)std::cerr<<"segment "<<i/512<<" ring mismatch\n";offset+=44100*4;}
 if(failures>100)break;
 }
 check(failures==0,"Delay differs from instruction oracle");std::cout<<"PASS: Delay 262144 samples, 4096 states/displays, 225 coefficients and eight complete delay rings\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
