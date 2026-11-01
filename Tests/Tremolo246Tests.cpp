// SPDX-License-Identifier: AGPL-3.0-only
#include "Tremolo246Voice.h"
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& path){std::ifstream f(path,std::ios::binary);check(bool(f),"Missing Tremolo fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){return int16_t(uint16_t(p[0])|(uint16_t(p[1])<<8));}
int main(int argc,char** argv){try{
 check(argc==2,"Provide reference fixture directory");const std::filesystem::path root(argv[1]);
 auto in=read(root/"tremolo246-input.bin"),out=read(root/"tremolo246-output.bin");
 check(in.size()==2048*856&&out.size()==96+2048*832,"Wrong Tremolo fixture sizes");
 std::array<int16_t,8192> sine{};for(size_t i=0;i<sine.size();++i)sine[i]=int16_t(std::lround(std::sin(i*3.14159265358979323846/4096)*32767));
 std::array<int32_t,24> decay{};for(size_t i=0;i<decay.size();++i)decay[i]=pocket::reference::signedPhase(word(out.data()+i*4));
 check(decay==pocket::reference::tremolo246Decay,"Tremolo decay coefficients differ from original conversion");
 pocket::reference::Tremolo246State state;std::array<uint32_t,8> display{};int mismatches=0;std::array<int,5> regions{};
 for(size_t caseIndex=0;caseIndex<2048;++caseIndex){const auto* x=in.data()+caseIndex*856;const auto* y=out.data()+96+caseIndex*832;
  if(word(x+84)){std::memcpy(&state,x,28);for(size_t i=0;i<8;++i)display[i]=word(x+28+i*4);}
  std::array<int16_t,8> c{};for(size_t i=0;i<8;++i)c[i]=half(x+60+i*2);
  if(word(x+84)==2)pocket::reference::tremolo246NoteOn(state,display,c);
  std::array<uint32_t,128> pitch{};std::array<int16_t,128> audio{};
  for(size_t i=0;i<128;++i){pitch[i]=word(x+88+i*4);audio[i]=half(x+600+i*2);}
  for(bool buffered:{false,true})if(word(x+80)==0||word(x+80)==3){
   pocket::Tremolo246Voice adapter;adapter.state=state;adapter.display=display;adapter.active=true;
   std::array<int16_t,4> raw{{c[0],c[1],c[2],c[3]}};adapter.configure(true,raw,c[7]/6554,word(x+76),buffered);
   auto ap=pitch;auto aa=audio;if(buffered)adapter.pitchBlock(ap,sine);
   for(size_t i=0;i<128;++i){if(!buffered)ap[i]=adapter.pitch(ap[i],sine);aa[i]=adapter.amplitude(aa[i],sine);}
   check(std::memcmp(&adapter.state,y,28)==0&&std::memcmp(adapter.display.data(),y+28,32)==0&&std::memcmp(ap.data(),y+60,512)==0&&std::memcmp(aa.data(),y+572,256)==0,"Streaming or buffered adapter differs from original callback");
  }
  const auto result=pocket::reference::tremolo246Block(state,display,c,word(x+76),int(word(x+80)),pitch,audio,sine,decay);
  std::array<unsigned char,832> actual{};std::memcpy(actual.data(),&state,28);std::memcpy(actual.data()+28,display.data(),32);std::memcpy(actual.data()+60,pitch.data(),512);std::memcpy(actual.data()+572,audio.data(),256);const uint32_t r=result?1:0;std::memcpy(actual.data()+828,&r,4);
  for(size_t reg=0;reg<5;++reg){const size_t starts[]={0,28,60,572,828},ends[]={28,60,572,828,832};for(size_t i=starts[reg];i<ends[reg];++i)if(actual[i]!=y[i]){if(regions[reg]++<2)std::cerr<<"region "<<reg<<" case "<<caseIndex<<" offset "<<i<<" actual "<<word(actual.data()+i-i%4)<<" expected "<<word(y+i-i%4)<<"\n";break;}}
  for(size_t i=0;i<actual.size();++i)if(actual[i]!=y[i]){if(mismatches++<12){std::cerr<<"case "<<caseIndex<<" shape "<<c[7]<<" kind "<<word(x+80)<<" byte "<<i<<" actual "<<int(actual[i])<<" expected "<<int(y[i])<<" controls "<<c[0]<<','<<c[1]<<','<<c[2]<<','<<c[3]<<'\n';}break;}
 }

 check(mismatches==0,"Tremolo original sample/state mismatch");
 std::cout<<"PASS: 2048 original Tremolo calls; 262144 pitch words and audio samples; five shapes, full state/display, reset and retained streams\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
