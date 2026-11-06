// SPDX-License-Identifier: AGPL-3.0-only
#include "CoreInitialState246.h"
#include "Core246ControlWorkspace.h"
#include "../drum-voice/DrumVoice246.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<uint8_t>;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Core initializer fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t get(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static void put(uint8_t* p,uint32_t word){for(unsigned i=0;i<4;++i)p[i]=uint8_t(word>>(i*8));}
static size_t offset(uint32_t address){size_t start=0;for(auto window:std::array<std::pair<uint32_t,unsigned>,4>{{{0xff800ff8,2024},{0x01aaabd4,3560},{0x01bab538,16},{0xffb00000,256}}}){if(address>=window.first&&uint64_t(address)<uint64_t(window.first)+window.second)return start+address-window.first;start+=window.second;}throw std::runtime_error("Address outside observed windows");}
static void compare(const Bytes& expected,const uint8_t* original,size_t record,const char* stage){for(size_t i=0;i<expected.size();++i)if(expected[i]!=original[i]){std::cerr<<stage<<" case "<<record<<" byte "<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(original[i])<<'\n';throw std::runtime_error("Original Core state differs");}}
int main(int argc,char** argv){try{using namespace pocket::research;using namespace pocket::reference;
 check(argc==2,"Provide uncompressed Core initialization fixtures");const std::filesystem::path root(argv[1]);const auto input=read(root/"core-initial-state246-input.bin"),output=read(root/"core-initial-state246-output.bin");
 constexpr size_t count=256,snapshot=5856,stride=5880,result=11736;check(input.size()==3072+count*stride&&output.size()==count*result,"Wrong Core fixture dimensions");
 DrumVoice246::Controls controls{};for(size_t bank=0;bank<8;++bank)for(size_t pad=0;pad<24;++pad)for(size_t i=0;i<8;++i){const auto* p=input.data()+(bank*24+pad)*16+i*2;const auto value=unsigned(p[0])|(unsigned(p[1])<<8);controls[bank][pad][i]=int16_t(value<32768?int(value):int(value)-65536);}
 std::array<bool,snapshot> touched{};for(auto span:coreInitialWrites246){check(span.count>0&&span.offset+span.count<=snapshot,"Derived write outside image");for(unsigned i=0;i<span.count;++i){check(!touched[span.offset+i],"Overlapping derived writes");touched[span.offset+i]=true;}}
 check(std::count(touched.begin(),touched.end(),true)==4641,"Core default write footprint changed");
 std::array<bool,192> pairs{};std::array<bool,256> transpositions{};unsigned signedVelocities=0;
 for(size_t n=0;n<count;++n){const auto* x=input.data()+3072+n*stride;const auto* y=output.data()+n*result;Bytes image(x,x+snapshot);coreInitialState246(image.data());compare(image,y,n,"init");Core246ControlWorkspace workspace;std::copy_n(image.begin()+offset(0xff8016d8),workspace.size(),workspace.begin());core246InitializeControlWorkspaces(workspace);std::copy(workspace.begin(),workspace.end(),image.begin()+offset(0xff8016d8));const auto* event=x+snapshot;const auto sound=get(event);const int transpose=signedPhase(get(event+4));
  check(sound<8&&transpose>=-128&&transpose<=127&&get(event+8)>=53&&get(event+8)<=76,"Invalid supplied event");pairs[sound*24+get(event+8)-53]=true;transpositions[size_t(transpose+128)]=true;signedVelocities+=(get(event+12)&32768u)!=0;std::copy_n(input.begin(),3072,image.begin()+offset(0x01aaadb4));put(image.data()+offset(0x01aaac10),sound);put(image.data()+offset(0x01aaac48),0);std::fill_n(image.begin()+offset(0x01aaad84),48,0);put(image.data()+offset(0xff801038),1);std::fill_n(image.begin()+offset(0xff801070),12,0);
  DrumVoice246 model;for(size_t v=0;v<12;++v){auto& state=model.audio.pool[v];const auto* p=image.data()+offset(0xffb00000+uint32_t(v)*20);state.tag=get(p);state.handle=get(p+4);state.position=signedPhase(get(p+8));state.mode=signedPhase(get(p+12));state.blocks=get(p+16);}
  check(model.audio.factory(sound),"Factory sound rejected");std::copy_n(image.begin()+offset(0xff801038),model.globals.size(),model.globals.begin());std::fill_n(model.held.begin(),20,0x5a);DrumVoice246::put(model.held.data()+2560,0);
  std::array<uint32_t,4> words{{get(event+8),get(event+12),get(event+16),get(event+20)}};check(model.on(words,transpose,-1,sound,controls)==0,"Initialized event not accepted");
  std::copy(model.globals.begin(),model.globals.end(),image.begin()+offset(0xff801038));for(size_t v=0;v<12;++v){const auto& state=model.audio.pool[v];auto* p=image.data()+offset(0xffb00000+uint32_t(v)*20);unsigned i=0;for(auto value:{state.tag,state.handle,uint32_t(state.position),uint32_t(state.mode),state.blocks}){put(p+i,value);i+=4;}put(image.data()+offset(0x01aaad30+uint32_t(v)*4),model.audio.pointers[v]);}
  image.insert(image.end(),model.held.begin(),model.held.begin()+20);image.insert(image.end(),model.held.begin()+2560,model.held.begin()+2564);compare(image,y+snapshot,n,"factory/note-on");
 }
 for(bool seen:pairs)check(seen,"Incomplete sound/pad coverage");for(bool seen:transpositions)check(seen,"Incomplete integer transposition coverage");check(signedVelocities>0&&signedVelocities<count,"Signed velocity cases absent");
 std::cout<<"PASS: 256 complete original Core initializers, shared workspace initialization and factory/note-on chains; 4641 constant / 1215 preserved bytes across four windows; all 192 sound/pad pairs, signed velocities and 256 integer transpositions; no audio render\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
