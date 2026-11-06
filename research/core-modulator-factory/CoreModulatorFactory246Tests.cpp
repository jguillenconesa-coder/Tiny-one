// SPDX-License-Identifier: AGPL-3.0-only
#include "CoreModulatorFactory246.h"
#include "Value246.h"
#include "Random246.h"
#include "Tremolo246.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<uint8_t>;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing factory fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const uint8_t* p){const auto v=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(v<32768?int(v):int(v)-65536);}
static void put(uint8_t* p,uint32_t value){for(unsigned i=0;i<4;++i)p[i]=uint8_t(value>>(i*8));}
template<class State,class F>static void apply(uint8_t* p,F&& f){State s;std::memcpy(&s,p,sizeof(s));f(s);std::memcpy(p,&s,sizeof(s));}
int main(int argc,char** argv){try{
 using namespace pocket::research;using namespace pocket::reference;
 check(argc==2,"Provide factory fixtures directory");const std::filesystem::path root(argv[1]);const auto in=read(root/"core-modulator-factory246-input.bin"),out=read(root/"core-modulator-factory246-output.bin");constexpr size_t cases=3072,stride=1088,result=1068;
 check(in.size()==cases*stride&&out.size()==cases*result,"Wrong factory fixture dimensions");Bytes expected(result);CoreModulatorFactory246 m;std::array<bool,432> grid{};size_t reloads=0,retained=0,factories=0,clears=0,dispatches=0,accepted=0,primary=0,secondary=0,retainedTypeChanges=0,retainedRouteChanges=0;std::array<size_t,3> constructed{};
 for(size_t n=0;n<cases;++n){const auto* x=in.data()+n*stride;const auto flags=word(x),drum=word(x+4),type=word(x+8),voice=word(x+12),sound=word(x+16);check(drum<2&&type<3&&voice<(drum?12u:6u)&&sound<8,"Invalid supplied factory domain");
  if(n&&!(flags&1)){const auto* previous=in.data()+(n-1)*stride;retainedTypeChanges+=word(previous+8)!=type;retainedRouteChanges+=word(previous+4)!=drum;}
  if(flags&1){++reloads;std::copy_n(x+20,result,expected.begin());}else ++retained;
  std::copy_n(expected.begin(),68,m.core.globals.begin());for(size_t i=0;i<21;++i)m.core.pointers[i]=word(expected.data()+68+i*4);std::copy_n(expected.begin()+152,588,m.pool.begin());
  m.core.put(drum?4:0,sound);m.core.globals[(drum?44:28)+sound]=uint8_t(n<768||sound!=0?0xa5:0);
  if(flags&2){++factories;check(m.factory(drum!=0,type),"Valid factory rejected");constructed[type]+=drum?12:6;}
  if(flags&4){++clears;m.core.clear(drum!=0);}
  if(flags&8){++dispatches;const bool ok=m.core.reset(drum!=0,voice,[&](uint32_t pointer,uint32_t parameters){auto* p=m.object(pointer);check(p!=nullptr,"Reset outside real pool");const auto bankBase=drum?0xff801644u:0xff8014c4u;check(parameters>=bankBase&&parameters-bankBase<128&&(parameters-bankBase)%16==0,"Unexpected parameter bank");std::array<int16_t,8> c{};for(size_t i=0;i<8;++i)c[i]=half(expected.data()+(drum?868:740)+(parameters-bankBase)+i*2);const auto tag=word(p);if(pointer==0xffb00c10u)++secondary;else ++primary;
   if(tag==0x0191097c||tag==0x0191099c||tag==0x019109bc)apply<Value246State>(p,[&](auto& s){s.noteOn(c);});
   else if(tag==0x0191090c||tag==0x0191092c||tag==0x0191094c)apply<Random246State>(p,[&](auto& s){s.noteOn(c[3]);});
   else{check(tag==0x019107fc,"Unknown original reset vtable");std::array<uint32_t,8> display{};for(size_t i=0;i<8;++i)display[i]=word(expected.data()+1028+i*4);apply<Tremolo246State>(p,[&](auto& s){tremolo246NoteOn(s,display,c);});for(size_t i=0;i<8;++i)put(expected.data()+1028+i*4,display[i]);}
  });if(ok){++accepted;grid[(drum?144:0)+type*(drum?96:48)+sound*(drum?12:6)+voice]=true;}}
  std::copy(m.core.globals.begin(),m.core.globals.end(),expected.begin());for(size_t i=0;i<21;++i)put(expected.data()+68+i*4,m.core.pointers[i]);std::copy(m.pool.begin(),m.pool.end(),expected.begin()+152);
  const auto* y=out.data()+n*result;for(size_t i=0;i<result;++i)if(expected[i]!=y[i]){std::cerr<<"case "<<n<<" route/type "<<drum<<'/'<<type<<" byte "<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(y[i])<<'\n';throw std::runtime_error("Complete original modulator factory differs");}
 }
 for(bool seen:grid)check(seen,"Missing accepted route/type/sound/voice combination");check(reloads==1584&&retained==1488&&accepted>0&&secondary>0&&clears>0,"Missing retained/clear/reset coverage");check(retainedTypeChanges==12&&retainedRouteChanges==4,"Missing retained selector transitions");for(auto count:constructed)check(count>0,"Missing original constructor family");const auto savedPool=m.pool;const auto savedGlobals=m.core.globals;const auto savedPointers=m.core.pointers;check(!m.factory(true,3)&&!m.factory(false,7),"Authored domain guard failed");check(m.pool==savedPool&&m.core.globals==savedGlobals&&m.core.pointers==savedPointers,"Invalid factory mutated memory");
 std::cout<<"PASS: 3072 original factory scenarios, "<<reloads<<" reloads / "<<retained<<" retained; "<<factories<<" factories, "<<clears<<" clears, "<<dispatches<<" resets requested / "<<accepted<<" accepted, "<<primary<<" primary / "<<secondary<<" secondary bodies; constructors by family "<<constructed[0]<<'/'<<constructed[1]<<'/'<<constructed[2]<<"; 12 retained family / 4 route changes; full 432 accepted route/type/bank/voice grid\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
