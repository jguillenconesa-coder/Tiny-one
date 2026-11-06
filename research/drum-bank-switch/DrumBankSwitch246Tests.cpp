// SPDX-License-Identifier: AGPL-3.0-only
#include "DrumBankSwitch246.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<uint8_t>;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing drum bank-switch fixture");return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv){try{
 using M=pocket::research::DrumBankSwitch246;check(argc==2,"Provide fixture directory");const std::filesystem::path root(argv[1]);const auto in=read(root/"drum-bank-switch246-input.bin"),out=read(root/"drum-bank-switch246-output.bin");constexpr size_t cases=2048,stride=8828,result=8928;check(in.size()==cases*stride&&out.size()==cases*result,"Wrong switch fixture dimensions");M::Image image{};M::Stats stats;std::array<bool,448> grid{};size_t reloads=0,retained=0,changed=0,same=0,noop=0,empty=0,duplicateCases=0;
 for(size_t n=0;n<cases;++n){const auto* x=in.data()+n*stride;const auto flags=M::get(x),ui=M::get(x+4),old=M::get(x+8),target=M::get(x+12),family=M::get(x+16),oldType=M::get(x+20),held=M::get(x+24),delay=M::get(x+28);check(flags<2&&(ui==1||ui==2)&&old<8&&target<8&&family<7&&oldType<7&&held<=24&&delay>=1&&delay<=4,"Invalid supplied switch profile");
  if(flags){++reloads;std::copy_n(x+32,image.size(),image.begin());}else ++retained;
  constexpr std::array<std::array<size_t,2>,6> patches{{{{1164,1024}},{{2188,32}},{{2252,2564}},{{4816,812}},{{5628,3072}},{{44,8}}}};for(auto span:patches)std::copy_n(x+32+span[0],span[1],image.begin()+span[0]);M::put(image.data()+4,old);M::put(image.data()+8716,ui);check(M::get(image.data()+4812)==held&&M::get(image.data()+2032)==old,"Caller setup differs");check(M::constructDrum(image,old,oldType),"Old-bank setup rejected");std::copy_n(x+32+224,256,image.begin()+224);M::Trace trace{};unsigned calls=0;const auto beforeBusy=image[8704];
  check(M::change(image,target,delay,trace,calls,stats),"Bank changer model rejected valid profile");if(ui==2){++noop;check(calls==0&&image[8704]==beforeBusy,"No-op mode touched service flag");}else{grid[(old*8+target)*7+family]=true;if(old==target){++same;check(calls==0&&image[8704]==beforeBusy,"Same bank requested service");}else{++changed;empty+=held==0;duplicateCases+=held>12;check(calls==delay&&M::get(image.data()+4)==target&&image[8704]==0,"Changed bank did not complete");}}
  Bytes expected(image.begin(),image.end());expected.insert(expected.end(),trace.begin(),trace.end());const auto offset=expected.size();expected.resize(offset+4);M::put(expected.data()+offset,calls);check(expected.size()==result,"Internal switch result size differs");const auto* y=out.data()+n*result;for(size_t i=0;i<result;++i)if(expected[i]!=y[i]){std::cerr<<"case "<<n<<" UI/banks "<<ui<<'/'<<old<<'/'<<target<<" byte "<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(y[i])<<'\n';throw std::runtime_error("Complete original drum bank changer differs");}
 }
 for(bool seen:grid)check(seen,"Missing bank/family combination");check(changed>0&&same>0&&noop>0&&empty>0&&duplicateCases>0&&retained>0,"Missing change boundary coverage");for(size_t f=0;f<7;++f)check(stats.resets[f]>0,"Missing original reset family");
 std::cout<<"PASS: 2048 original bank-change entries, "<<reloads<<" reloads / "<<retained<<" retained; "<<changed<<" changed / "<<same<<" same-bank / "<<noop<<" UI-2 no-op, "<<stats.service<<" authored service calls, "<<stats.held<<" real reader resets, "<<stats.primary<<" primary / "<<stats.secondary<<" secondary reset bodies; family resets";for(auto c:stats.resets)std::cout<<' '<<c;std::cout<<"; "<<empty<<" empty / "<<duplicateCases<<" duplicate-voice held cases; full 448 bank/family grid, windows/controls/registry/traces match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
