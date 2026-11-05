// SPDX-License-Identifier: AGPL-3.0-only
#include "Manual246FxAudio.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Crank fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const unsigned x=p[0]|(unsigned(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{check(argc==2,"Provide unpacked fixture directory");using namespace pocket::reference;
 for(bool bend:{true,false}){const std::string name=bend?"bend":"crank";const size_t stride=bend?96:128,outStride=bend?1108:1140,cpos=bend?64:96,ret=bend?80:112,buf=ret+4;Bend246State bendState;
 const auto in=read(std::filesystem::path(argv[1])/(name+"-fx246-input.bin")),out=read(std::filesystem::path(argv[1])/(name+"-fx246-output.bin"));check(in.size()==8192*stride&&out.size()==8192*outStride,"Incomplete manual FX fixtures");Crank246State state;uint32_t gainState=0;
 for(size_t n=0;n<8192;++n){const auto* x=in.data()+n*stride;const auto* y=out.data()+n*outStride;const auto action=word(x+12);
  if(bend){if(action&1){std::memcpy(bendState.words.data(),x+20,32);std::memcpy(bendState.history.data(),x+52,8);bendState.display=word(x+60);}if(action&2)bend246Prepare(bendState);if(action&4)bend246ResetInput(bendState);}
  else{if(action&1){std::memcpy(state.words.data(),x+20,28);std::memcpy(state.history.data(),x+48,40);state.previous=int32_t(word(x+88));gainState=word(x+92);}if(action&2)crank246Prepare(state);if(action&4)crank246ResetInput(state);}
  std::array<int16_t,8> c{},targets{};for(size_t i=0;i<8;++i){c[i]=half(x+cpos+i*2);targets[i]=half(x+cpos+16+i*2);}
  std::array<uint32_t,128> pitch{};std::array<int16_t,128> gain{};for(size_t i=0;i<128;++i){pitch[i]=0x01234567u*uint32_t(i+1);const auto v=0x04030807u*uint32_t(i/2+1);gain[i]=half(reinterpret_cast<const unsigned char*>(&v)+(i%2)*2);}const auto inputPitch=pitch;const auto inputGain=gain;
  std::array<int16_t,128> audio{};for(size_t i=0;i<128;++i){const uint32_t v=0x09020705u*uint32_t(i/2+1);audio[i]=half(reinterpret_cast<const unsigned char*>(&v)+(i%2)*2);}const auto inputAudio=audio;
  const uint32_t accepted=bend?bend246FxAudioStep(bendState,c,int(word(x+8)),audio):crank246FxAudioStep(state,c,int(word(x+8)),audio);
  if(accepted){const pocket::Manual246FxAudio bridge{bend?saturate16(int(int16_t(bendState.words[0]))*256):saturate16((crank246Level(state)-64)*512),c[bend?2:1],int(c[bend?0:3]/4096)};for(size_t i=0;i<64;++i)check(bridge.apply(inputAudio[i])==audio[i],"FX sample bridge differs from original");}
  std::vector<unsigned char> actual(outStride);std::memcpy(actual.data(),x+16,4);
  if(bend){std::memcpy(actual.data()+4,bendState.words.data(),32);std::memcpy(actual.data()+36,bendState.history.data(),8);std::memcpy(actual.data()+44,&bendState.display,4);}
  else{std::memcpy(actual.data()+4,state.words.data(),28);std::memcpy(actual.data()+32,state.history.data(),40);std::memcpy(actual.data()+72,&state.previous,4);std::memcpy(actual.data()+76,&gainState,4);}
  std::memcpy(actual.data()+cpos-16,c.data(),16);std::memcpy(actual.data()+cpos,targets.data(),16);std::memcpy(actual.data()+ret,&accepted,4);
  for(size_t i=0;i<256;++i){const uint32_t v=i<128?0x01234567u*uint32_t(i+1):i<192?0x04030807u*uint32_t(i-127):0x09020705u*uint32_t(i-191);std::memcpy(actual.data()+buf+i*4,&v,4);}
  std::memcpy(actual.data()+buf,pitch.data(),512);std::memcpy(actual.data()+buf+512,gain.data(),256);
  std::memcpy(actual.data()+buf+768,audio.data(),256);
  if(std::memcmp(actual.data(),y,outStride)){for(size_t i=0;i<outStride;++i)if(actual[i]!=y[i]){std::cerr<<name<<" call "<<n<<" family "<<word(x)<<" type "<<word(x+4)<<" kind "<<word(x+8)<<" byte "<<i<<" expected "<<int(y[i])<<" got "<<int(actual[i])<<'\n';break;}throw std::runtime_error("Original Crank source/controls/targets/return/buffer mismatch");}
 }
 }std::cout<<"PASS: 16384 Bend/Crank FX callbacks, shared source state, all controls/targets and exact pitch/gain/audio buffers plus sample bridge\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
