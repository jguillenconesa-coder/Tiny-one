// SPDX-License-Identifier: AGPL-3.0-only
#include "Element246Pitch.h"
#include "Bend246Voice.h"
#include <string>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Element fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const unsigned v=p[0]|unsigned(p[1])<<8;return int16_t(v<32768?int(v):int(v)-65536);}
int main(int argc,char** argv){try{using namespace pocket::reference;check(argc==2,"Provide unpacked fixture directory");
 size_t bridgeCalls=0;
 const auto input=read(std::filesystem::path(argv[1])/"element-pitch246-input.bin"),output=read(std::filesystem::path(argv[1])/"element-pitch246-output.bin");check(input.size()==8192*444&&output.size()==8192*1204,"Incomplete Element fixtures");
 for(size_t n=0;n<8192;++n){const auto* x=input.data()+n*444;const auto* y=output.data()+n*1204;std::array<int16_t,8> controls{},targets{};std::array<int16_t,64> external{};std::array<int16_t,128> gain{};
  for(size_t i=0;i<8;++i){controls[i]=half(x+16+i*2);targets[i]=half(x+32+i*2);}for(size_t i=0;i<64;++i)external[i]=half(x+60+i*2);for(size_t i=0;i<128;++i)gain[i]=half(x+188+i*2);
  Element246Inputs in{half(x+48),element246PositivePeak(external),element246PositivePeak(gain),half(x+50),half(x+52),half(x+58)};
  std::array<int16_t,128> audio{};for(size_t i=0;i<128;++i){const uint32_t v=word(x+12)+0x09020705u*uint32_t(i/2);audio[i]=half(reinterpret_cast<const unsigned char*>(&v)+(i%2)*2);}const auto before=audio;
  std::array<uint32_t,128> pitch{};for(size_t i=0;i<128;++i)pitch[i]=word(x+12)*uint32_t(i+1);
  const auto originalGain=gain;const auto originalPitch=pitch;
  const uint32_t accepted=element246PitchBlock(in,controls,int(word(x)),int(word(x+8)),external,gain,pitch);
  if(element246PitchRoute(controls,int(word(x)),int(word(x+8)))&&(controls[0]/2048!=2||controls[3]/4096==0||std::all_of(originalGain.begin(),originalGain.end(),[&](int16_t v){return v==originalGain[0];}))){
   const int source=controls[0]/2048;const auto carrier=source==2?originalGain[0]:source==3?in.level:saturate16(-int(in.sensor));const auto coefficient=multiplyQ15(carrier,controls[1]);
   for(bool blocked:{false,true}){pocket::Bend246Voice bridge;bridge.configure(true,1,controls[1],coefficient,blocked,4,source==1?6:source==2?5:4,&external,0);auto p=originalPitch;if(blocked)bridge.pitchBlock(p);for(size_t i=0;i<128;++i){if(!blocked)p[i]=bridge.pitch(p[i]);check(p[i]==pitch[i]&&bridge.amplitude(originalGain[i])==gain[i],"Element pitch stream/block bridge differs from original");}}++bridgeCalls;
  }
  std::array<unsigned char,1204> actual{};std::memcpy(actual.data(),x+12,4);std::memcpy(actual.data()+4,controls.data(),16);std::memcpy(actual.data()+20,targets.data(),16);std::memcpy(actual.data()+36,&accepted,4);
  const uint32_t sensorWord=uint32_t(uint16_t(in.sensor))*65536u+0x2468u;std::memcpy(actual.data()+40,&sensorWord,4);std::memcpy(actual.data()+44,x+50,4);std::memcpy(actual.data()+48,x+56,4);std::memcpy(actual.data()+52,external.data(),128);std::memcpy(actual.data()+180,gain.data(),256);
  for(size_t i=0;i<192;++i){const uint32_t v=i<128?word(x+12)*uint32_t(i+1):0x09020705u*uint32_t(i-127);std::memcpy(actual.data()+436+i*4,&v,4);}
  std::memcpy(actual.data()+436,pitch.data(),512);std::memcpy(actual.data()+948,audio.data(),256);
  if(std::memcmp(actual.data(),y,actual.size())){for(size_t i=0;i<actual.size();++i)if(actual[i]!=y[i]){std::cerr<<"Element call "<<n<<" family "<<word(x)<<" type "<<word(x+4)<<" kind "<<word(x+8)<<" source "<<controls[0]/2048<<" destination "<<controls[2]/2048<<" byte "<<i<<" expected "<<int(y[i])<<" got "<<int(actual[i])<<'\n';break;}throw std::runtime_error("Original Element source/parameter/buffer mismatch");}
 }
 std::cout<<"Bridge original calls: "<<bridgeCalls<<" stream and block captures; source-2 pitch restricted to supplied constant gain for the host bridge\n";
 std::cout<<"PASS: 8192 original Element pitch callbacks, four sources, exact return including processed-zero Level, all controls/targets/sources/pitch/gain/audio\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
