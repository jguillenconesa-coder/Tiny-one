// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
inline constexpr const char* randomFxTestEffects[]={"punch246Enabled","filter246Enabled","nitro246Enabled","delay246Enabled","grid246Enabled","phone246Enabled","spring246Enabled","cwo246Enabled"};
inline void configureRandomFxTest(PocketProcessor& p,int fx){
 p.applyDrWave246Init();auto& raw=p.phaseParameters();
 for(const char* id:{"element246Enabled","crank246Enabled","bend246Enabled","midi246Enabled","value246Enabled","valueVoice246Enabled","tremolo246Enabled"})raw.getParameter(id)->setValueNotifyingHost(0);
 for(int i=0;i<8;++i)raw.getParameter(randomFxTestEffects[i])->setValueNotifyingHost(i==fx?1.f:0.f);
 raw.getParameter("random246Enabled")->setValueNotifyingHost(1);raw.getParameter("random246Destination")->setValueNotifyingHost(1);raw.getParameter("random246Speed")->setValueNotifyingHost(1);
 for(const auto& pair:std::array<std::pair<const char*,float>,2>{{{"random246Amount",24000},{"random246Envelope",16000}}}){auto* q=raw.getParameter(pair.first);q->setValueNotifyingHost(q->convertTo0to1(pair.second));}
}
