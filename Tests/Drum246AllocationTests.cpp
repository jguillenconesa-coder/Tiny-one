// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static pocket::Kit kit(){pocket::Kit k;for(auto& s:k){s.rate=44100;s.frames.resize(4410);for(size_t i=0;i<s.frames.size();++i)s.frames[i]=float(.3*std::sin(i*.071));s.prepareDrumPcm();}return k;}
int main(){try{using namespace pocket;
 auto b=std::make_unique<Drum246Bank>();
 for(int i=0;i<12;++i){check(b->noteOn(60+i,1,1,i,48000),"Initial hit rejected");check(b->hits[size_t(i)].note==60+i,"First-free allocation changed");b->hits[size_t(i)].voice.state.mode=1;b->hits[size_t(i)].voice.state.blocks=2;}
 b->hits[1].voice.state.blocks=99;b->hits[2].voice.state.blocks=99;
 check(b->noteOn(84,.8f,1,0,48000)&&b->hits[1].note==84,"Greatest block count / first tie failed");
 const auto age=b->age;b->hits[0].voice.state.mode=0;b->hits[0].voice.state.blocks=77;
 check(!b->noteOn(84,.2f,1,0,48000)&&b->age==age&&b->hits[0].voice.state.blocks==77,"Duplicate consulted or retriggered voices");
 b->noteOff(61,1);check(b->hits[1].keyDown&&!b->hits[1].released,"Stale note-off released new owner");
 b->noteOff(84,1);check(!b->hits[1].keyDown&&b->hits[1].released,"Owner note-off not routed");
 check(b->noteOn(61,1,1,1,48000)&&b->hits[2].note==61&&b->hits[0].voice.state.blocks==0,"Re-press or inactive query side effect failed");
 b->reset();for(int i=0;i<12;++i)b->noteOn(60+i,1,1,i,48000);for(auto& h:b->hits){h.voice.state.mode=0;h.voice.state.blocks=33;}
 b->hits[4].active=false;check(b->noteOn(90,1,1,6,48000)&&b->hits[4].note==90,"Host retirement did not supply a free voice");
 for(size_t i=0;i<4;++i)check(b->hits[i].voice.state.blocks==0,"Queries before first free were skipped");check(b->hits[5].voice.state.blocks==33,"Selection queried beyond first free");
 b->reset();for(int i=0;i<12;++i){b->noteOn(60+i,1,1,i,48000);b->hits[size_t(i)].voice.state.mode=1;b->hits[size_t(i)].voice.state.blocks=0xffffffffu;}
 check(b->noteOn(100,1,1,0,48000)&&b->hits[0].note==100,"Own wrapped-count fallback failed");
 b->reset();check(b->noteOn(60,1,1,0,48000)&&b->noteOn(60,1,2,0,48000)&&b->noteOn(60,1,17,0,48000),"Host channels were merged");
 b->allNotesOff(2);check(b->heldVoice[1][60]==-1&&b->heldVoice[0][60]>=0&&b->heldVoice[16][60]>=0,"Channel panic damaged other memberships");
 b->allNotesOff(0);for(const auto& ch:b->heldVoice)for(auto v:ch)check(v==-1,"Global panic retained membership");
 b->reset();b->noteOn(60,1,3,0,48000);b->pedal(3,true);b->noteOff(60,3);check(!b->hits[0].keyDown&&!b->hits[0].released,"Sustain released immediately");
 b->noteOn(60,1,3,0,48000);b->pedal(3,false);check(b->hits[0].released&&b->hits[1].keyDown&&!b->hits[1].released,"Sustain confused old and new press");
 b->noteOff(60,3);check(b->hits[1].released,"Re-pressed owner did not release");
 check(!b->noteOn(-1,1,1,0,48000)&&!b->noteOn(60,1,0,0,48000)&&!b->noteOn(60,1,1,24,48000),"Invalid host event accepted");
 // Every supported host key can remain held while only twelve readers exist.
 b->reset();for(int ch=1;ch<=17;++ch)for(int note=0;note<128;++note)check(b->noteOn(note,.8f,ch,note%24,48000),"Bounded host registry lost a distinct key");
 for(int ch=1;ch<=17;++ch)for(int note=0;note<128;++note){if(ch==17&&note==127)continue;b->noteOff(note,ch);}
 check(b->hits[0].channel==17&&b->hits[0].note==127&&b->hits[0].keyDown&&!b->hits[0].released,"Dense stale offs released latest owner");
 b->noteOff(127,17);check(b->hits[0].released,"Dense final owner did not release");b->reset();
 size_t cases=0;
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.})for(int mode=0;mode<3;++mode)for(int modulation=0;modulation<4;++modulation){
  auto a=std::make_unique<Engine>(),c=std::make_unique<Engine>();a->setKit(kit(),24);c->setKit(kit(),24);a->prepare(rate);c->prepare(rate);Parameters p;p.engine=5;p.drum246=true;p.effect=0;p.tone=.5;p.valueVoice246=modulation==1;p.valueVoice246Colour=1;p.valueVoice246Amount=18000;p.valueVoice246Speed=12000;p.random246=modulation==2;p.random246Controls={{18000,32767,0,24000}};p.tremolo246=modulation==3;p.tremolo246Controls={{12000,8000,16000,16000}};for(auto& pad:p.pads)pad.nativeMode=mode;a->next(p);c->next(p);a->noteOn(60,.8f,1);c->noteOn(60,.8f,1);
  double energy=0;for(int i=0;i<int(rate*.18);++i){if(i==31||i==71)c->noteOn(60,.2f,1);if(i==int(rate*.06)){a->noteOff(60,1);c->noteOff(60,1);}const auto x=a->next(p),y=c->next(p);check(x==y,"Duplicate changed rendered reader or per-voice LFO state");check(std::isfinite(y),"Allocation adapter nonfinite");energy+=y*y;}
  check(energy>0,"Allocation case silent");check(a->visualState(5).active==0&&c->visualState(5).active==0,"Host reader retirement hung");
  a->noteOn(60,1);for(int i=0;i<128;++i)a->next(p);check(a->visualState(5).active==1,"Natural completion retained a stale busy flag or key membership was not removed");
  a->allNotesOff();for(int i=0;i<int(rate*.2);++i)a->next(p);check(a->visualState(5).active==0,"Host panic did not drain reader");++cases;
 }
 std::cout<<"PASS: original selector in host, duplicate suppression, stale owners, query order, signed-count fallback, channels/sustain/panic and "<<cases<<" seven-rate reader compositions\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
