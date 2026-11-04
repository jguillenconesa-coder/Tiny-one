// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{using namespace pocket;
 for(double rate:{8000.,32000.,44100.,48000.,88200.,96000.,192000.}){
  Element246Control c;const double step=44100./(64*rate);const int frames=int(std::ceil(1/step));
  for(int i=0;i<frames;++i){c.next(true,1,24000,0,0,-12345,i==0?.5f:-1.f,.8f,-.25f,step);if(i+1<frames)check(c.inputs.externalPeak==0,"Element frontend publishes incomplete window");}
  check(c.inputs.externalPeak==16384&&c.inputs.level==8192,"Element input positive peak/own level scale mismatch");check(c.inputs.sensor==-12345&&c.inputs.drumEnvelope==26214&&c.inputs.synthGainPeak==6553&&c.inputs.synthEnvelope==6553,"Element sensor/host envelope words mismatch");
  for(int i=0;i<frames;++i)c.next(true,1,24000,0,0,0,-1.f,0,0,step);check(c.inputs.externalPeak==0,"Element external frontend uses absolute peak or stale sample");
  c.next(true,3,24000,0,0,0,std::numeric_limits<float>::quiet_NaN(),0,std::numeric_limits<float>::infinity(),1);check(c.inputs.externalPeak==0&&c.inputs.level==0,"Element frontend fails non-finite input");
  c.next(false,0,0,0,0,0,0,0,0,step);check(!c.active&&c.level()==0,"Element disable retains source");
 }
 Sample sample;sample.rate=44100;sample.frames.resize(12000);for(size_t i=0;i<sample.frames.size();++i)sample.frames[i]=float(.3*std::sin(i*.075)+.1*std::sin(i*.113));sample.prepareDrumPcm();Kit kit;for(auto& s:kit)s=sample;
 auto render=[&](double rate,int mode,int destination,int source,bool enabled,int block,bool competing=false){auto e=std::make_unique<Engine>();e->prepare(rate);e->setSample(sample);e->setKit(kit,24);Parameters p;p.engine=mode==12?0:mode;p.digital246=mode==12;p.phase246=p.phase246Envelope=p.digital246Envelope=p.sampler246=p.sampler246Envelope=true;p.drum246=true;p.effect=0;p.drive=0;p.tone=.5f;p.element246=enabled;p.element246Source=source;p.element246Destination=destination;p.element246Colour=destination==1?3:mode==3&&destination==0?7:mode==7&&destination==0?2:0;p.element246Amount=30000;p.element246Sensor=-20000;p.sampleLoop=true;p.sampler246Controls=Sampler246Voice::clipControls(sample);p.punch246=destination==2;for(auto& pad:p.pads)pad.nativeMode=2;
  if(competing){p.crank246=p.bend246=p.midi246=p.random246=p.tremolo246=p.value246=p.valueVoice246=true;p.lfoDepth=.7f;p.lfoDestination=1;}
  for(int i=0;i<int(rate*.025);++i)e->next(p,.5f);e->noteOn(60,.8f,1);std::vector<float> result;
  for(int start=0;start<8192;start+=block)for(int i=start;i<std::min(start+block,8192);++i){if(i==1537)p.element246Sensor=-12000;if(i==6145)e->noteOff(60,1);const float external=float(.5+.4*std::sin(i*.031));const float v=e->next(p,external);check(std::isfinite(v)&&std::abs(v)<8,"Element audio is unstable");result.push_back(v);}
  e->reset();check(e->element246Level()==0&&e->visualState(p.engine).active==0,"Element panic retains source or voices");return result;};
 size_t cases=0;
 for(double rate:{32000.,44100.,48000.,88200.,96000.,176400.,192000.})for(int mode:{0,1,2,3,5,6,7,8,9,10,11,12})for(int destination=0;destination<3;++destination){
  const auto base=render(rate,mode,destination,0,false,64);
  for(int source=0;source<4;++source){const auto audio=render(rate,mode,destination,source,true,64),other=render(rate,mode,destination,source,true,257);check(audio==other,"Element source/audio depends on buffer partition");if(destination==1&&(mode==5||mode==11))check(audio==base,"Drum Element ADSR should be rejected");else if(audio==base){std::cerr<<"Element unchanged: rate "<<rate<<" mode "<<mode<<" dest "<<destination<<" source "<<source<<'\n';throw std::runtime_error("Supported Element destination does not affect audio");}++cases;}
 }
 check(render(48000,0,0,0,true,64)==render(48000,0,0,0,true,64,true),"Element simultaneous-mode/own-filter priority differs");
 // These cases contain no Value flag that could mask the old own-filter gate.
 auto manual=[&](int mode,bool own){auto e=std::make_unique<Engine>();e->prepare(48000);Parameters p;p.effect=0;p.drive=0;p.tone=.2f;p.lfoDestination=1;p.lfoDepth=own?.7f:0;p.element246=mode==0;p.crank246=mode==1;p.bend246=mode==2;p.midi246=mode==3;p.element246Sensor=-20000;p.crank246Amount=-24000;e->noteOn(60,.8f);std::vector<float> audio;float peak=0;for(int i=0;i<4096;++i){const float v=e->next(p);peak=std::max(peak,std::abs(v));audio.push_back(v);}check(peak>.00001f,"Manual-LFO own-filter test is silent");return audio;};
 for(int mode=0;mode<4;++mode)check(manual(mode,false)==manual(mode,true),"Own LFO tone filter leaks into Element/Crank/Bend/MIDI");
 std::cout<<"PASS: Element source windows at seven rates, "<<cases<<" engine/destination/source/rate cases, rejected drum ADSR, exact 64/257 audio, mode/filter priority and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
