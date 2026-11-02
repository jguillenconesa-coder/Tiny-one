// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
 using namespace pocket;using namespace pocket::reference;
 // Adapter composition: independently supply the callback's pitch/gain and
 // compare every sample, retained state and context over note release.
 Phase246Config rate;rate.prepare(88200);rate.filters={};Dbox246Voice voice;Dbox246State state;state.construct();state.bind(7);Dbox246Context a,b;a.initialize();b.initialize();b.reset(state);const auto controls=dbox246Default(7);
 for(int block=0;block<32;++block){if(block==4)state.release();std::array<uint32_t,128> pitch{},derived{};std::array<int16_t,128> gain{},audio{},second{};gain.fill(16384);const double hz=40.*std::exp2(double(controls[0])/4096.)*16.;pitch.fill(uint32_t(std::clamp(hz,0.,88200.*.49)/88200.*4294967296.+.5));check(dbox246Block(state,b,controls,pitch,gain,rate.table,audio,derived,second),"Direct D-box failed");for(int i=0;i<128;++i)check(voice.source(1,controls,a,7,rate,block>=4)==audio[size_t(i)]/32768.f,"D-box gain/pitch/filter composition mismatch");state.active();check(std::memcmp(&state,&voice.state,sizeof(state))==0&&std::memcmp(&a,&b,sizeof(a))==0,"D-box retained state/context mismatch");}
 // Editing another pad must never change the active pad's signal.
 auto render=[](bool change){auto e=std::make_unique<Engine>();Parameters p;p.engine=11;p.effect=0;p.volume=1;if(change)p.dbox246Controls[23].fill(32767);e->prepare(48000);e->selectEngine(11);e->noteOn(60,1);std::array<float,8192> samples{};for(auto& v:samples)v=e->next(p);return samples;};check(render(false)==render(true),"Unplayed pad edit leaked into active voice");
 for(double hostRate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  auto e=std::make_unique<Engine>();Parameters p;p.engine=11;p.effect=0;p.volume=1;e->prepare(hostRate);e->selectEngine(11);for(int i=0;i<24;++i)e->noteOn(60+i,.8f,i%3+1);check(e->visualState(11).active==12,"D-box allocation is not twelve voices");double energy=0;
  for(int i=0;i<int(hostRate*.08);++i){const auto x=e->next(p);check(std::isfinite(x)&&std::abs(x)<4,"D-box render unstable");energy+=double(x)*x;}check(energy>.001,"D-box bank is silent");
  e->allNotesOff();for(int i=0;i<int(hostRate*8);++i){const auto x=e->next(p);check(std::isfinite(x),"D-box release nonfinite");}check(e->visualState(11).active==0,"D-box release left a voice running");
  e->noteOn(83,1);for(int i=0;i<128;++i)e->next(p);e->reset();for(int i=0;i<128;++i)check(e->next(p)==0,"D-box panic left audio");
  e->selectEngine(11);e->noteOn(60,.8f,2);e->sustainPedal(2,true);e->noteOff(60,2);for(int i=0;i<128;++i)e->next(p);check(e->visualState(11).active==1,"D-box sustain lost voice");e->sustainPedal(2,false);e->allNotesOff(2);e->selectEngine(0);check(e->visualState(11).active==0,"D-box engine switch retained voices");
 }
 // Eight controls at all corners, all eight mixes, both pitch endpoints and
 // Tremolo shapes while twelve voices are occupied; fixed stack storage.
 for(int shape=0;shape<5;++shape){auto e=std::make_unique<Engine>();Parameters p;p.engine=11;p.effect=0;p.tremolo246=true;p.tremolo246Shape=shape;p.tremolo246Controls={20000,12000,18000,16000};e->prepare(48000);e->selectEngine(11);for(int i=0;i<12;++i)e->noteOn(i%2?127:0,1);
  for(int corner=0;corner<256;++corner){for(auto& pad:p.dbox246Controls)for(int k=0;k<8;++k)pad[size_t(k)]=corner&(1<<k)?32767:0;for(int i=0;i<128;++i)check(std::isfinite(e->next(p)),"D-box control corner unstable");}
 }
 check(dbox246Pad(60)==0&&dbox246Pad(83)==23&&dbox246Pad(84)==0,"D-box note mapping changed");
 std::cout<<"PASS: D-box adapter/core composition, 24 independent pads, twelve voices/stealing, seven rates, channels/sustain/release/panic/switching, control corners and Tremolo\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
