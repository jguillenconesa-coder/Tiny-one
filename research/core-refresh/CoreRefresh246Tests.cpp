// SPDX-License-Identifier: AGPL-3.0-only
#include "CoreRefresh246.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<uint8_t>;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Core refresh fixture");return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv){try{
 using pocket::research::CoreRefresh246;check(argc==2,"Provide fixture directory");const std::filesystem::path root(argv[1]);const auto in=read(root/"core-refresh246-input.bin"),out=read(root/"core-refresh246-output.bin");constexpr size_t cases=4160,stride=5660,result=5628;check(in.size()==cases*stride&&out.size()==cases*result,"Wrong refresh fixture dimensions");CoreRefresh246::Image image{};std::array<bool,3136> grid{};size_t reloads=0,retained=0,calls=0,skips=0,editDifferences=0,audioAliases=0,modAliases=0;
 for(size_t n=0;n<cases;++n){const auto* x=in.data()+n*stride;const auto flags=CoreRefresh246::get(x),dm=CoreRefresh246::get(x+4),sm=CoreRefresh246::get(x+8),ds=CoreRefresh246::get(x+12),ss=CoreRefresh246::get(x+16),ed=CoreRefresh246::get(x+20),es=CoreRefresh246::get(x+24);check(flags<4&&dm<7&&sm<7&&ds<8&&ss<8&&ed<8&&es<8,"Invalid supplied refresh domain");
  if(flags&1){++reloads;std::copy_n(x+32,result,image.begin());}else ++retained;
  auto put=[&](size_t offset,uint32_t v){CoreRefresh246::put(image.data()+offset,v);};const size_t cfg=1164;put(0,ss);put(4,ds);put(cfg+864,es);put(cfg+868,ed);
  for(size_t b=0;b<8;++b){put(cfg+880+b*4,6);put(cfg+912+b*4,8);put(cfg+960+b*4,0);put(cfg+992+b*4,0);put(2188+b*4,0x02010000u+uint32_t(b)*8);put(2220+b*4,0x02020000u+uint32_t(b)*8);}
  put(cfg+16+es*4,sm);put(cfg+48+ed*4,dm);
  if(flags&2){++calls;grid[((ss*8+ds)*7+sm)*7+dm]=true;check(CoreRefresh246::refresh(image),"Valid refresh rejected");editDifferences+=ss!=es||ds!=ed;
   check(CoreRefresh246::get(image.data()+68)==CoreRefresh246::get(image.data()+92),"Shared audio base lost");audioAliases+=CoreRefresh246::get(image.data()+224)!=0x0190fac4u;
   const auto synthPointer=CoreRefresh246::get(image.data()+140+3*4),drumPointer=CoreRefresh246::get(image.data()+140+9*4);check(synthPointer==drumPointer&&synthPointer==0xffb00a00u,"Shared modulation base lost");modAliases+=sm!=dm;
   check(CoreRefresh246::get(image.data()+140)==CoreRefresh246::get(image.data()+144)&&CoreRefresh246::get(image.data()+504)==0,"FX final synth-route setup lost");
  }else ++skips;
  const auto* y=out.data()+n*result;for(size_t i=0;i<result;++i)if(image[i]!=y[i]){std::cerr<<"case "<<n<<" byte "<<i<<" expected "<<unsigned(image[i])<<" original "<<unsigned(y[i])<<'\n';throw std::runtime_error("Complete original Core refresh differs");}
 }
 for(bool seen:grid)check(seen,"Incomplete modulator/bank grid");check(reloads==3168&&retained==992&&calls==3956&&skips==204&&editDifferences>0&&audioAliases==calls&&modAliases>0,"Refresh coverage differs");auto saved=image;CoreRefresh246::put(image.data(),8);const auto invalid=image;check(!CoreRefresh246::refresh(image)&&image==invalid,"Authored invalid-bank guard mutated state");image=saved;
 std::cout<<"PASS: 4160 refresh scenarios, "<<calls<<" complete original refreshes / "<<skips<<" skips, "<<reloads<<" reloads / "<<retained<<" retained; full 3136 modulator/bank grid, "<<editDifferences<<" editor/Core bank differences, "<<audioAliases<<" audio overlays / "<<modAliases<<" different-family modulation aliases; pools, tables, controls, references, held registry and voice globals match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
