// SPDX-License-Identifier: AGPL-3.0-only
#include "../drum-voice/DrumVoice246.h"
#include "../core-modulator-reset/CoreModulatorReset246.h"
#include "Drum246ModulationReference.h"
#include "Drum246Tables.h"
#include "Tremolo246.h"
#include "Tremolo246Tables.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<uint8_t>;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static Bytes read(const std::filesystem::path& path){std::ifstream f(path,std::ios::binary);check(bool(f),"Missing modulated render fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const uint8_t* p){const auto v=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(v<32768?int(v):int(v)-65536);}
static void append(Bytes& b,uint32_t value,unsigned size=4){for(unsigned i=0;i<size;++i)b.push_back(uint8_t(value>>(8*i)));}
template<class Range>static void words(Bytes& b,const Range& range){for(auto value:range)append(b,uint32_t(value),sizeof(value));}
template<class State>static void object(Bytes& b,const State& state){const auto* p=reinterpret_cast<const uint8_t*>(&state);b.insert(b.end(),p,p+sizeof(state));}
int main(int argc,char** argv){try{
 using namespace pocket::reference;using namespace pocket::research;
 check(argc==2,"Provide fixture directory");const std::filesystem::path root(argv[1]);const auto in=read(root/"drum-modulated-render246-input.bin"),out=read(root/"drum-modulated-render246-output.bin");
 constexpr size_t cases=4608,stride=988,result=8728,frames=529200,pcmBytes=8*(frames+32)*2,prefix=3072+pcmBytes,outputPrefix=18596;
 check(in.size()==prefix+cases*stride&&out.size()==outputPrefix+cases*result+pcmBytes,"Wrong modulated render fixture dimensions");
 for(size_t i=0;i<1025;++i)check(half(out.data()+i*2)==drum246FirCoefficients[i],"FIR conversion differs");
 const auto sineFile=read(root/"verified-sine.bin");check(sineFile.size()>=16384,"Incomplete verified sine");std::array<int16_t,8192> sine{};
 for(size_t i=0;i<8192;++i){sine[i]=half(sineFile.data()+i*2);check(half(out.data()+2052+i*2)==sine[i],"Original sine differs");}
 for(size_t i=0;i<24;++i)check(signedPhase(word(out.data()+18436+i*4))==tremolo246Decay[i],"Original decay differs");
 for(size_t i=0;i<32;++i)check(half(out.data()+18532+i*2)==drum246ModulationRanges[0][i/4][i%4],"Original drum range differs");
 check(std::equal(in.begin()+3072,in.begin()+prefix,out.begin()+outputPrefix+cases*result),"Original composition modified source PCM");
 std::vector<int16_t> pcm(8*frames);for(size_t b=0;b<8;++b)for(size_t i=0;i<frames;++i)pcm[b*frames+i]=half(in.data()+3072+b*(frames+32)*2+32+i*2);
 DrumVoice246::Controls banks{};for(size_t b=0;b<8;++b)for(size_t p=0;p<24;++p)for(size_t i=0;i<8;++i)banks[b][p][i]=half(in.data()+(b*24+p)*16+i*2);
 DrumVoice246 voice;CoreModulatorReset246 core;Value246State value;Random246State random;Tremolo246State trem;
 std::array<uint32_t,8> vdisplay{},tdisplay{};uint32_t rng=0,meter=0;std::array<std::array<int16_t,8>,8> mods{};std::array<int32_t,128> derived{};derived.fill(0x6c6c6c6c);
 std::array<bool,288> grid{};std::array<bool,24> pads{};std::array<size_t,3> accepted{},rejected{},audible{};size_t resets=0,releases=0,renders=0,skips=0;
 for(size_t n=0;n<cases;++n){const auto* x=in.data()+prefix+n*stride;const auto flags=word(x),v=word(x+4),sound=word(x+8),pad=word(x+12),family=word(x+16),tempo=word(x+24),kind=word(x+28);
  check(v<12&&sound<8&&pad<24&&family<3,"Invalid supplied composition selection");std::array<int16_t,8> base{},c{};for(size_t i=0;i<8;++i){base[i]=half(x+32+i*2);c[i]=half(x+48+i*2);}banks[sound][pad]=base;mods[sound]=c;auto targets=base;
  if(flags&1){++resets;grid[(sound*3+family)*12+v]=true;pads[pad]=true;voice.audio.pool={};voice.reset(sound);std::fill_n(voice.at(0x1070),12,1);*voice.at(0x1070+v)=0;
   core={};core.put(4,sound);core.globals[44+sound]=1;core.pointers[9+v]=family==0?0x02010100u:family==1?0x02010120u:0x02010150u;mods={};mods[sound]=c;
   std::memcpy(&value,x+64,12);std::memcpy(&random,x+76,44);std::memcpy(&trem,x+120,28);value.tag=0x02010180;random.tag=0x020101a0;trem.tag=0x020101c0;
   for(size_t i=0;i<8;++i){vdisplay[i]=word(x+148+i*4);tdisplay[i]=word(x+180+i*4);}meter=word(x+212);rng=word(x+216);
   const auto selected=voice.on({{pad+53,100,171,0x24681357}},signedPhase(word(x+20)),-1,sound,banks,[&](unsigned chosen,unsigned selectedSound){check(chosen==v,"Voice allocation differs");check(core.reset(true,chosen,[&](uint32_t receiver,uint32_t parameters){check(receiver==core.pointers[9+v]&&parameters==0xff801644u+sound*16,"Reset routing differs");if(family==0)value.noteOn(c);else if(family==1)random.noteOn(c[3]);else tremolo246NoteOn(trem,tdisplay,c);}),"Reset gate rejected");return selectedSound;});check(selected==int(v),"Note-on rejected valid composition");
  }
  if(flags&2){++releases;check(voice.off(pad+53,sound,banks),"Note-off failed");}
  std::array<uint32_t,128> pitch{};std::array<int16_t,128> gain{},audio{};for(size_t i=0;i<128;++i){pitch[i]=word(x+220+i*4);gain[i]=half(x+732+i*2);audio[i]=int16_t(i%2?0x2468:0x1357);}
  const bool ok=family==0?valueDrum246Step(value,vdisplay,c,tempo,int(kind),8,targets,sine):family==1?randomDrum246Step(random,rng,meter,c,int(kind),8,targets):tremolo246Block(trem,tdisplay,c,tempo,int(kind),pitch,gain,sine,tremolo246Decay);
  if(ok)++accepted[family];else ++rejected[family];
  if(flags&4)++skips;else{++renders;auto& state=voice.audio.pool[v];check(state.handle==0x02010000+sound*8,"Wrong bound PCM");check(drum246Block(state,targets,pitch,gain,{pcm.data()+sound*frames,frames},drum246FirCoefficients,audio,derived),"Render rejected supplied buffers");bool nonzero=false;for(auto sample:audio)nonzero|=sample!=0;audible[family]+=nonzero;}
  Bytes expected;expected.reserve(result);for(const auto& state:voice.audio.pool)object(expected,state);expected.insert(expected.end(),voice.globals.begin(),voice.globals.end());object(expected,value);object(expected,random);object(expected,trem);words(expected,vdisplay);words(expected,tdisplay);append(expected,meter);append(expected,rng);words(expected,targets);append(expected,ok?1u:0u);words(expected,audio);words(expected,pitch);words(expected,gain);words(expected,derived);expected.insert(expected.end(),voice.held.begin(),voice.held.end());for(const auto& m:mods)words(expected,m);words(expected,voice.audio.pointers);for(const auto& bank:banks)for(const auto& row:bank)words(expected,row);expected.insert(expected.end(),core.globals.begin(),core.globals.end());words(expected,core.pointers);
  check(expected.size()==result,"Internal record format mismatch");const auto* y=out.data()+outputPrefix+n*result;for(size_t i=0;i<result;++i)if(expected[i]!=y[i]){std::cerr<<"block "<<n<<" family "<<family<<" byte "<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(y[i])<<'\n';throw std::runtime_error("Original modulated drum composition differs");}
 }
 for(bool seen:grid)check(seen,"Missing voice/bank/modulator combination");for(bool seen:pads)check(seen,"Missing pad");for(size_t f=0;f<3;++f)check(accepted[f]>0&&rejected[f]>0&&audible[f]>0,"Missing accepted/rejected/audible family");check(resets==288&&releases==288&&renders==4464&&skips==144,"Lifecycle coverage differs");check(accepted==std::array<size_t,3>{{1316,1317,1316}}&&rejected==std::array<size_t,3>{{220,219,220}},"Callback gate coverage differs");
 std::cout<<"PASS: 288 retained sessions, 4608 original modulation blocks, 288 note-ons/releases, 4464 renders / 571392 output samples, 144 poisoned skips; accepted/rejected and audible by family:";for(size_t f=0;f<3;++f)std::cout<<' '<<accepted[f]<<'/'<<rejected[f]<<'/'<<audible[f];std::cout<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
