// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Manual246FxPluginSupport.h"
inline void configureElementFxTest(PocketProcessor& p,int fx,int source,int colour){
 configureManualFxTest(p,fx,false,2);auto& r=p.phaseParameters();r.getParameter("bend246Enabled")->setValueNotifyingHost(0);r.getParameter("element246Enabled")->setValueNotifyingHost(1);
 const char* ids[]={"element246Source","element246Amount","element246Destination","element246Colour","element246Sensor"};const float values[]={float(source),24000.f,3.f,float(colour),18000.f};
 for(int i=0;i<5;++i){auto* q=r.getParameter(ids[i]);q->setValueNotifyingHost(q->convertTo0to1(values[i]));}
}
