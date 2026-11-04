// SPDX-License-Identifier: AGPL-3.0-only
#include "Random246FxControl.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& path){std::ifstream f(path,std::ios::binary);check(bool(f),"Missing Random fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){return int16_t(uint16_t(p[0])|(uint16_t(p[1])<<8));}
int main(int argc,char** argv){try{
 check(argc==2,"Provide reference fixture directory");const std::filesystem::path root(argv[1]);
 const auto in=read(root/"random-fx246-input.bin"),out=read(root/"random-fx246-output.bin");
 check(in.size()==4096*88&&out.size()==4096*72,"Wrong Random fixture sizes");
 pocket::reference::Random246State state;uint32_t rng=0,meter=0;int mismatches=0,accepted=0;
 for(size_t n=0;n<4096;++n){const auto* x=in.data()+n*88;const auto* y=out.data()+n*72;const auto meta=word(x+84);
  if(meta&255){std::memcpy(&state,x,44);rng=word(x+44);meter=word(x+48);}
  std::array<int16_t,8> c{},targets{};for(size_t i=0;i<8;++i){c[i]=half(x+52+i*2);targets[i]=half(x+68+i*2);}
  if((meta&255)==2)state.noteOn(c[3]);
  pocket::Random246FxControl bridge;bridge.state=state;bridge.meter=meter;uint32_t bridgeRng=rng;const auto bridgeBase=targets;
  const uint32_t result=pocket::reference::random246FxStep(state,rng,meter,c,int((meta>>8)&255),int(meta>>16),targets)?1u:0u;accepted+=int(result);
  if(result){const auto actual=bridge.next(true,{c[0],c[1],4096,c[3]},int(meta>>16),1,bridgeRng,{bridgeBase[0],bridgeBase[1],bridgeBase[2],bridgeBase[3]});check(std::equal(actual.begin(),actual.end(),targets.begin())&&std::memcmp(&bridge.state,&state,44)==0&&bridgeRng==rng&&bridge.meter==meter,"Random FX host bridge differs from original fixture");}
  std::array<unsigned char,72> actual{};std::memcpy(actual.data(),&state,44);std::memcpy(actual.data()+44,&rng,4);std::memcpy(actual.data()+48,&meter,4);std::memcpy(actual.data()+52,targets.data(),16);std::memcpy(actual.data()+68,&result,4);
  if(std::memcmp(actual.data(),y,72)!=0){if(mismatches++<12){for(size_t i=0;i<72;++i)if(actual[i]!=y[i]){std::cerr<<"case "<<n<<" byte "<<i<<" actual "<<int(actual[i])<<" expected "<<int(y[i])<<" controls "<<c[0]<<','<<c[1]<<','<<c[2]<<','<<c[3]<<'\n';break;}}}
 }
 check(mismatches==0,"Random FX original state/RNG/meter/target mismatch");
 std::cout<<"PASS: 4096 original Random FX callbacks ("<<accepted<<" accepted); complete 44-byte self, RNG, meter, eight targets and return\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
