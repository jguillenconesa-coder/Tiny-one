// SPDX-License-Identifier: AGPL-3.0-only
#include "Spring246Lifecycle.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static auto read(const std::string& path){std::ifstream f(path,std::ios::binary);check(bool(f),"Missing Spring lifecycle fixture");return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),{});}
static uint32_t word(const uint8_t* p){uint32_t x;std::memcpy(&x,p,4);return x;}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string dir=argv[1];const auto in=read(dir+"/spring246-lifecycle-input.bin"),out=read(dir+"/spring246-lifecycle-output.bin");
 check(in.size()==24*316&&out.size()==24*116848,"Incomplete lifecycle fixture");
 auto memory=std::make_unique<std::array<Spring246Memory,2>>();Spring246State state{};std::array<int32_t,2> pending{};std::array<uint32_t,16> display{};
 for(size_t i=0;i<24;++i){const auto* input=in.data()+i*316;const auto* expected=out.data()+i*116848;const auto action=word(input+304);const auto slot=word(input+312);
  std::array<int16_t,64> audio;std::memcpy(audio.data(),input+112,128);
  if(!action){std::memcpy(&state,input,104);std::memcpy(display.data(),input+240,64);pending={{0x12345678,0x23456789}};auto seed=word(input+308);
   for(auto& bank:*memory)for(size_t j=0;j<bank.size();j+=2){seed=1664525u*seed+1013904223u;bank[j]=int16_t(seed&65535u);bank[j+1]=int16_t(seed>>16);}
   check(spring246Reset(state,(*memory)[slot],int32_t(slot),pending[slot]),"Reset rejected explicit slot");
  }else check(spring246DeferredClear((*memory)[slot],pending[slot]),"Deferred callback rejected pending state");
  if(std::memcmp(&state,expected,104)||std::memcmp(audio.data(),expected+104,128)||std::memcmp(display.data(),expected+232,64)||std::memcmp(pending.data(),expected+296,8)||std::memcmp(memory->data(),expected+304,116544)){std::cerr<<"Spring lifecycle mismatch at case "<<i<<'\n';throw std::runtime_error("Lifecycle differs from original instructions");}
 }
 const auto before=(*memory)[0];pending[0]=0;check(!spring246DeferredClear((*memory)[0],pending[0]),"Zero pending counter was accepted");check((*memory)[0]==before,"Invalid pending clear mutated memory");
 check(!spring246Reset(state,(*memory)[0],2,pending[0]),"Out-of-domain slot accepted");
 std::cout<<"PASS: Spring 8 resets / 16 deferred callbacks; 24 complete states, audio/display, counters and dual-slot workspaces match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
