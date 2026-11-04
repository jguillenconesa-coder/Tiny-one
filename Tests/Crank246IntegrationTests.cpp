// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{using namespace pocket;using namespace pocket::reference;
 for(double rate:{8000.,32000.,44100.,48000.,88200.,96000.,192000.}){
  Crank246Control control;Crank246State expected;std::array<int16_t,8> c{};double position=1;unsigned ticks=0;
  for(int frame=0;frame<int(rate*1.5);++frame){const int angle=frame<int(rate*.8)?(int((frame/37)%256)-128)*64:0;c[0]=int16_t(angle);if(position>=1){crank246Prepare(expected);crank246Advance(expected,c);position-=1;++ticks;}control.next(true,angle,24000,0,0,44100./(64*rate));check(control.state.words==expected.words&&control.state.history==expected.history&&control.state.previous==expected.previous,"Crank host tick or shared input state mismatch");position+=44100./(64*rate);}
  check(ticks>0,"Crank clock never advanced");control.next(false,0,0,0,0,1);check(!control.active&&control.phase()==0,"Crank disable retains live source");
 }
 // Direction, fractional stepping, inertia and quiet settling of the host source.
 Crank246Control motion;int nonzero=0;bool positive=false,negative=false;
 for(int tick=0;tick<8000;++tick){const int angle=tick<1500?((tick*5)%256-128)*64:tick<3000?((256-(tick*5)%256)%256-128)*64:0;motion.next(true,angle,24000,0,0,1);const int velocity=signedPhase(motion.state.words[3]);positive|=velocity>0;negative|=velocity<0;nonzero+=velocity!=0;}
 check(positive&&negative&&nonzero>500,"Crank does not respond in both directions");check(motion.state.words[3]==0&&motion.state.words[5]==0,"Crank inertia does not settle after quiet input");const auto stopped=motion.state.words[2];for(int tick=0;tick<200;++tick)motion.next(true,0,24000,0,0,1);check(motion.state.words[2]==stopped,"Quiet Crank does not retain its stopped phase");

 Sample sample;sample.rate=44100;sample.frames.resize(12000);for(size_t i=0;i<sample.frames.size();++i)sample.frames[i]=float(.3*std::sin(i*.075)+.1*std::sin(i*.113));sample.prepareDrumPcm();Kit kit;for(auto& s:kit)s=sample;
 auto render=[&](double rate,int mode,int destination,bool enabled,int block,bool competing=false){auto e=std::make_unique<Engine>();e->prepare(rate);e->setSample(sample);e->setKit(kit,24);Parameters p;p.engine=mode==12?0:mode;p.digital246=mode==12;p.phase246=p.phase246Envelope=p.digital246Envelope=p.sampler246=p.sampler246Envelope=true;p.drum246=true;p.effect=0;p.drive=0;p.tone=.5f;p.crank246=enabled;p.crank246Destination=destination;p.crank246Colour=destination==1?3:mode==3&&destination==0?7:0;p.crank246Amount=-24000;p.sampleLoop=true;p.sampler246Controls=Sampler246Voice::clipControls(sample);p.punch246=destination==2;for(auto& pad:p.pads)pad.nativeMode=2;
  if(competing){p.bend246=p.midi246=p.random246=p.tremolo246=p.value246=p.valueVoice246=true;p.lfoDepth=.7f;}
  for(int i=0;i<int(rate*.025);++i)e->next(p);e->noteOn(60,.8f,1);std::vector<float> result;
  for(int start=0;start<16384;start+=block)for(int i=start;i<std::min(start+block,16384);++i){if(i%521==0)p.crank246Angle=(int(i/521*29)%256-128)*64;if(i==12289)e->noteOff(60,1);const float v=e->next(p);check(std::isfinite(v)&&std::abs(v)<8,"Crank audio is unstable");result.push_back(v);}e->reset();check(e->crank246Phase()==0&&e->visualState(p.engine).active==0,"Crank panic retains source or voices");return result;};
 size_t cases=0;for(double rate:{32000.,44100.,48000.,88200.,96000.,176400.,192000.})for(int mode:{0,1,2,3,5,6,7,8,9,10,11,12})for(int destination=0;destination<3;++destination){const auto audio=render(rate,mode,destination,true,64),other=render(rate,mode,destination,true,257),base=render(rate,mode,destination,false,64);check(audio==other,"Crank audio depends on buffer partition");if(destination==1&&(mode==5||mode==11))check(audio==base,"Drum Crank ADSR should be rejected");else if(audio==base){std::cerr<<"Crank unchanged: rate "<<rate<<" mode "<<mode<<" dest "<<destination<<'\n';throw std::runtime_error("Supported Crank destination does not affect audio");}++cases;}
 check(render(48000,0,0,true,64)==render(48000,0,0,true,64,true),"Crank simultaneous-mode priority differs in engine");
 std::cout<<"PASS: Crank clock/source at seven rates, "<<cases<<" engine/destination/rate cases, rejected drum ADSR, exact 64/257 audio, priority and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
