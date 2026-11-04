// SPDX-License-Identifier: AGPL-3.0-only
#include "EnvelopePublisher246.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool b,const char* why){if(!b)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing publisher fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const unsigned v=p[0]|unsigned(p[1])<<8;return int16_t(v<32768?int(v):int(v)-65536);}
int main(int argc,char** argv){try{using namespace pocket::reference;
 check(argc==2,"Provide fixture directory");const auto input=read(std::filesystem::path(argv[1])/"envelope-publisher246-input.bin"),output=read(std::filesystem::path(argv[1])/"envelope-publisher246-output.bin");check(input.size()==4096*280&&output.size()==4096*312,"Incomplete publisher fixtures");
 std::array<EnvelopePublisher246State,4> states{};int32_t published=0;
 for(size_t n=0;n<4096;++n){const auto* x=input.data()+n*280;const auto* y=output.data()+n*312;const auto action=word(x);auto& state=states[(action>>2)&3u];
  if(action&1){state.first=signedPhase(word(x+4));state.second=signedPhase(word(x+8));published=signedPhase(word(x+12));}if(action&2)EnvelopePublisher246::resetPublished(published);
  std::array<int16_t,4> controls{};std::array<int16_t,128> audio{};for(size_t i=0;i<4;++i)controls[i]=half(x+16+i*2);for(size_t i=0;i<128;++i)audio[i]=half(x+24+i*2);
  EnvelopePublisher246::process(state,published,controls,audio);
  std::array<unsigned char,312> actual{};static_assert(sizeof(states)==32);std::memcpy(actual.data(),states.data(),32);std::memcpy(actual.data()+32,&published,4);const uint32_t getter=0xa5b60000u|uint16_t(EnvelopePublisher246::readPublished(published));std::memcpy(actual.data()+36,&getter,4);std::memcpy(actual.data()+40,controls.data(),8);std::memcpy(actual.data()+48,audio.data(),256);const uint32_t guard=0x2468ace0u;std::memcpy(actual.data()+304,&guard,4);std::memcpy(actual.data()+308,&action,4);
  if(std::memcmp(actual.data(),y,actual.size())){for(size_t i=0;i<actual.size();++i)if(actual[i]!=y[i]){std::cerr<<"Publisher block "<<n<<" owner "<<((action>>2)&3)<<" byte "<<i<<" expected "<<int(y[i])<<" got "<<int(actual[i])<<'\n';break;}throw std::runtime_error("Original publisher state/global/audio mismatch");}
 }
 // The word is not an amplitude-only detector: silence can publish a nonzero
 // gain-like value. Resetting it must not reset the independent caller states.
 EnvelopePublisher246State state;int32_t value=0;std::array<int16_t,128> silence{};
 for(int n=0;n<100;++n)EnvelopePublisher246::process(state,value,{{0,16384,0,0}},silence);
 check(value>1000000000&&state.first==0&&state.second==0,"Silence publication property mismatch");for(auto v:silence)check(v==0,"Silence became audio");
 state.first=123;state.second=456;EnvelopePublisher246::resetPublished(value);check(value==0&&state.first==123&&state.second==456,"Publisher reset changed caller state");
 std::cout<<"PASS: 4096 original blocks / 524288 audio samples, four caller states, shared publisher, reset/getter, controls and guard; nonzero silence publication\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
