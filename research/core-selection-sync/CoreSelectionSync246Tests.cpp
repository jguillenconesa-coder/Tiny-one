// SPDX-License-Identifier: AGPL-3.0-only
#include "CoreSelectionSync246.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<uint8_t>;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing selection sync fixture");return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv){try{
 using pocket::research::CoreSelectionSync246;using M=CoreSelectionSync246;check(argc==2,"Provide fixture directory");const std::filesystem::path root(argv[1]);const auto in=read(root/"core-selection-sync246-input.bin"),out=read(root/"core-selection-sync246-output.bin");constexpr size_t cases=4096,stride=308,result=392;check(in.size()==cases*stride&&out.size()==cases*result,"Wrong synchronizer fixture dimensions");size_t requests=0,special=0,drumCalls=0,synthCalls=0,ready=0,retargets=0,modeChanges=0;std::array<bool,14> coverage{};
 for(size_t n=0;n<cases;++n){const auto* x=in.data()+n*stride;const auto route=M::get(x),planned=M::get(x+4);check(route<2&&planned<=6,"Invalid authored response schedule");const bool drum=route!=0;coverage[route*7+planned]=true;M model;std::copy_n(x+32,132,model.windows.begin());for(auto pair:{std::array<unsigned,2>{{0,12}},{{4,16}},{{76,20}},{{80,24}},{{108,8}}})M::put(model.windows.data()+pair[0],M::get(x+pair[1]));
  Bytes expected(result,0x5a);unsigned count=0;uint32_t r2=M::get(x+28);
  auto invoke=[&](uint32_t endpoint,uint32_t r0,uint32_t r1,uint32_t incomingR2){check(count<planned,"Outer sync exceeded supplied response count");const auto trace=132+count*40;const std::array<uint32_t,9> fields{{endpoint,r0,r1,incomingR2,model.actual(false),model.actual(true),model.selected(false),model.selected(true),M::get(model.windows.data()+108)}};for(size_t i=0;i<fields.size();++i)M::put(expected.data()+trace+i*4,fields[i]);if(drum)++drumCalls;else ++synthCalls;if(endpoint)++special;else ++requests;
   const auto* step=x+164+count*24;const auto before=model.selected(drum),priorMode=M::get(model.windows.data()+108);for(size_t i=0;i<5;++i){constexpr std::array<size_t,5> offsets{{0,4,76,80,108}};M::put(model.windows.data()+offsets[i],M::get(step+i*4));}r2=M::get(step+20);retargets+=before!=model.selected(drum);modeChanges+=priorMode!=M::get(model.windows.data()+108);++count;
  };
  check(model.sync(drum,r2,invoke),"Finite authored response did not converge");check(count==planned,"Original-domain call count differs");ready+=planned==0;const auto final=model.actual(drum),finalR2=r2;M::put(expected.data()+376,final);M::put(expected.data()+380,r2);check(model.sync(drum,r2,invoke)&&count==planned&&r2==finalR2,"Ready repeated call dispatched or clobbered R2");M::put(expected.data()+384,model.actual(drum));M::put(expected.data()+388,r2);M::put(expected.data()+372,count);std::copy(model.windows.begin(),model.windows.end(),expected.begin());
  const auto* y=out.data()+n*result;for(size_t i=0;i<result;++i)if(expected[i]!=y[i]){std::cerr<<"case "<<n<<" route "<<route<<" byte "<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(y[i])<<'\n';throw std::runtime_error("Original outer synchronizer differs");}
 }
 for(bool seen:coverage)check(seen,"Missing route/delay combination");check(ready>0&&special>0&&requests>0&&retargets>0&&modeChanges>0,"Missing dynamic synchronization branch");M bounded;M::put(bounded.windows.data()+76,1);uint32_t r2=123;unsigned attempted=0;check(!bounded.sync(false,r2,[&](auto,auto,auto,auto){++attempted;},1)&&attempted==1,"Authored budget guard failed");
 std::cout<<"PASS: 4096 original outer sync scenarios / 8192 complete entries; "<<ready<<" initially ready, "<<drumCalls<<" drum / "<<synthCalls<<" synth authored responses, "<<requests<<" generic / "<<special<<" special endpoints, "<<retargets<<" selection / "<<modeChanges<<" mode changes; every repeated ready call bypasses callbacks; windows, argument traces, guards and return observations match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
