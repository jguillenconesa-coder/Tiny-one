// SPDX-License-Identifier: AGPL-3.0-only
#include "CoreModulatorReset246.h"
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
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Core reset fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t get(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static void put(uint8_t* p,uint32_t value,unsigned bytes=4){for(unsigned i=0;i<bytes;++i)p[i]=uint8_t(value>>(i*8));}
static int16_t half(const uint8_t* p){const auto word=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(word<32768?int(word):int(word)-65536);}
template<class State,class Reset>static void apply(uint8_t* p,Reset&& reset){State state;std::memcpy(&state,p,sizeof(state));reset(state);std::memcpy(p,&state,sizeof(state));}
int main(int argc,char** argv){try{using namespace pocket::reference;using pocket::research::CoreModulatorReset246;
 check(argc==2,"Provide uncompressed reset fixtures");const std::filesystem::path root(argv[1]);const auto input=read(root/"core-modulator-reset246-input.bin"),output=read(root/"core-modulator-reset246-output.bin");constexpr size_t cases=8192,stride=740,result=732;check(input.size()==cases*stride&&output.size()==cases*result,"Wrong reset fixture dimensions");std::array<size_t,6> calls{};size_t accepted=0,clears=0,mutations=0,secondaryCalls=0;std::array<bool,2*12*8*6> coverage{};
 for(size_t n=0;n<cases;++n){const auto* x=input.data()+n*stride;const bool drum=get(x)!=0;const unsigned voice=get(x+4),sound=get(x+8),first=get(x+12),second=get(x+16),actions=get(x+28),newSound=get(x+32);const std::array<int,2> adjustment{{signedPhase(get(x+20)),signedPhase(get(x+24))}};check(voice<(drum?12u:6u)&&sound<8&&first<6&&second<6&&newSound<8,"Invalid supplied dispatcher case");for(int delta:adjustment)check(delta>=-16&&delta<=16&&delta%8==0,"This adjustment outside bounded proxy memory");coverage[((drum?1:0)*12*8*6)+voice*8*6+sound*6+first]=true;Bytes expected(x+36,x+stride);put(expected.data()+152+64,0x02010100);put(expected.data()+280+64,0x02010120);
  CoreModulatorReset246 model;std::copy_n(expected.begin(),68,model.globals.begin());for(size_t i=0;i<21;++i)model.pointers[i]=get(expected.data()+68+i*4);model.put(drum?4:0,sound);model.pointers[(drum?9:3)+voice]=first?0x02010040u:0;model.put(drum?60:56,second?0x020100c0u:0);if(actions&1){model.clear(drum);++clears;}
  std::array<uint32_t,7> trace{};auto invoke=[&](uint32_t object,uint32_t controls){const unsigned role=object==0x02010040?0:object==0x020100c0?1:2;check(role<2,"Unknown supplied proxy object");const auto kind=role?second:first;check(kind>0&&kind<6,"Absent callback invoked");++calls[kind];++trace[0];const auto selector=model.get(drum?4:0);trace[1+role*3]=object+uint32_t(adjustment[role]);trace[2+role*3]=controls;trace[3+role*3]=selector;const uint32_t base=drum?0xff801644u:0xff8014c4u;check(controls>=base&&(controls-base)%16==0&&(controls-base)/16<8,"Wrong original control pointer");std::array<int16_t,8> c{};const size_t controlOffset=(drum?568:440)+(controls-base);for(size_t i=0;i<8;++i)c[i]=half(expected.data()+controlOffset+i*2);
   if(role==0&&(actions&2)){model.put(drum?4:0,newSound);++mutations;}if(role)++secondaryCalls;auto* state=expected.data()+(role?280:152)+64+adjustment[role];
   if(kind<=2)apply<Value246State>(state,[&](auto& s){s.noteOn(c);});else if(kind<=4)apply<Random246State>(state,[&](auto& s){s.noteOn(c[3]);});else{std::array<uint32_t,8> display{};for(size_t i=0;i<8;++i)display[i]=get(expected.data()+408+i*4);apply<Tremolo246State>(state,[&](auto& s){tremolo246NoteOn(s,display,c);});for(size_t i=0;i<8;++i)put(expected.data()+408+i*4,display[i]);}
  };
  if(model.reset(drum,voice,invoke))++accepted;std::copy(model.globals.begin(),model.globals.end(),expected.begin());for(size_t i=0;i<21;++i)put(expected.data()+68+i*4,model.pointers[i]);for(auto word:trace){const auto pos=expected.size();expected.resize(pos+4);put(expected.data()+pos,word);}check(expected.size()==result,"Wrong modeled snapshot size");const auto* y=output.data()+n*result;for(size_t i=0;i<result;++i)if(expected[i]!=y[i]){std::cerr<<"case "<<n<<" byte "<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(y[i])<<'\n';throw std::runtime_error("Complete Core reset route differs");}
 }
 for(unsigned route=0;route<2;++route)for(unsigned voice=0;voice<(route?12u:6u);++voice)for(unsigned sound=0;sound<8;++sound)for(unsigned kind=0;kind<6;++kind)check(coverage[route*12*8*6+voice*8*6+sound*6+kind],"Incomplete route/slot/sound/reset coverage");
 for(size_t i=1;i<6;++i)check(calls[i]>0,"Original reset family not covered");check(accepted>0&&accepted<cases&&clears>0&&mutations>0&&secondaryCalls>0,"Dispatcher boundary absent");std::cout<<"PASS: 8192 original Core reset routes; "<<accepted<<" accepted, "<<clears<<" clears, "<<mutations<<" synthetic selector mutations, "<<secondaryCalls<<" secondary resets; five original reset bodies, signed this and full state/guards\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
