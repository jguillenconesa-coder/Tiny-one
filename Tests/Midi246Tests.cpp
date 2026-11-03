// SPDX-License-Identifier: AGPL-3.0-only
#include "Midi246.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing MIDI fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const unsigned x=p[0]|(unsigned(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
 check(argc==2,"Provide unpacked fixture directory");using namespace pocket::reference;
 const auto in=read(std::filesystem::path(argv[1])/"midi246-input.bin"),out=read(std::filesystem::path(argv[1])/"midi246-output.bin");
 check(in.size()==6144*1096&&out.size()==6144*1072,"Incomplete MIDI fixtures");
 for(size_t n=0;n<6144;++n){const auto* x=in.data()+n*1096;const auto* y=out.data()+n*1072;
  std::array<int16_t,8> controls{},targets{};std::array<uint32_t,4> cc{};std::array<int16_t,4> display{};std::array<uint32_t,128> pitch{};std::array<int16_t,128> gain{},audio{};
  for(size_t i=0;i<8;++i){controls[i]=half(x+16+i*2);targets[i]=half(x+56+i*2);}
  for(size_t i=0;i<4;++i){cc[i]=word(x+32+i*4);display[i]=half(x+48+i*2);}
  for(size_t i=0;i<128;++i){pitch[i]=word(x+72+i*4);gain[i]=half(x+584+i*2);audio[i]=half(x+840+i*2);}
  const auto family=word(x);const int type=int(word(x+4)),kind=int(word(x+8));
  const uint32_t accepted=family==0?midi246SynthStep(controls,cc,display,kind,type,targets,pitch,gain):family==1?midi246DrumStep(controls,cc,kind,type,targets):midi246FxStep(controls,cc,kind,type,targets,audio);
  std::array<unsigned char,1072> actual{};std::memcpy(actual.data(),x+12,4);std::memcpy(actual.data()+4,cc.data(),16);std::memcpy(actual.data()+20,display.data(),8);std::memcpy(actual.data()+28,targets.data(),16);std::memcpy(actual.data()+44,pitch.data(),512);std::memcpy(actual.data()+556,gain.data(),256);std::memcpy(actual.data()+812,audio.data(),256);std::memcpy(actual.data()+1068,&accepted,4);
  if(std::memcmp(actual.data(),y,1072)){for(size_t i=0;i<1072;++i)if(actual[i]!=y[i]){std::cerr<<"MIDI case "<<n<<" family "<<family<<" type "<<type<<" kind "<<kind<<" byte "<<i<<" expected "<<int(y[i])<<" got "<<int(actual[i])<<'\n';break;}throw std::runtime_error("Original MIDI callback mismatch");}
 }
 std::cout<<"PASS: 6144 MIDI callbacks, all supplied state/CC/display/targets/pitch/gain/FX audio/returns\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
