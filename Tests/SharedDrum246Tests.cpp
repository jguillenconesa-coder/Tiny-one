// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{using namespace pocket;
 for(double rate:{8000.,32000.,44100.,48000.,96000.,192000.}){Sample s;s.rate=rate;s.frames.resize(size_t(rate*13),.25f);check(prepareSharedDrum246(s),"Shared source rejected valid rate");check(s.frames.size()==size_t(rate*12)&&s.drumPcm.size()==529200,"Shared source cap or PCM rate is wrong");}
 {Sample bad;check(!prepareSharedDrum246(bad),"Empty source accepted");bad.frames={1};bad.rate=std::numeric_limits<double>::quiet_NaN();check(!prepareSharedDrum246(bad),"Invalid source rate accepted");}
 Sample source;source.rate=44100;source.frames.resize(44100);for(size_t i=0;i<source.frames.size();++i)source.frames[i]=float(.2*std::sin(i*.04)+.1*std::sin(i*.113));prepareSharedDrum246(source);
 Kit shared,reference;shared[0]=source;for(auto& s:reference)s=source;
 Parameters p;p.engine=5;p.drum246=true;p.tone=.5f;p.effect=0;for(int i=0;i<24;++i){p.pads[size_t(i)]=sharedDrum246Region(i);check(p.pads[size_t(i)].start<p.pads[size_t(i)].end,"Empty initial region");if(i)check(p.pads[size_t(i-1)].end==p.pads[size_t(i)].start,"Initial region gap");}
 size_t cases=0;
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.})for(int mode=0;mode<3;++mode)for(int modulation=0;modulation<5;++modulation){
  auto a=std::make_unique<Engine>(),b=std::make_unique<Engine>();a->setKit(shared,24,true);b->setKit(reference,24);a->prepare(rate);b->prepare(rate);auto q=p;q.valueVoice246=modulation==1;q.valueVoice246Colour=1;q.valueVoice246Amount=18000;q.random246=modulation==2;q.random246Controls={{18000,32767,0,24000}};q.tremolo246=modulation==3;q.tremolo246Controls={{12000,8000,8000,16000}};q.midi246=modulation==4;q.midi246Routes={{1,2,3,8}};for(auto& pad:q.pads)pad.nativeMode=mode;a->next(q);b->next(q);
  check(a->isSharedKit()&&a->getKit()[23].frames.empty()&&a->getKit()[0].drumPcm.size()==44100,"Shared kit duplicates or loses its asset");
  for(int note=60;note<84;++note){a->noteOn(note,.7f,note%3+1);b->noteOn(note,.7f,note%3+1);}check(a->visualState(5).active==12,"Shared kit voice stealing exceeds twelve voices");double energy=0;
  for(int i=0;i<4096;++i){if(i==1037){a->midiController(2,80);b->midiController(2,80);}if(i==2049){a->allNotesOff();b->allNotesOff();}const auto x=a->next(q),y=b->next(q);check(x==y,"Shared source differs from identical explicit per-pad reference PCM");check(std::isfinite(x)&&std::abs(x)<8,"Nonfinite or excessive shared-source output");energy+=x*x;}check(energy>0,"Shared source silent");a->reset();check(a->visualState(5).active==0,"Shared source panic retains voices");Kit empty;a->swapKit(empty,8);check(!a->isSharedKit()&&a->getKitCount()==8,"Legacy replacement retains shared layout");++cases;
 }
 // Every pad must address its own absolute region rather than an empty slot.
 for(bool native:{false,true})for(int pad=0;pad<24;++pad){auto local=p;local.drum246=native;auto e=std::make_unique<Engine>();e->prepare(48000);e->setKit(shared,24,true);e->next(local);e->noteOn(60+pad,1);double energy=0;for(int i=0;i<512;++i){auto x=e->next(local);energy+=x*x;}check(energy>0,"An individual shared region is silent");}
 std::cout<<"PASS: shared 12s cap, single asset, 24 regions, "<<cases<<" seven-rate/mode/LFO compositions, twelve voices, per-pad reference PCM equality, all-pad reachability and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
