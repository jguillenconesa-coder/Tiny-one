// SPDX-License-Identifier: AGPL-3.0-only
#include "SynthHeldBankSwitch246.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdexcept>
using Bytes=std::vector<uint8_t>;
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Missing fixture");return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv){try{
 using M=pocket::research::SynthHeldBankSwitch246;if(argc!=2)throw std::runtime_error("Provide fixture directory");
 const auto root=std::filesystem::path(argv[1]);auto in=read(root/"synth-held-bank-switch246-input.bin"),out=read(root/"synth-held-bank-switch246-output.bin");
 if(in.size()!=2048*4164u||out.size()!=2048*4256u)throw std::runtime_error("Fixture size");
 M::Stats stats;size_t same=0,changed=0,service=0,deferred=0,initialized=0;std::array<bool,64> grid{};std::array<bool,129> lengths{};std::array<bool,5> modes{};std::array<bool,8> voices{};
 for(size_t n=0;n<2048;++n){const auto* x=in.data()+n*4164;const auto target=M::get(x),delay=M::get(x+4);M::Image m;std::copy_n(x+8,4156,m.begin());const auto old=M::get(m.data()+76),before=M::get(m.data()+188),active=M::get(m.data()+148);M::Trace trace;unsigned calls=0;
  const auto held=M::get(m.data()+344);if(held>128)throw std::runtime_error("List size");lengths[held]=true;const auto mode=unsigned(M::half(m.data()+348+target*16+8))/4096;if(mode>=5)throw std::runtime_error("Mode");modes[mode]=true;for(size_t i=0;i<held;++i){const auto voice=pocket::reference::signedPhase(M::get(m.data()+764+i*20));if(voice<-2||voice>=6)throw std::runtime_error("Voice");voices[size_t(voice+2)]=true;}
  if(!M::change(m,target,delay,trace,calls,stats))throw std::runtime_error("Profile rejected");
  grid[old*8+target]=true;service+=calls;
  if(old==target)++same;else{++changed;initialized+=active==0;deferred+=before!=M::get(m.data()+188);}
  Bytes y(m.begin(),m.end());y.insert(y.end(),trace.begin(),trace.end());y.resize(4256);M::put(y.data()+4252,calls);
  for(size_t i=0;i<4256;++i)if(y[i]!=out[n*4256+i]){std::cerr<<"case "<<n<<" byte "<<i<<" model "<<unsigned(y[i])<<" original "<<unsigned(out[n*4256+i])<<'\n';throw std::runtime_error("Original changer differs");}
 }
 for(bool b:grid)if(!b)throw std::runtime_error("Missing bank pair");
 for(bool b:lengths)if(!b)throw std::runtime_error("Missing list length");for(bool b:modes)if(!b)throw std::runtime_error("Missing selector");for(bool b:voices)if(!b)throw std::runtime_error("Missing voice index");
 if(!stats.translated||!stats.refreshEntries||!stats.voiceResets)throw std::runtime_error("Missing held-note effects");
 if(!deferred||!initialized||!same)throw std::runtime_error("Missing boundaries");
 std::cout<<"PASS: 2048 original synth entries, "<<changed<<" changes / "<<same<<" same-bank, "<<service<<" authored calls, "<<initialized<<" translation initializations / "<<deferred<<" deferred pitch updates; "<<stats.translated<<" MIDI-helper pairs / "<<stats.refreshEntries<<" refresh entries / "<<stats.voiceResets<<" Dr. Wave resets / "<<stats.envelopeResets<<" fast releases; held translation and refresh, all windows match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
