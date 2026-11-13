// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "../core-selection-sync/CoreSelectionSync246.h"
#include <algorithm>
namespace pocket::research {
// Empty-registry profile; L1 completions are authored, not recovered services.
struct SynthBankSwitch246 {
 using Image=std::array<uint8_t,476>;
 using Trace=std::array<uint8_t,96>;
 static uint32_t get(const uint8_t* p){return CoreSelectionSync246::get(p);}
 static void put(uint8_t* p,uint32_t v){CoreSelectionSync246::put(p,v);}
 static bool change(Image& m,unsigned target,unsigned delay,Trace& trace,unsigned& calls){
  trace.fill(0x5a);calls=0;const auto old=get(m.data()+76);
  if(target>=8||old>=8||get(m.data()+340)!=0||get(m.data()+344)!=0||get(m.data()+136)==4||get(m.data()+336)==0||delay<1||delay>4)return false;
  if(old==target)return true;
  m[144]=1;
  if(get(m.data()+148)==0){put(m.data()+148,1);m[152]=m[133];}
  for(unsigned v=0;v<6;++v){put(m.data()+192+v*24,8);put(m.data()+196+v*24,0);}
  const auto delta=get(m.data()+156+target*4)-get(m.data()+156+old*4);
  if(delta&&m[152])put(m.data()+188,get(m.data()+188)+delta);
  put(m.data()+76,target);
  CoreSelectionSync246 sync;std::copy_n(m.begin(),132,sync.windows.begin());uint32_t r2=0;
  const bool ok=sync.sync(false,r2,[&](uint32_t endpoint,uint32_t a,uint32_t b,uint32_t c){
   if(calls>=4)return;
   const unsigned offset=calls++*24;
   const std::array<uint32_t,6> fields{{endpoint,a,b,c,get(sync.windows.data()),get(sync.windows.data()+76)}};
   for(unsigned i=0;i<6;++i)put(trace.data()+offset+i*4,fields[i]);
   if(calls==delay)put(sync.windows.data(),get(sync.windows.data()+76));
  },4);
  if(!ok)return false;
  std::copy(sync.windows.begin(),sync.windows.end(),m.begin());
  m[144]=0;return true;
 }
};
}
