// SPDX-License-Identifier: AGPL-3.0-only
#include "SynthBankSwitch246.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdexcept>
using Bytes=std::vector<uint8_t>;
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Missing fixture");return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv){try{
 using M=pocket::research::SynthBankSwitch246;if(argc!=2)throw std::runtime_error("Provide fixture directory");
 const auto root=std::filesystem::path(argv[1]);auto in=read(root/"synth-bank-switch246-input.bin"),out=read(root/"synth-bank-switch246-output.bin");
 if(in.size()!=2048*484u||out.size()!=2048*576u)throw std::runtime_error("Fixture size");
 size_t same=0,changed=0,service=0,deferred=0,initialized=0;std::array<bool,64> grid{};
 for(size_t n=0;n<2048;++n){const auto* x=in.data()+n*484;const auto target=M::get(x),delay=M::get(x+4);M::Image m;std::copy_n(x+8,476,m.begin());const auto old=M::get(m.data()+76),before=M::get(m.data()+188),active=M::get(m.data()+148);M::Trace trace;unsigned calls=0;
  if(!M::change(m,target,delay,trace,calls))throw std::runtime_error("Profile rejected");
  grid[old*8+target]=true;service+=calls;
  if(old==target)++same;else{++changed;initialized+=active==0;deferred+=before!=M::get(m.data()+188);}
  Bytes y(m.begin(),m.end());y.insert(y.end(),trace.begin(),trace.end());y.resize(576);M::put(y.data()+572,calls);
  for(size_t i=0;i<576;++i)if(y[i]!=out[n*576+i]){std::cerr<<"case "<<n<<" byte "<<i<<" model "<<unsigned(y[i])<<" original "<<unsigned(out[n*576+i])<<'\n';throw std::runtime_error("Original changer differs");}
 }
 for(bool b:grid)if(!b)throw std::runtime_error("Missing bank pair");
 if(!deferred||!initialized||!same)throw std::runtime_error("Missing boundaries");
 std::cout<<"PASS: 2048 original synth entries, "<<changed<<" changes / "<<same<<" same-bank, "<<service<<" authored calls, "<<initialized<<" translation initializations / "<<deferred<<" deferred pitch updates; six envelope resets, all windows match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
