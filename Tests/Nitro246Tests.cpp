// SPDX-License-Identifier: AGPL-3.0-only
#include "Nitro246.h"
#include "Filter246Tables.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static auto read(const std::string& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Nitro hybrid fixture");return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),{});}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string d=argv[1];auto in=read(d+"/nitro-hybrid246-input.bin"),out=read(d+"/nitro-hybrid246-output.bin"),table=read(d+"/verified-sine.bin");check(in.size()==4096*264&&out.size()==536+4096*252,"Incomplete Nitro hybrid oracle");
 std::array<int32_t,5> mix;std::array<int32_t,129> frequency;std::array<int16_t,8192> sine;std::memcpy(mix.data(),out.data(),20);std::memcpy(frequency.data(),out.data()+20,516);std::memcpy(sine.data(),table.data(),16384);
 check(mix==nitro246HybridMix&&frequency==filter246Frequency,"Runtime tables differ from explicit hybrid mix/shared frequency oracle");
 Nitro246State state;std::array<uint32_t,16> meters{};size_t failures=0;
 for(size_t i=0;i<4096;++i){const auto* p=in.data()+i*264;const auto* expected=out.data()+536+i*252;uint32_t action;std::memcpy(&action,p+260,4);if(action){std::memcpy(&state,p,60);std::memcpy(meters.data(),p+196,64);}std::array<int16_t,4> controls;std::array<int16_t,64> audio;std::memcpy(controls.data(),p+60,8);std::memcpy(audio.data(),p+68,128);check(nitro246HybridBlock(state,controls,mix,frequency,sine,audio,meters),"Valid Nitro hybrid input rejected");
 for(size_t j=0;j<64;++j){int16_t e;std::memcpy(&e,expected+j*2,2);if(audio[j]!=e&&failures++<12)std::cerr<<"case "<<i<<" sample "<<j<<" got "<<audio[j]<<" expected "<<e<<'\n';}

 if(std::memcmp(&state,expected+128,60)!=0&&failures++<12){std::cerr<<"case "<<i<<" state mismatch";for(size_t word=0;word<15;++word){uint32_t got,want;std::memcpy(&got,reinterpret_cast<const uint8_t*>(&state)+word*4,4);std::memcpy(&want,expected+128+word*4,4);if(got!=want)std::cerr<<" word "<<word<<" got "<<got<<" expected "<<want;}std::cerr<<'\n';}
 if(std::memcmp(meters.data(),expected+188,64)!=0&&failures++<12)std::cerr<<"case "<<i<<" meter mismatch\n";
 if(failures>100)break;
 }
 check(failures==0,"Nitro hybrid differs from conditionally substituted instruction oracle");std::cout<<"PASS: Nitro hybrid 262144 samples, 4096 complete states/displays and explicit hybrid coefficients and shared frequency table\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
