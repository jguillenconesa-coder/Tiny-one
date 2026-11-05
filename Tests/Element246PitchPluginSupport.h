// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Element246GainPluginSupport.h"
inline void configureElementPitchTest(PocketProcessor& p,int mode,int source){
 configureElementGainTest(p,mode,source);auto& r=p.phaseParameters();r.getParameter("element246Colour")->setValueNotifyingHost(1/7.f);auto* amount=r.getParameter("element246Amount");amount->setValueNotifyingHost(amount->convertTo0to1(20000));
}
