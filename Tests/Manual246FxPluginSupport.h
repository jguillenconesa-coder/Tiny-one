// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "PluginProcessor.h"
inline void configureManualFxTest(PocketProcessor& p,int fx,bool crank,int colour){
 p.applyDrWave246Init();auto& r=p.phaseParameters();const char* effects[]={"filter246Enabled","nitro246Enabled","delay246Enabled","grid246Enabled","phone246Enabled","spring246Enabled","cwo246Enabled"};
 for(int i=0;i<7;++i)r.getParameter(effects[i])->setValueNotifyingHost(i==fx?1.f:0.f);
 for(const char* id:{"element246Enabled","midi246Enabled","random246Enabled","tremolo246Enabled","value246Enabled","valueVoice246Enabled","punch246Enabled"})r.getParameter(id)->setValueNotifyingHost(0);
 r.getParameter("crank246Enabled")->setValueNotifyingHost(crank?1.f:0.f);r.getParameter("bend246Enabled")->setValueNotifyingHost(crank?0.f:1.f);
 const char* ids[]={crank?"crank246Destination":"bend246Destination",crank?"crank246Colour":"bend246Colour",crank?"crank246Amount":"bend246Amount",crank?"crank246Angle":"bend246Angle"};const float values[]={3.f,float(colour),24000.f,crank?0.f:8192.f};
 for(int i=0;i<4;++i){auto* q=r.getParameter(ids[i]);q->setValueNotifyingHost(q->convertTo0to1(values[i]));}
}
