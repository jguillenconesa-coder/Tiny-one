// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Midi246PluginSupport.h"
inline void configureCrankVoiceTest(PocketProcessor& p,int mode,int target){
 configureMidiVoiceTestEngine(p,mode);auto& r=p.phaseParameters();r.getParameter("midi246Enabled")->setValueNotifyingHost(0);r.getParameter("bend246Enabled")->setValueNotifyingHost(0);r.getParameter("crank246Enabled")->setValueNotifyingHost(1);
 for(const auto& item:std::array<std::pair<const char*,float>,3>{{{"crank246Destination",3},{"crank246Colour",float(target)},{"crank246Amount",24000}}}){auto* value=r.getParameter(item.first);value->setValueNotifyingHost(value->convertTo0to1(item.second));}
}
