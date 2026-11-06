// SPDX-License-Identifier: AGPL-3.0-only
#include "CoreBufferInit246.h"
#include "../core-signal/CoreSignal246.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<uint8_t>;
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Core buffer fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const uint8_t* p){const auto v=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(v<32768?int(v):int(v)-65536);}
static void put(uint8_t* p,uint32_t value,unsigned bytes=4){for(unsigned i=0;i<bytes;++i)p[i]=uint8_t(value>>(8*i));}
int main(int argc,char** argv){try{
 using namespace pocket::research;using namespace pocket::reference;
 check(argc==2,"Provide fixture directory");const std::filesystem::path root(argv[1]);const auto in=read(root/"core-buffer-init246-input.bin"),out=read(root/"core-buffer-init246-output.bin");constexpr size_t cases=1024,stride=1764,result=1760;
 check(in.size()==cases*stride&&out.size()==cases*result,"Wrong Core buffer fixture size");Bytes expected(result);size_t reloads=0,retained=0,inits=0,kernels=0,composed=0,nonzeroAfterInit=0;
 for(size_t n=0;n<cases;++n){const auto* x=in.data()+n*stride;const auto flags=word(x);check(flags<8&&(n!=0||(flags&1)),"Invalid scenario flags");if(flags&1){++reloads;std::copy_n(x+4,result,expected.begin());}else ++retained;
  if(flags&2){++inits;std::array<uint8_t,24> defaults{};std::array<uint8_t,1552> buffers{};std::copy_n(expected.begin(),24,defaults.begin());std::copy_n(expected.begin()+24,1552,buffers.begin());CoreBufferInit246::apply(defaults,buffers);std::copy(defaults.begin(),defaults.end(),expected.begin());std::copy(buffers.begin(),buffers.end(),expected.begin()+24);}
  if(flags&4){++kernels;CoreSignal246 model;model.previousInput=signedPhase(word(expected.data()+1584));model.previousQuantized=signedPhase(word(expected.data()+1588));model.previousOutput=signedPhase(word(expected.data()+1592));std::array<int16_t,64> signal{},audio{};for(size_t i=0;i<64;++i)signal[i]=half(expected.data()+1312+i*2);model.block(signedPhase(word(expected.data()+1608)),signal,audio);put(expected.data()+1584,uint32_t(model.previousInput));put(expected.data()+1588,uint32_t(model.previousQuantized));put(expected.data()+1592,uint32_t(model.previousOutput));bool nonzero=false;for(size_t i=0;i<64;++i){put(expected.data()+1624+i*2,uint16_t(audio[i]),2);nonzero|=audio[i]!=0;}if(flags&2){++composed;nonzeroAfterInit+=nonzero;}}
  const auto* y=out.data()+n*result;for(size_t i=0;i<result;++i)if(expected[i]!=y[i]){std::cerr<<"case "<<n<<" byte "<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(y[i])<<'\n';throw std::runtime_error("Original Core buffer composition differs");}
 }
 check(reloads==528&&retained==496&&inits==512&&kernels==512&&composed==256&&nonzeroAfterInit>0,"Scenario coverage differs");std::cout<<"PASS: 1024 scenarios, 528 reloads / 496 retained, 512 original buffer initializations, 512 kernel blocks / 32768 samples, 256 composed; "<<nonzeroAfterInit<<" composed blocks remain nonzero with cleared input and retained history\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
