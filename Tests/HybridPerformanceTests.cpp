// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){try{
 for(double rate:{8000.,44100.,48000.,88200.,96000.,192000.})for(int mode:{0,2,6,7})for(int policy:{1,2}){
  auto engine=std::make_unique<pocket::Engine>();pocket::Parameters p;
  p.engine=mode;p.hybridVoiceMode=policy;p.hybridGlideMs=100;p.effect=0;
  p.fm246Controls[0]=0;p.string246Adsr[3]=p.fm246Adsr[3]=p.drWave246Adsr[3]=p.cluster246Adsr[3]=16383;p.cluster246Controls={3072,0,512,3};
  engine->prepare(rate);engine->selectEngine(mode);engine->configureHybridPerformance(policy,100);
  auto render=[&](int count){double energy=0;for(int i=0;i<count;++i){const float s=engine->next(p);check(std::isfinite(s)&&std::abs(s)<1,"Unstable performance output");energy+=s*s;}return energy;};
  auto state=[&]{return engine->visualState(mode);};
  auto note=[&](int expected,int channel){check(state().active==1&&state().note==expected&&state().channel==channel,"Last-note/channel priority failed");};
  engine->noteOn(57,1,1);check(render(int(rate*.2))>.01,"Silent first mono note");note(57,1);const auto trigger=state().trigger;
  engine->noteOn(69,.7f,2);note(69,2);
  check((state().trigger>trigger)==(policy==1),"Mono/Legato envelope retrigger policy failed");
  check(policy==1?state().envelope==0:state().envelope>.01,"Legato lost envelope / Mono failed to reset it");
  render(int(rate*.05));check(std::abs(state().frequency-std::sqrt(220.*440.))<.01,"Glide not linear in semitones / host-rate independent");
  render(int(rate*.05));check(std::abs(state().frequency-440)<1e-8,"Glide did not reach exact target");
  int crossings=0;float previous=0;
  for(int i=0;i<int(rate*.4);++i){const float sample=engine->next(p);if(previous<=0&&sample>0)++crossings;previous=sample;}
  check(mode==2||std::abs(crossings/.4-440)<8,"Audible glide endpoint not tuned to target note");
  engine->pitchBend(1,2);check(std::abs(state().frequency-440)<1e-8,"Bend leaked across channels");
  engine->pitchBend(2,-2);check(std::abs(state().frequency-440*std::exp2(-2./12))<.001,"Current-channel bend failed");
  engine->noteOff(69,2);note(57,1);render(int(rate*.1));check(std::abs(state().frequency-220*std::exp2(2./12))<.001,"Fallback pitch/bend failed");
  // Releasing a non-priority key must not retrigger or change pitch.
  engine->noteOn(60,1,3);render(100);const auto latest=state().trigger;
  engine->noteOff(57,1);note(60,3);check(state().trigger==latest,"Releasing older key retriggered voice");
  engine->sustainPedal(3,true);engine->noteOff(60,3);render(100);note(60,3);
  engine->noteOn(72,1,4);render(100);engine->noteOff(72,4);note(60,3);
  engine->sustainPedal(4,false);render(100);note(60,3);
  engine->sustainPedal(3,false);render(int(rate*2));check(state().active==0,"Pedal-up left a hanging mono note");
  // A detached note starts immediately at its own pitch, even during a release.
  engine->pitchBend(1,0);engine->noteOn(81,1);check(std::abs(state().frequency-880)<1e-8,"Isolated note glides from stale pitch");render(100);
  engine->noteOn(57,1,2);render(100);engine->allNotesOff(2);note(81,1);
  engine->allNotesOff();render(int(rate*2));check(state().active==0,"All-notes-off revived priority stack");
  // Same pitch on independent channels and the internal sequencer channel.
  engine->noteOn(60,1,1);engine->noteOn(60,.5f,17);render(100);note(60,17);
  engine->noteOff(60,1);note(60,17);engine->noteOn(60,0,17);render(int(rate*2));check(state().active==0,"Zero-velocity note-on not released");
  engine->noteOn(40,1);engine->noteOn(80,1);engine->reset();engine->sustainPedal(1,false);render(128);check(state().active==0,"Panic retained held notes");
  for(int channel=1;channel<=17;++channel)for(int n=0;n<128;++n)engine->noteOn(n,.5f,channel);
  render(128);note(127,17);
  for(int channel=17;channel>=1;--channel)for(int n=127;n>=0;--n)engine->noteOff(n,channel);
  render(int(rate*2));check(state().active==0,"Full MIDI range overflowed / left hanging priority notes");
  engine->noteOn(40,1);p.hybridVoiceMode=0;render(1);check(state().active==0,"Mode automation retained incompatible voices");
  for(int n=60;n<66;++n)engine->noteOn(n,1);render(128);check(state().active==6,"Polyphony not restored");
  p.hybridVoiceMode=2;render(1);check(state().active==0,"Mono mode change retained poly voices");
  engine->noteOn(60,1);engine->selectEngine(mode==6?7:6);check(state().active==0,"Engine switch retained performance voice");
  engine->selectEngine(mode);engine->noteOff(60);render(1);check(state().active==0,"Engine switch retained priority notes");
 }
 // Glide trajectory remains bounded on interrupts, downward motion and zero time.
 pocket::HybridGlide glide;glide.start(880,220,2000,48000);
 for(int i=0;i<24000;++i)glide.next();check(std::abs(glide.value-880/std::sqrt(2.))<1e-6,"Downward glide incorrect");
 glide.start(glide.value,1760,0,48000);check(glide.next()==1760,"Zero-time glide did not snap");
 std::cout<<"PASS: Mono/Legato routing, semitone glide at six rates, per-channel bend/sustain, fallback, panic/mode/engine changes and restored six-voice polyphony\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
