// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "../synth-held-bank-switch/SynthHeldBankSwitch246.h"
#include "Voice246ReleaseSelectors.h"
namespace pocket::research {
struct SynthRangeBankSwitch246 {
 using Base=SynthHeldBankSwitch246;
 using Image=std::array<uint8_t,8324>;
 using Trace=Base::Trace;
 struct Stats:Base::Stats {size_t removed=0,offPackets=0,onPackets=0,queued=0,dropped=0,queueWraps=0;};
 static int16_t half(const uint8_t* p){return Base::half(p);}
 static uint32_t get(const uint8_t* p){return Base::get(p);}
 static void put(uint8_t* p,uint32_t v){Base::put(p,v);}
 // Original event producer -> bounded original ring; the DMA pump is busy.
 static bool packet(Image& m,bool on,uint32_t note,uint32_t velocity,Stats& stats){
  if(!get(m.data()+3448))return true;
  constexpr size_t context=4160,pending=4201,ring=4204;
  auto* event=m.data()+context+(on?28:24);
  event[0]=on?9:8;event[1]=uint8_t(m[context]|(on?0x90:0x80));event[2]=uint8_t(note);event[3]=on?uint8_t(velocity):64;m[pending]=1;
  if(on)++stats.onPackets;else ++stats.offPackets;
  if(!m[ring+4112])return true;
  const auto read=get(m.data()+ring+4),write=get(m.data()+ring+4108);
  if(1u<read+1024u-write){put(m.data()+ring+12+(write&1023u)*4,get(event));put(m.data()+ring+4108,write+1u);++stats.queued;stats.queueWraps+=write==0xffffffffu;}
  else ++stats.dropped;
  return true;
 }
 static bool off(Image& m,uint32_t note,Stats& stats){if(note>=128)return false;if(!m[3316+note])return true;m[3316+note]=0;return packet(m,false,note,64,stats);}
 static bool on(Image& m,uint32_t note,uint32_t velocity,Stats& stats){if(note>=128)return false;if(!velocity)return off(m,note,stats);if(m[3316+note])return true;m[3316+note]=1;return packet(m,true,note,velocity,stats);}
 static bool remove(Image& m,uint32_t note,Stats& stats){
  using namespace reference;const auto bank=get(m.data()+76),before=get(m.data()+344);const auto mode=Base::half(m.data()+348+bank*16+8),sustain=Base::half(m.data()+348+bank*16+4);
  if(floorDivide(mode,4096)<0||floorDivide(mode,4096)>3||sustain<0)return false;
  Voice246ReleaseList list;std::copy_n(m.begin()+748,2560,list.bytes.begin());list.count=before;
  std::array<Adsr246State,6> env{};std::array<int32_t,6> pitch{};
  for(size_t v=0;v<6;++v){auto* p=m.data()+192+v*24;auto& e=env[v];e.stage=get(p);e.blocks=get(p+4);e.level=signedPhase(get(p+8));e.field12=get(p+12);e.field16=get(p+16);e.gain=get(p+20);pitch[v]=signedPhase(get(m.data()+3476+v*4));}
  auto random=get(m.data()+3500);const auto result=floorDivide(mode,4096)==0?voice246PolyRelease(list,note,env,sustain):voice246ReleaseSelectors(list,note,mode,env,sustain,signedPhase(get(m.data()+4024)),pitch,random);
  if(result!=Voice246ReleaseResult::removed||list.count+1!=before)return false;
  std::copy(list.bytes.begin(),list.bytes.end(),m.begin()+748);put(m.data()+344,list.count);put(m.data()+3308,list.count);put(m.data()+3500,random);
  for(size_t v=0;v<6;++v){auto* p=m.data()+192+v*24;const auto& e=env[v];put(p,e.stage);put(p+4,e.blocks);put(p+8,uint32_t(e.level));put(p+12,e.field12);put(p+16,e.field16);put(p+20,e.gain);put(m.data()+3476+v*4,uint32_t(pitch[v]));}
  ++stats.removed;++stats.midiOff;return off(m,note,stats);
 }
 static bool change(Image& m,unsigned target,unsigned delay,Trace& trace,unsigned& calls,Stats& stats){
  if(m[4156]||get(m.data()+100)||m[4160]>15||get(m.data()+4204+8)<1)return false;
  return Base::changeWithCallbacks(m,target,delay,trace,calls,stats,
   [&](auto& image,uint32_t note){return remove(image,note,stats);},
   [&](auto& image,uint32_t note){return off(image,note,stats);},
   [&](auto& image,uint32_t note,uint32_t velocity){return on(image,note,velocity,stats);});
 }
};
}
