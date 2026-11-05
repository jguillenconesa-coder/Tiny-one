// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket;
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
int main(int argc,char**){try{const bool quick=argc>1;
 Bend246Voice bridge;bridge.configure(true,0,12000,0,true,1);std::array<uint32_t,128> pitch{};pitch.fill(12345678);bridge.pitchBlock(pitch);bridge.configure(true,1,16000,4000,true,2);check(!bridge.started,"Crank inherited a captured Bend block");bridge.pitchBlock(pitch);check(pitch[0]!=12345678&&bridge.owner==2,"Crank owner capture failed");bridge.configure(false,0,0,0,true,2);check(!bridge.enabled&&bridge.owner==0,"Crank disable retains voice owner");
 Crank246Control control;for(int n=0;n<100;++n)control.next(true,0,0,3,0,1);check(std::abs(int(int16_t(control.gainState&65535u))-16384)<8,"Crank zero amount fails unity smoothing");const auto retained=control.gainState;for(int n=0;n<100;++n)control.next(true,0,20000,3,1,1);check(control.gainState==retained,"Crank Pitch advances inactive gain coefficient");
 Sample sample;sample.rate=44100;sample.frames.resize(12000);for(size_t i=0;i<sample.frames.size();++i)sample.frames[i]=float(.3*std::sin(i*.075)+.1*std::sin(i*.113));sample.prepareDrumPcm();Kit kit;for(auto& clip:kit)clip=sample;
 auto render=[&](double rate,int mode,int route,int partition,bool events,int performance=0,int amount=24000){
  auto e=std::make_unique<Engine>();e->prepare(rate);e->setSample(sample);e->setKit(kit,24);
  Parameters p;p.engine=mode==12?0:mode;p.digital246=mode==12;p.phase246=p.phase246Envelope=p.digital246Envelope=p.sampler246=p.sampler246Envelope=true;p.drum246=true;p.drive=0;p.tone=.5f;p.sampleLoop=true;p.sampler246Controls=Sampler246Voice::clipControls(sample);p.crank246=true;p.crank246Destination=3;p.crank246Colour=route;p.crank246Amount=amount;p.hybridVoiceMode=performance;p.hybridGlideMs=50;
  for(auto* env:{&p.fm246Adsr,&p.string246Adsr,&p.phase246Adsr,&p.digital246Adsr,&p.drWave246Adsr,&p.cluster246Adsr,&p.dna246Adsr,&p.voltage246Adsr,&p.dsynth246Adsr,&p.sampler246Adsr})(*env)[2]=16384;
  for(int n=0;n<int(rate*.2);++n)e->next(p);e->noteOn(60,.8f);const double duration=amount<0?.65:.35;const int length=int(rate*duration),event=int(rate*.02);bool crossedNegativePitch=false;std::vector<float> audio;audio.reserve(size_t(length));
  for(int start=0;start<length;start+=partition)for(int n=start;n<std::min(start+partition,length);++n){
   if(events&&n>=event){const int phase=(128+int((n-event)*1000/rate))%256;p.crank246Angle=(phase-128)*64;}if(n==int(rate*.15)&&performance)e->noteOn(67,.7f);if(n==int(rate*.22)&&performance)e->noteOff(67,1);if(n==int(rate*(duration-.07)))e->allNotesOff();
   const float v=e->next(p);const int phase=int(e->crank246Phase()*256);if(events&&amount<0&&route==1&&n<int(rate*(duration-.07))&&phase>=74&&phase<=181)crossedNegativePitch=true;if(!std::isfinite(v)||std::abs(v)>=8){std::cerr<<"mode "<<mode<<" route "<<route<<" rate "<<rate<<" sample "<<n<<'\n';throw std::runtime_error("Crank voice output unstable");}audio.push_back(v);
  }
  if(events&&amount==-32767&&route==1)check(crossedNegativePitch,"Crank negative test never reached the negative-pitch domain");
  e->reset();check(e->crank246Phase()==0&&e->visualState(p.engine).active==0,"Crank voice panic retains owner/latch");return audio;
 };
 size_t cases=0;
 for(double rate:{32000.,44100.,48000.,88200.,96000.,176400.,192000.})if(!quick||rate==48000)for(int mode:{0,1,2,3,4,5,6,7,8,9,10,11,12})for(int route:{0,1}){
  const auto a=render(rate,mode,route,64,true),b=render(rate,mode,route,257,true),neutral=render(rate,mode,route,64,false);
  check(a==b,"Crank voice output depends on host partition");check(std::equal(a.begin(),a.begin()+int(rate*.02),neutral.begin()),"Crank voice manual input affects output before its timestamp");
  if(mode==4||mode==5||mode==11)check(a==neutral,"Unsupported voice gain/pitch route affects drum/legacy engine");else if(a==neutral){std::cerr<<"mode "<<mode<<" route "<<route<<" rate "<<rate<<'\n';throw std::runtime_error("Supported Crank voice route does not affect audio");}++cases;
 }
 for(int mode:{0,2,3,6,7,8,9,10})for(int performance:{1,2})for(int route:{0,1})check(render(48000,mode,route,64,true,performance)==render(48000,mode,route,257,true,performance),"Crank voice Mono/Legato/glide depends on partition");
 for(int mode:{0,1,2,3,6,7,8,9,10,12})for(int route:{0,1})for(int amount:{-32767,0,32767})check(render(48000,mode,route,64,true,0,amount)==render(48000,mode,route,257,true,0,amount),"Crank signed amount depends on partition");
 std::cout<<"PASS: "<<cases<<" voice gain/pitch/rate/engine cases; ten supported melodic engines, drum/legacy rejection, manual input capture, shared coefficient and per-voice block capture, 64/257 exact audio, Mono/Legato/glide, retained coefficient routing and panic\n";
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
