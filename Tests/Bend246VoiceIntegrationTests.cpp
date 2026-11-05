// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket;
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
int main(int argc,char**){try{const bool quick=argc>1;
 Bend246Voice voice;voice.configure(true,0,12000,0,true);check(voice.amplitude(16384)==16384,"Blocked Bend starts gain before a pitch block");std::array<uint32_t,128> block{};block.fill(12345678);voice.pitchBlock(block);voice.configure(true,1,16000,2000,true);for(int i=0;i<128;++i)check(voice.amplitude(16384)==reference::midi246VoiceGain(16384,12000),"Bend changed a captured block");voice.pitchBlock(block);check(block[0]!=12345678,"Bend pitch misses pending block");voice.configure(false,0,0,0,true);check(voice.amplitude(16384)==16384&&!voice.started,"Bend disable retains processing");
 Bend246Control control;for(int n=0;n<100;++n)control.next(true,0,3,0,0,true,1);check(std::abs(int(int16_t(control.state.display&65535u))-16384)<8,"Bend neutral gain does not settle toward unity");
 Sample sample;sample.rate=44100;sample.frames.resize(12000);for(size_t i=0;i<sample.frames.size();++i)sample.frames[i]=float(.3*std::sin(i*.075)+.1*std::sin(i*.113));sample.prepareDrumPcm();Kit kit;for(auto& clip:kit)clip=sample;
 auto render=[&](double rate,int mode,int route,int partition,bool events,int performance=0,int amount=24000){
  auto e=std::make_unique<Engine>();e->prepare(rate);e->setSample(sample);e->setKit(kit,24);
  Parameters p;p.engine=mode==12?0:mode;p.digital246=mode==12;p.phase246=p.phase246Envelope=p.digital246Envelope=p.sampler246=p.sampler246Envelope=true;p.drum246=true;p.drive=0;p.tone=.5f;p.sampleLoop=true;p.sampler246Controls=Sampler246Voice::clipControls(sample);p.bend246=true;p.bend246Destination=3;p.bend246Colour=route;p.bend246Amount=amount;p.bend246Reverse=true;p.hybridVoiceMode=performance;p.hybridGlideMs=50;
  for(auto* env:{&p.fm246Adsr,&p.string246Adsr,&p.phase246Adsr,&p.digital246Adsr,&p.drWave246Adsr,&p.cluster246Adsr,&p.dna246Adsr,&p.voltage246Adsr,&p.dsynth246Adsr,&p.sampler246Adsr})(*env)[2]=16384;
  e->next(p);e->noteOn(60,.8f);std::vector<float> audio;audio.reserve(8192);
  for(int start=0;start<8192;start+=partition)for(int n=start;n<std::min(start+partition,8192);++n){
   if(events&&n>=1537)p.bend246Angle=std::min(12000,(n-1537)*2);if(n==4099&&performance)e->noteOn(67,.7f);if(n==5121&&performance)e->noteOff(67,1);if(n==6145)e->allNotesOff();
   const float v=e->next(p);if(!std::isfinite(v)||std::abs(v)>=8){std::cerr<<"mode "<<mode<<" route "<<route<<" rate "<<rate<<" sample "<<n<<'\n';throw std::runtime_error("Bend voice output unstable");}audio.push_back(v);
  }
  e->reset();check(e->bend246Level()==0&&e->visualState(p.engine).active==0,"Bend voice panic retains owner/latch");return audio;
 };
 size_t cases=0;
 for(double rate:{32000.,44100.,48000.,88200.,96000.,176400.,192000.})if(!quick||rate==48000)for(int mode:{0,1,2,3,4,5,6,7,8,9,10,11,12})for(int route:{0,1}){
  const auto a=render(rate,mode,route,64,true),b=render(rate,mode,route,257,true),neutral=render(rate,mode,route,64,false);
  check(a==b,"Bend voice output depends on host partition");check(std::equal(a.begin(),a.begin()+1537,neutral.begin()),"Bend voice manual input affects output before its timestamp");
  if(mode==4||mode==5||mode==11)check(a==neutral,"Unsupported voice gain/pitch route affects drum/legacy engine");else if(a==neutral){std::cerr<<"mode "<<mode<<" route "<<route<<" rate "<<rate<<'\n';throw std::runtime_error("Supported Bend voice route does not affect audio");}++cases;
 }
 for(int mode:{0,2,3,6,7,8,9,10})for(int performance:{1,2})for(int route:{0,1})check(render(48000,mode,route,64,true,performance)==render(48000,mode,route,257,true,performance),"Bend voice Mono/Legato/glide depends on partition");
 for(int route:{0,1})for(int amount:{-32767,0,32767})check(render(48000,0,route,64,true,0,amount)==render(48000,0,route,257,true,0,amount),"Bend signed amount depends on partition");
 std::cout<<"PASS: "<<cases<<" voice gain/pitch/rate/engine cases; ten supported melodic engines, drum/legacy rejection, manual input capture, shared coefficient and per-voice block capture, 64/257 exact audio, Mono/Legato/glide, retained coefficient routing and panic\n";
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
