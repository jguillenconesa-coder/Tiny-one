// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "../synth-bank-switch/SynthBankSwitch246.h"
#include "DrWave246PitchRefresh.h"
#include <cstring>
namespace pocket::research {
// Original readable synth changer + Dr. Wave refresh, supplied initialized graph.
// L1 completion is authored. Original MIDI helpers use explicit output-disabled state.
struct SynthHeldBankSwitch246 {
 using Image=std::array<uint8_t,4156>;
 using Trace=SynthBankSwitch246::Trace;
 struct Stats {size_t translated=0,midiOff=0,midiOn=0,refreshEntries=0,voiceResets=0,envelopeResets=0,initializations=0,deferred=0;};
 static uint32_t get(const uint8_t* p){return SynthBankSwitch246::get(p);}
 static void put(uint8_t* p,uint32_t v){SynthBankSwitch246::put(p,v);}
 static int16_t half(const uint8_t* p){unsigned v=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(v<32768?int(v):int(v)-65536);}
 static void putHalf(uint8_t* p,int16_t v){p[0]=uint8_t(v);p[1]=uint8_t(uint16_t(v)>>8);}
 static bool change(Image& m,unsigned target,unsigned delay,Trace& trace,unsigned& calls,Stats& stats){
  using namespace reference;trace.fill(0x5a);calls=0;
  const auto old=get(m.data()+76),count=get(m.data()+344);
  if(target>=8||old>=8||count>128||get(m.data()+3308)!=count||get(m.data()+340)!=0||get(m.data()+136)!=2||!get(m.data()+336)||get(m.data()+3448)||get(m.data()+4020)!=1200||delay<1||delay>4)return false;
  for(size_t v=0;v<6;++v)if(get(m.data()+476+v*4)!=0xffb00000+v*36||get(m.data()+500+v*4)||get(m.data()+524+v*36)!=0x0190fb2c)return false;
  for(size_t i=0;i<count;++i)if(get(m.data()+748+i*20)>=128||signedPhase(get(m.data()+764+i*20))<-2||signedPhase(get(m.data()+764+i*20))>=6)return false;
  if(old==target)return true;
  m[144]=1;
  if(!get(m.data()+148)){put(m.data()+148,1);m[152]=m[133];++stats.initializations;}
  for(unsigned v=0;v<6;++v){put(m.data()+192+v*24,8);put(m.data()+196+v*24,0);++stats.envelopeResets;}
  const auto delta=get(m.data()+156+target*4)-get(m.data()+156+old*4);
  if(delta){
   if(m[152]){put(m.data()+188,get(m.data()+188)+delta);++stats.deferred;}
   else for(size_t i=0;i<count;++i){auto* row=m.data()+748+i*20;if(!row[8])continue;const auto before=get(row),after=before+delta*12u;if(after>=128)return false;
    ++stats.midiOff;m[3316+before]=0;
    put(row,after);++stats.midiOn;
    const auto velocity=get(row+4);
    if(!velocity)m[3316+after]=0;else if(!m[3316+after])m[3316+after]=1;
    ++stats.translated;
   }
  }
  put(m.data()+76,target);CoreSelectionSync246 sync;std::copy_n(m.begin(),132,sync.windows.begin());uint32_t r2=0;
  if(!sync.sync(false,r2,[&](uint32_t endpoint,uint32_t a,uint32_t b,uint32_t c){if(calls>=4)return;const unsigned offset=calls++*24;const std::array<uint32_t,6> fields{{endpoint,a,b,c,get(sync.windows.data()),get(sync.windows.data()+76)}};for(unsigned i=0;i<6;++i)put(trace.data()+offset+i*4,fields[i]);if(calls==delay)put(sync.windows.data(),get(sync.windows.data()+76));},4))return false;
  std::copy(sync.windows.begin(),sync.windows.end(),m.begin());
  if(!m[152]){
   ++stats.refreshEntries;
   DrWave246Pool pool;std::memcpy(pool.data(),m.data()+524,sizeof(pool));
   Voice246Registers regs;std::array<Adsr246State,6> env{};
   for(size_t v=0;v<6;++v){regs.active[v]=m[3452+v];regs.velocity[v]=half(m.data()+3460+v*2);regs.pitch[v]=signedPhase(get(m.data()+3476+v*4));auto* p=m.data()+192+v*24;auto& e=env[v];e.stage=get(p);e.blocks=get(p+4);e.level=signedPhase(get(p+8));e.field12=get(p+12);e.field16=get(p+16);e.gain=get(p+20);}
   std::array<Voice246PitchEntry,128> entries{};
   for(size_t i=0;i<count;++i){auto* p=m.data()+748+i*20;entries[i]={{get(p),get(p+4),p[8],signedPhase(get(p+12))},signedPhase(get(p+16))};}
   auto random=get(m.data()+3500);Voice246TuningTable tuning{};tuning[1200]=signedPhase(get(m.data()+4024));const auto mode=half(m.data()+348+target*16+8);
   if(!drWave246RefreshPitch(pool,regs,env,random,entries.data(),count,mode,tuning,1200,int(get(m.data())),int(target)))return false;
   if(count&&floorDivide(mode,4096)!=4){if(floorDivide(mode,4096)==0){for(size_t i=0;i<count;++i)stats.voiceResets+=entries[i].voice>=0;}else stats.voiceResets+=floorDivide(mode,4096)==3?6:1;}
   std::memcpy(m.data()+524,pool.data(),sizeof(pool));put(m.data()+3500,random);
   for(size_t v=0;v<6;++v){m[3452+v]=regs.active[v];putHalf(m.data()+3460+v*2,regs.velocity[v]);put(m.data()+3476+v*4,uint32_t(regs.pitch[v]));auto* p=m.data()+192+v*24;const auto& e=env[v];put(p,e.stage);put(p+4,e.blocks);put(p+8,uint32_t(e.level));put(p+12,e.field12);put(p+16,e.field16);put(p+20,e.gain);}
  }
  m[144]=0;return true;
 }
};
}
