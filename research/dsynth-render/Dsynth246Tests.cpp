// SPDX-License-Identifier: AGPL-3.0-only
// Lifecycle reconstruction plus characterization of original render observations.
// This test does not compare a reconstructed D-synth audio renderer.
#include "Dsynth246Lifecycle.h"
#include "Envelope246Tables.h"
#include "Filter246Tables.h"
#include "ValueVoice246Ranges.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <vector>
static void check(bool x,const char* why){if(!x)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing D-synth fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto x=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
 using namespace pocket::reference;check(argc==2,"Provide D-synth fixtures");const std::filesystem::path root(argv[1]);
 const auto in=read(root/"dsynth246-input.bin"),out=read(root/"dsynth246-output.bin");check(in.size()==4096*2048&&out.size()==1604+4096*4320,"Wrong D-synth fixture size");
 for(size_t i=0;i<256;++i)check(word(out.data()+i*4)==uint32_t(i<128?adsr246Attack[i]:adsr246Decay[i-128]),"D-synth envelope table mismatch");
 for(size_t i=0;i<129;++i)check(word(out.data()+1024+i*4)==uint32_t(filter246Frequency[i]),"D-synth filter table mismatch");
 for(size_t i=0;i<8;++i)for(size_t j=0;j<4;++j)check(half(out.data()+1540+i*8+j*2)==valueVoice246Ranges[8][i][j],"D-synth descriptor mismatch");
 std::array<Dsynth246State,6> pool{};Dsynth246Context context{};uint64_t energy=0;unsigned captures=0;
 for(size_t n=0;n<4096;++n){const auto* x=in.data()+n*2048;const auto* y=out.data()+1604+n*4320;const auto flags=word(x+1272),slot=word(x+1276);check(slot<6,"Invalid D-synth fixture slot");
  if(flags&1){std::memcpy(pool.data(),x,432);std::memcpy(&context,x+432,824);}
  std::array<int16_t,8> controls{};for(size_t i=0;i<8;++i)controls[i]=half(x+1256+i*2);
  auto& state=pool[slot];if(flags&16)context.initialize();if(flags&2){state.construct();check(state.bind(int(slot)),"D-synth bind failed");}if(flags&4)check(context.reset(state,controls),"D-synth reset failed");if(flags&8)state.release();if(flags&32)check(context.decayMeter(int(slot)),"Meter slot failed");if(flags&64)check(context.resetCapture(int(slot)),"Capture slot failed");
  check(std::memcmp(pool.data(),y,432)==0,"D-synth lifecycle pool mismatch");if(std::memcmp(&context,y+432,824)){const auto* actual=reinterpret_cast<const unsigned char*>(&context);for(size_t i=0;i<824;++i)if(actual[i]!=y[432+i]){std::cerr<<"case "<<n<<" context byte "<<i<<" got "<<int(actual[i])<<" expected "<<int(y[432+i])<<" flags "<<flags<<"\n";break;}throw std::runtime_error("D-synth lifecycle context mismatch");}
  // Render itself remains the original observation: establish its memory contract.
  const auto* post=y+1256;for(size_t i=0;i<6;++i)if(i!=slot)check(std::memcmp(y+i*72,post+i*72,72)==0,"Original render touched another voice");
  check(std::memcmp(y+432,post+432,8)==0,"Original render touched shared constants");
  for(size_t i=0;i<6;++i)if(i!=slot){check(std::memcmp(y+440+i*4,post+440+i*4,4)==0,"Original render touched another meter");check(std::memcmp(y+464+i*128,post+464+i*128,128)==0,"Original render touched another history");}
  std::array<int16_t,128> audio{};for(size_t i=0;i<128;++i){audio[i]=half(y+2512+i*2);energy+=uint64_t(int64_t(audio[i])*audio[i]);}
  const auto beforeCount=context.counts[slot];check(context.capture(int(slot),audio),"D-synth capture bound failed");captures+=beforeCount<60;
  check(std::memcmp(context.history.data(),post+464,768)==0&&std::memcmp(context.counts.data(),post+1232,24)==0,"D-synth history capture mismatch");
  check(std::memcmp(x+1280,y+2768,512)==0&&std::memcmp(x+1792,y+4048,256)==0,"Original render changed caller pitch/gain");
  const uint32_t guards[]={0x13579bdf,0x2468ace0,0x789abcde,0x89abcdef};for(size_t i=0;i<4;++i)check(word(y+4304+i*4)==guards[i],"Original render guard/RNG mismatch");
  // Explicit handoff: the next lifecycle uses the original renderer's resulting
  // pool/context. It is not computed by our lifecycle-only reconstruction.
  std::memcpy(pool.data(),post,432);std::memcpy(&context,post+432,824);
 }
 check(energy>0&&captures>0,"Empty D-synth characterization");
 {Dsynth246State s;s.slot=3;check(!s.bind(-1)&&!s.bind(6)&&s.slot==3,"Invalid bind modified state");Dsynth246Context c;c.counts.fill(99);c.initialize();check(c.counts[0]==99&&c.constants[0]==0x00a3d700,"Init incorrectly reset retained counters");}
 std::cout<<"PASS: D-synth lifecycle and history reconstruction; original-only render characterization, 4096 blocks/524288 samples (no C++ audio equivalence claimed)\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
