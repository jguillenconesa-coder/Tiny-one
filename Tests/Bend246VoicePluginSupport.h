// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Midi246PluginSupport.h"
inline void configureBendVoiceTest(PocketProcessor& p,int mode,int target){
 configureMidiVoiceTestEngine(p,mode);auto& r=p.phaseParameters();r.getParameter("midi246Enabled")->setValueNotifyingHost(0);r.getParameter("bend246Enabled")->setValueNotifyingHost(1);r.getParameter("bend246Reverse")->setValueNotifyingHost(1);
 for(const auto& item:std::array<std::pair<const char*,float>,3>{{{"bend246Destination",3},{"bend246Colour",float(target)},{"bend246Amount",target==0?-24000.f:24000.f}}}){auto* value=r.getParameter(item.first);value->setValueNotifyingHost(value->convertTo0to1(item.second));}
}
