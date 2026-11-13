// SPDX-License-Identifier: AGPL-3.0-only
#include "MidiPump246.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<uint8_t>;
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Missing MIDI fixture");return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv){try{
 using M=pocket::research::MidiPump246;if(argc!=2)throw std::runtime_error("Provide fixture directory");const auto root=std::filesystem::path(argv[1]);const auto in=read(root/"midi-pump246-input.bin"),out=read(root/"midi-pump246-output.bin");
 if(in.size()!=4096*5108u||out.size()!=4096*5092u)throw std::runtime_error("Fixture size");M::Stats stats;std::array<bool,5> actions{};std::array<bool,129> lengths{};
 for(size_t n=0;n<4096;++n){const auto* x=in.data()+n*5108;const auto action=M::get(x),count=M::get(x+4),complete=M::get(x+8);M::Image m;std::copy_n(x+20,5088,m.begin());uint32_t returned=0;
  if(!M::run(m,action,count,complete,M::get(x+12),M::get(x+16),returned,stats))throw std::runtime_error("Profile rejected");actions[action]=true;if(action==1)lengths[count]=true;
  Bytes y(m.begin(),m.end());y.resize(5092);M::put(y.data()+5088,returned);
  for(size_t i=0;i<5092;++i)if(y[i]!=out[n*5092+i]){std::cerr<<"case "<<n<<" action "<<action<<" byte "<<i<<" model "<<unsigned(y[i])<<" original "<<unsigned(out[n*5092+i])<<'\n';throw std::runtime_error("Original MIDI flow differs");}
 }
 for(bool b:actions)if(!b)throw std::runtime_error("Missing action");for(bool b:lengths)if(!b)throw std::runtime_error("Missing copy length");for(size_t i=1;i<=32;++i)if(!stats.chunks[i])throw std::runtime_error("Missing contiguous chunk size");for(bool b:stats.buffers)if(!b)throw std::runtime_error("Missing buffer");
 if(!stats.typeRejected||!stats.disabled||!stats.noSlot||!stats.submitted||!stats.submitRejected||!stats.readWraps||!stats.writeWraps||!stats.rejectedBatches)throw std::runtime_error("Missing MIDI boundary");
 std::cout<<"PASS: 4096 original MIDI profiles, "<<stats.pumps<<" pump calls / "<<stats.copyBytes<<" copied bytes / "<<stats.submitted<<" submissions / "<<stats.submitRejected<<" null-endpoint rejects / "<<stats.typeRejected<<" type rejects / "<<stats.disabled<<" disabled / "<<stats.noSlot<<" missing slots; "<<stats.acceptedWords<<" enqueued words / "<<stats.rejectedBatches<<" rejected batches / "<<stats.completed<<" completion callbacks / "<<stats.resets<<" resets / "<<stats.scheduled<<" schedules / "<<stats.readWraps<<" read wraps / "<<stats.writeWraps<<" write wraps; all windows and original direct returns match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
