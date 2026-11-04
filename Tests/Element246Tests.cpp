// SPDX-License-Identifier: AGPL-3.0-only
#include "Element246.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Element fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const unsigned v=p[0]|unsigned(p[1])<<8;return int16_t(v<32768?int(v):int(v)-65536);}
int main(int argc,char** argv){try{using namespace pocket::reference;check(argc==2,"Provide unpacked fixture directory");
 const auto input=read(std::filesystem::path(argv[1])/"element246-input.bin"),output=read(std::filesystem::path(argv[1])/"element246-output.bin");check(input.size()==18432*444&&output.size()==18432*1204,"Incomplete Element fixtures");
 for(size_t n=0;n<18432;++n){const auto* x=input.data()+n*444;const auto* y=output.data()+n*1204;std::array<int16_t,8> controls{},targets{};std::array<int16_t,64> external{};std::array<int16_t,128> gain{};
  for(size_t i=0;i<8;++i){controls[i]=half(x+16+i*2);targets[i]=half(x+32+i*2);}for(size_t i=0;i<64;++i)external[i]=half(x+60+i*2);for(size_t i=0;i<128;++i)gain[i]=half(x+188+i*2);
  Element246Inputs in{half(x+48),element246PositivePeak(external),element246PositivePeak(gain),half(x+50),half(x+52),half(x+58)};
  const uint32_t accepted=element246Apply(in,controls,int(word(x)),int(word(x+8)),int(word(x+4)),targets);
  std::array<unsigned char,1204> actual{};std::memcpy(actual.data(),x+12,4);std::memcpy(actual.data()+4,controls.data(),16);std::memcpy(actual.data()+20,targets.data(),16);std::memcpy(actual.data()+36,&accepted,4);
  const uint32_t sensorWord=uint32_t(uint16_t(in.sensor))*65536u+0x2468u;std::memcpy(actual.data()+40,&sensorWord,4);std::memcpy(actual.data()+44,x+50,4);std::memcpy(actual.data()+48,x+56,4);std::memcpy(actual.data()+52,external.data(),128);std::memcpy(actual.data()+180,gain.data(),256);
  for(size_t i=0;i<192;++i){const uint32_t v=i<128?0x01234567u*uint32_t(i+1):0x09020705u*uint32_t(i-127);std::memcpy(actual.data()+436+i*4,&v,4);}
  if(std::memcmp(actual.data(),y,actual.size())){for(size_t i=0;i<actual.size();++i)if(actual[i]!=y[i]){std::cerr<<"Element call "<<n<<" family "<<word(x)<<" type "<<word(x+4)<<" kind "<<word(x+8)<<" source "<<controls[0]/2048<<" destination "<<controls[2]/2048<<" byte "<<i<<" expected "<<int(y[i])<<" got "<<int(actual[i])<<'\n';break;}throw std::runtime_error("Original Element source/parameter/buffer mismatch");}
 }
 std::cout<<"PASS: 18432 original Element callbacks, four explicit sources, dispatch/controls/targets/return and complete unchanged source/pitch/gain/FX buffers\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
