// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace pocket;
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
int main(){try{
 const std::array<int,4> level{{19,0,0,0}},pitch{{20,0,0,0}},off{};
 const std::array<uint32_t,4> cc{{64,64,64,64}};
 Midi246Voice blocked;blocked.configure(true,level,cc,true);check(blocked.amplitude(16000)==16000,"Blocked gain starts before its pitch block");
 std::array<uint32_t,128> words{};words.fill(12345678);blocked.pitchBlock(words);check(blocked.display[0]==2048,"Gain coefficient was not updated exactly once");
 const auto held=blocked.display;blocked.configure(true,level,{{0,64,64,64}},true);
 for(int i=0;i<128;++i)blocked.amplitude(16000);check(blocked.display==held&&!blocked.started,"CC changed an already captured voice block");
 blocked.pitchBlock(words);check(blocked.display[0]<held[0],"Next voice block missed the pending CC");
 const auto retained=blocked.display;blocked.configure(true,off,cc,true);check(blocked.display==retained,"Parameter-only routing clears retained gain display");blocked.configure(false,off,cc,true);check(blocked.display==std::array<int16_t,4>{},"Disabling MIDI retains per-voice display");
 // Independent owners must not advance one another's gain smoothing.
 std::array<Midi246Voice,6> owners{};for(auto& voice:owners)voice.configure(true,level,cc,false);
 owners[0].pitch(100);owners[0].amplitude(16384);for(size_t i=1;i<owners.size();++i)check(owners[i].display==std::array<int16_t,4>{},"Gain state leaked between voices");
 Sample sample;sample.rate=44100;sample.frames.resize(12000);for(size_t i=0;i<sample.frames.size();++i)sample.frames[i]=float(.3*std::sin(i*.075)+.1*std::sin(i*.113));sample.prepareDrumPcm();Kit kit;for(auto& clip:kit)clip=sample;
 auto render=[&](double rate,int mode,int route,int partition,bool events,int performance=0){
  auto e=std::make_unique<Engine>();e->prepare(rate);e->setSample(sample);e->setKit(kit,24);
  Parameters p;p.engine=mode==12?0:mode;p.digital246=mode==12;p.phase246=p.phase246Envelope=p.digital246Envelope=p.sampler246=p.sampler246Envelope=true;p.drum246=true;p.drive=0;p.tone=.5f;p.sampleLoop=true;p.sampler246Controls=Sampler246Voice::clipControls(sample);p.midi246=true;p.midi246Routes={{route,0,0,0}};p.hybridVoiceMode=performance;p.hybridGlideMs=50;
  e->next(p);e->noteOn(60,.8f);std::vector<float> audio;audio.reserve(8192);
  for(int start=0;start<8192;start+=partition)for(int n=start;n<std::min(start+partition,8192);++n){
   if(events&&n==1537)e->midiController(1,0);if(events&&n==3073)e->midiController(1,127);if(n==4099&&performance)e->noteOn(67,.7f);if(n==5121&&performance)e->noteOff(67,1);if(n==6145)e->allNotesOff();
   const float v=e->next(p);if(!std::isfinite(v)||std::abs(v)>=8){std::cerr<<"mode "<<mode<<" route "<<route<<" rate "<<rate<<" sample "<<n<<'\n';throw std::runtime_error("MIDI voice output unstable");}audio.push_back(v);
  }
  e->reset();check(e->midiControllers()==cc&&e->visualState(p.engine).active==0,"MIDI voice panic retains owner/latch");return audio;
 };
 size_t cases=0;
 for(double rate:{32000.,44100.,48000.,88200.,96000.,176400.,192000.})for(int mode:{0,1,2,3,4,5,6,7,8,9,10,11,12})for(int route:{19,20}){
  const auto a=render(rate,mode,route,64,true),b=render(rate,mode,route,257,true),neutral=render(rate,mode,route,64,false);
  check(a==b,"MIDI voice output depends on host partition");check(std::equal(a.begin(),a.begin()+1537,neutral.begin()),"MIDI voice CC affects output before its timestamp");
  if(mode==4||mode==5||mode==11)check(a==neutral,"Unsupported voice gain/pitch route affects drum/legacy engine");else if(a==neutral){std::cerr<<"mode "<<mode<<" route "<<route<<" rate "<<rate<<'\n';throw std::runtime_error("Supported MIDI voice route does not affect audio");}++cases;
 }
 for(int mode:{0,2,3,6,7,8,9,10})for(int performance:{1,2})for(int route:{19,20})check(render(48000,mode,route,64,true,performance)==render(48000,mode,route,257,true,performance),"MIDI voice Mono/Legato/glide depends on partition");
 std::cout<<"PASS: "<<cases<<" voice gain/pitch/rate/engine cases; ten supported melodic engines, drum/legacy rejection, CC capture, six independent owners, 64/257 exact audio, Mono/Legato/glide, retained coefficient routing and panic\n";
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
