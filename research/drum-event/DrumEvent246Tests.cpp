// SPDX-License-Identifier: AGPL-3.0-only
#include "../drum-voice/DrumVoice246.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<unsigned char>;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing drum event fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto v=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(v<32768?int(v):int(v)-65536);}
static void put(Bytes& b,uint32_t w,unsigned bytes=4){for(unsigned i=0;i<bytes;++i)b.push_back(static_cast<unsigned char>(w>>(8*i)));}
int main(int argc,char** argv){try{
 using namespace pocket::reference;using pocket::research::DrumVoice246;
 check(argc==2,"Provide uncompressed event fixtures");const std::filesystem::path root(argv[1]);const auto in=read(root/"drum-event246-input.bin"),out=read(root/"drum-event246-output.bin");
 constexpr size_t cases=65536,result=1076;check(in.size()==3072+cases*24&&out.size()==cases*result,"Wrong event fixture sizes");
 DrumVoice246::Controls controls{};for(size_t b=0;b<8;++b)for(size_t p=0;p<24;++p)for(size_t i=0;i<8;++i)controls[b][p][i]=half(in.data()+(b*24+p)*16+i*2);
 DrumVoice246 model;model.audio.factory(0);std::array<bool,65536> velocities{};std::array<bool,256> transpositions{};size_t negative=0,saturated=0;
 for(size_t n=0;n<cases;++n){const auto* x=in.data()+3072+n*24;const auto* y=out.data()+n*result;const auto sound=word(x);const auto transpose=signedPhase(word(x+4));const std::array<uint32_t,4> event{{word(x+8),word(x+12),word(x+16),word(x+20)}};
  check(sound<8&&transpose>=-128&&transpose<=127,"Invalid supplied event");model.globals.fill(0);DrumVoice246::put(model.at(0x1038),1);std::fill_n(model.held.begin(),20,0x5a);DrumVoice246::put(model.held.data()+2560,0);
  check(model.on(event,transpose,-1,sound,controls)==0,"Original fresh event not accepted by model");
  Bytes expected;expected.reserve(result);for(const auto& s:model.audio.pool)for(auto w:{s.tag,s.handle,uint32_t(s.position),uint32_t(s.mode),s.blocks})put(expected,w);expected.insert(expected.end(),model.globals.begin(),model.globals.end());expected.insert(expected.end(),model.held.begin(),model.held.begin()+20);put(expected,DrumVoice246::get(model.held.data()+2560));
  check(expected.size()==result,"Event result format error");if(!std::equal(expected.begin(),expected.end(),y)){std::cerr<<"case "<<n<<" first differing byte ";for(size_t i=0;i<result;++i)if(expected[i]!=y[i]){std::cerr<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(y[i])<<'\n';break;}throw std::runtime_error("Complete original note-on differs");}
  velocities[event[1]&65535u]=true;transpositions[size_t(transpose+128)]=true;negative+=half(y+932)<0;saturated+=word(y+956)==0x7fffffffu;
 }
 for(auto b:velocities)check(b,"Velocity domain incomplete");for(auto b:transpositions)check(b,"Pitch domain incomplete");check(negative>0&&saturated>0,"Signed/saturated event cases absent");
 uint32_t sentinel=0x13579bdf;check(!drum246EventPitchWord(-129,sentinel)&&sentinel==0x13579bdf&&!drum246EventPitchWord(128,sentinel),"Own discrete-domain guard failed");
 check(drum246EventPitchWords[128]==0x04000000&&drum246EventPitchWords[188]==0x7fffffff,"Neutral/saturation anchors failed");
 std::cout<<"PASS: 65536 complete original note-on records, all raw velocity halfwords / 256 integer transpositions; full twelve-object pool, 812 global bytes, retained row/count; "<<negative<<" negative velocity words / "<<saturated<<" saturated pitch words\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
