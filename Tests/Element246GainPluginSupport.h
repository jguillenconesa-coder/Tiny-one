// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Midi246PluginSupport.h"
inline void configureElementGainTest(PocketProcessor& p,int mode,int source){
 configureMidiVoiceTestEngine(p,mode);auto& r=p.phaseParameters();for(const char* id:{"crank246Enabled","bend246Enabled","midi246Enabled"})r.getParameter(id)->setValueNotifyingHost(0);r.getParameter("element246Enabled")->setValueNotifyingHost(1);
 const char* ids[]={"element246Destination","element246Colour","element246Source","element246Amount","element246Sensor"};const float values[]={3.f,0.f,float(source),-24000.f,-18000.f};for(int i=0;i<5;++i){auto* q=r.getParameter(ids[i]);q->setValueNotifyingHost(q->convertTo0to1(values[i]));}
}
