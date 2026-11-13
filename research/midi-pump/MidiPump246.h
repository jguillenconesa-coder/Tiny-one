// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "../core-selection-sync/CoreSelectionSync246.h"
#include <algorithm>
namespace pocket::research {
// Supplied detached requests and a busy endpoint. No physical-driver callback.
struct MidiPump246 {
 using Image=std::array<uint8_t,5088>;
 struct Stats {
  size_t acceptedWords=0,rejectedBatches=0,pumps=0,typeRejected=0,disabled=0,noSlot=0,copyBytes=0,submitted=0,submitRejected=0,completed=0,resets=0,scheduled=0,readWraps=0,writeWraps=0;
  std::array<bool,33> chunks{};std::array<bool,2> buffers{};
 };
 static uint32_t get(const uint8_t* p){return CoreSelectionSync246::get(p);}
 static void put(uint8_t* p,uint32_t v){CoreSelectionSync246::put(p,v);}
 static int32_t signedWord(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000ll);}
 static size_t offset(uint32_t address){
  constexpr std::array<std::array<uint32_t,3>,12> windows{{
   {{0x01ab1b38,4120,0}},{{0x01dfe038,12,4120}},{{0x018c1e90,12,4132}},{{0x018b6064,16,4144}},
   {{0x02021000,16,4160}},{{0x02021100,192,4176}},{{0x02021200,48,4368}},{{0x02021240,48,4416}},
   {{0x02021300,32,4464}},{{0x02021400,160,4496}},{{0x02021500,160,4656}},{{0x02021600,272,4816}}
  }};
  for(auto w:windows)if(address>=w[0]&&address-w[0]<w[1])return size_t(w[2]+address-w[0]);
  return 5088;
 }
 static bool writeAddress(Image& m,uint32_t address,uint32_t value){auto o=offset(address);if(o>m.size()-4)return false;put(m.data()+o,value);return true;}
 static uint32_t pump(Image& m,uint32_t source,unsigned bytes,Stats& s){
  ++s.pumps;
  if(get(m.data()+4168)!=6){++s.typeRejected;return 0xffffffffu;}
  if(!get(m.data()+4124)){++s.disabled;return 0xffffffffu;}
  const auto selector=get(m.data()+4360),request=get(m.data()+4352+selector*4);put(m.data()+4360,(selector+1)&1u);
  if(!request){++s.noSlot;return 0xffffffffu;}
  const auto p=offset(request);const auto buffer=get(m.data()+p+24);const auto src=offset(source),dst=offset(buffer);
  if(bytes>128||src>m.size()-bytes||dst>m.size()-bytes)return 0xfffffffeu; // own profile guard
  std::copy_n(m.begin()+src,bytes,m.begin()+dst);s.copyBytes+=bytes;s.buffers[request==0x02021240]=true;
  put(m.data()+p+36,0);put(m.data()+p+28,bytes);put(m.data()+p+32,bytes);
  if(!get(m.data()+p+8)){++s.submitRejected;return 0xffffffffu;}
  put(m.data()+p+40,0);
  const auto previous=get(m.data()+p+4),tail=get(m.data()+4472);
  if(!writeAddress(m,previous,0x02021304))return 0xfffffffeu;
  put(m.data()+4472,previous);put(m.data()+p+4,tail);
  if(!writeAddress(m,tail,request))return 0xfffffffeu;
  ++s.submitted;return 0;
 }
 static void schedule(Image& m,Stats& s){
  if(signedWord(get(m.data()+8))>=1)return;
  const auto read=get(m.data()+4)&1023u,write=get(m.data()+4108)&1023u;
  const auto span=(read<=write?write:1024u)-read;
  if(!span)return;
  const auto count=std::min(span,32u);put(m.data()+8,get(m.data()+8)+1u);++s.scheduled;s.chunks[count]=true;
  pump(m,0x01ab1b44+read*4,count*4,s); // original caller does not roll back on failure
 }
 static void enqueue(Image& m,unsigned words,Stats& s){
  if(!m[4112])return;
  const auto read=get(m.data()+4),write=get(m.data()+4108);
  if(words<read+1024u-write){for(unsigned i=0;i<words;++i)put(m.data()+12+((write+i)&1023u)*4,get(m.data()+4824+i*4));put(m.data()+4108,write+words);s.acceptedWords+=words;s.writeWraps+=write+words<write;}
  else ++s.rejectedBatches;
  schedule(m,s);
 }
 static bool run(Image& m,unsigned action,unsigned count,unsigned complete,uint32_t meta4,uint32_t meta5,uint32_t& returned,Stats& s){
  returned=0x5a5a5a5a;
  if(action>4||(action==1?count>128:count>64)||get(m.data()+4120)!=0x02021100||get(m.data()+4136)!=0x02021000||get(m.data()+4360)>1||!get(m.data()+4476))return false;
  if(get(m.data()+4468)!=0x02021304||get(m.data()+4472)!=0x02021304)return false;
  for(size_t v=0;v<2;++v){const auto p=4368+v*48;const auto addr=0x02021200u+uint32_t(v)*64;const auto slot=get(m.data()+4352+v*4);if((slot&&slot!=addr)||get(m.data()+p)!=addr||get(m.data()+p+4)!=addr||get(m.data()+p+24)!=0x02021408u+v*256)return false;const auto endpoint=get(m.data()+p+8);if(endpoint&&endpoint!=0x02021300)return false;}
  if(action==0||action==3)enqueue(m,count,s);
  if(action==1){returned=pump(m,0x02021608,count,s);if(returned==0xfffffffeu)return false;}
  if(action==2||action==3||action==4){
   put(m.data()+4148,meta4);put(m.data()+4152,meta5);
   if(action==4){for(size_t p:{size_t(0),size_t(4),size_t(8),size_t(4108)})put(m.data()+p,0);m[4112]=1;++s.resets;}
   else {put(m.data(),get(m.data())+1u);const auto before=get(m.data()+4),after=before+(complete>>2);put(m.data()+4,after);s.readWraps+=after<before;put(m.data()+8,get(m.data()+8)-1u);++s.completed;schedule(m,s);}
  }
  return true;
 }
};
}
