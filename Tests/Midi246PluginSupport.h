// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "PluginProcessor.h"
inline void configureMidiVoiceTestEngine(PocketProcessor& p,int mode){
 switch(mode){
  case 1:p.applyPhase246Preset(0);break;
  case 3:p.applyPreset(18);p.phaseParameters().getParameter("sampler246Enabled")->setValueNotifyingHost(1);p.phaseParameters().getParameter("sampler246Envelope")->setValueNotifyingHost(1);p.state.getParameter("sampleLoop")->setValueNotifyingHost(1);break;
  case 6:p.applyDrWave246Init();break;case 7:p.applyCluster246Init();break;
  case 8:p.applyDna246Init();break;case 9:p.applyVoltage246Init();break;case 10:p.applyDsynth246Init();break;
  case 12:p.applyDigital246Preset(0);break;
  default:p.applyPreset(mode==2?3:0);if(mode==0||mode==2)for(size_t i=0;i<4;++i)for(const char* id:{mode==2?pocket::string246Ids[i]:pocket::fm246Ids[i],mode==2?pocket::string246EnvelopeIds[i]:pocket::fm246EnvelopeIds[i]}){auto* value=p.phaseParameters().getParameter(id);value->setValueNotifyingHost(value->getDefaultValue());}break;
 }
 if(mode==1||mode==12)for(size_t i=0;i<4;++i){const auto* id=mode==1?pocket::phase246EnvelopeIds[i]:pocket::digital246EnvelopeIds[i];auto* value=p.phaseParameters().getParameter(id);value->setValueNotifyingHost(value->getDefaultValue());}
 for(const auto* id:{"effect","drive","lfoDepth"}){auto* value=p.state.getParameter(id);value->setValueNotifyingHost(value->convertTo0to1(0));}
 p.phaseParameters().getParameter("midi246Enabled")->setValueNotifyingHost(1);
}
