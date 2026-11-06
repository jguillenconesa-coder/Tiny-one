// SPDX-License-Identifier: AGPL-3.0-only
#include "CoreSignal246.h"
#include "../core-initial-state/CoreInitialState246.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<uint8_t>;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Core signal fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t get(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const uint8_t* p){const auto value=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(value<32768?int(value):int(value)-65536);}
static void put(uint8_t* p,uint32_t value,unsigned bytes=4){for(unsigned i=0;i<bytes;++i)p[i]=uint8_t(value>>(i*8));}
int main(int argc,char** argv){try{using namespace pocket::reference;
 check(argc==2,"Provide uncompressed Core signal fixtures");const std::filesystem::path root(argv[1]);const auto input=read(root/"core-signal246-input.bin"),output=read(root/"core-signal246-output.bin");constexpr size_t cases=4864,stride=332,result=328;check(input.size()==cases*stride&&output.size()==cases*result,"Wrong Core signal fixture size");
 Bytes history(28);size_t independent=0,retained=0,initialized=0;
 for(size_t n=0;n<cases;++n){const auto* x=input.data()+n*stride;const auto* y=output.data()+n*result;const auto mode=get(x);check(mode==(n<4096?1u:n<4608?0u:2u),"Unexpected reset sequence");if(mode){std::copy_n(x+16,28,history.begin());if(mode==1)++independent;else ++initialized;}else ++retained;
  Bytes expected(x+4,x+stride);std::copy(history.begin(),history.end(),expected.begin()+12);
  if(mode==2){std::array<uint8_t,5856> image{};constexpr size_t stateOffset=2024+0x01aaac0c-0x01aaabd4,coefOffset=2024+3560;std::copy_n(expected.begin(),12,image.begin()+coefOffset);std::copy(history.begin(),history.end(),image.begin()+stateOffset);pocket::research::coreInitialState246(image.data());std::copy_n(image.begin()+coefOffset,12,expected.begin());std::copy_n(image.begin()+stateOffset,28,history.begin());std::copy(history.begin(),history.end(),expected.begin()+12);}
  pocket::research::CoreSignal246 model;model.previousInput=signedPhase(get(history.data()+8));model.previousQuantized=signedPhase(get(history.data()+12));model.previousOutput=signedPhase(get(history.data()+16));std::array<int16_t,64> signal{},audio{};for(size_t i=0;i<64;++i)signal[i]=half(x+52+i*2);
  model.block(signedPhase(get(expected.data()+4)),signal,audio);put(expected.data()+20,uint32_t(model.previousInput));put(expected.data()+24,uint32_t(model.previousQuantized));put(expected.data()+28,uint32_t(model.previousOutput));for(size_t i=0;i<64;++i)put(expected.data()+192+i*2,uint16_t(audio[i]),2);
  for(size_t i=0;i<result;++i)if(expected[i]!=y[i]){std::cerr<<"case "<<n<<" byte "<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(y[i])<<'\n';throw std::runtime_error("Complete Core signal callback differs");}
  std::copy_n(expected.begin()+12,28,history.begin());
 }
 std::cout<<"PASS: "<<independent<<" independent / "<<retained<<" retained / "<<initialized<<" initializer-composed original Core callbacks; 311296 output words, state/coefficient/input/output and guards all match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
