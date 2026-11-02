// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static pocket::Kit kit(){pocket::Kit k;for(int pad=0;pad<24;++pad){auto& s=k[size_t(pad)];s.rate=44100;s.frames.resize(4410);for(size_t i=0;i<s.frames.size();++i)s.frames[i]=float(std::sin(i*(.05+pad*.003))*.45);s.prepareDrumPcm();}return k;}
int main(){try{
 using namespace pocket;using namespace pocket::reference;
 // Exact flat-gain composition of an independently supplied callback, including
 // the host's explicit eight-frame PCM alignment and original note release.
 Phase246Config rate;rate.prepare(88200);rate.filters={};Drum246Voice voice;Sample sample;sample.drumPcm.resize(264600);for(size_t i=0;i<8192;++i)sample.drumPcm[i]=int16_t(std::lround(std::sin(i*.07)*24000));PadSettings pad;const std::array<int16_t,8> controls{{0,0,16384,8192,0,0,0,16384}};Drum246State state;state.construct();state.bind(1);state.reset(controls);
 for(int block=0;block<16;++block){if(block==4)state.release(controls);std::array<uint32_t,128> pitch{};pitch.fill(0x08000000);std::array<int16_t,128> gain{},audio{};gain.fill(16384);std::array<int32_t,128> derived{};check(drum246Block(state,controls,pitch,gain,{sample.drumPcm.data(),sample.drumPcm.size(),-8},drum246FirCoefficients,audio,derived),"Direct reader failed");for(int i=0;i<128;++i)check(voice.source(1,pad,sample,drum246EnvelopeDefaults,rate,block>=4)==audio[size_t(i)]/32768.f,"Native sampler gain/pitch/alignment composition mismatch");check(std::memcmp(&state,&voice.state,sizeof(state))==0,"Native sampler state handoff mismatch");}
 // Host model contracts, without attributing the curve to original instructions.
 const std::array<int16_t,4> curve{{0,32767,0,32767}};check(drum246DynamicLevel(0,curve)==0&&drum246DynamicLevel(.5,curve)==1&&drum246DynamicLevel(1,curve)==0,"Dynamic region endpoints failed");check(std::abs(drum246DynamicLevel(.25,curve)-.5f)<1e-6,"Dynamic interpolation failed");check(drum246DynamicLevel(.99,drum246EnvelopeDefaults)==1,"Zero-region dynamics is not neutral");
 {Sample s;s.rate=48000;s.frames.resize(48000*13,1.f);s.prepareDrumPcm();check(s.drumPcm.size()==529200&&s.drumPcm[100]==32767,"Offline cache rate/12s cap/clipping failed");check(s.frames.size()==48000*13,"Cache conversion damaged original audio");s.frames[0]=std::numeric_limits<float>::quiet_NaN();s.prepareDrumPcm();check(s.drumPcm[0]==0,"Nonfinite import was not bounded");}
 for(double hostRate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.}){
  auto e=std::make_unique<Engine>();e->setKit(kit(),24);e->prepare(hostRate);Parameters p;p.engine=5;p.drum246=true;p.effect=0;p.volume=1;p.tone=.5;for(auto& pad:p.pads)pad.nativeMode=2;e->selectEngine(5,false,true);for(int n=60;n<84;++n)e->noteOn(n,.8f,n%3+1);check(e->visualState(5).active==12,"Native kit is not twelve voices");double energy=0;
  for(int i=0;i<int(hostRate*.15);++i){const auto v=e->next(p);check(std::isfinite(v)&&std::abs(v)<4,"Native kit unstable");energy+=v*v;}check(energy>.001&&e->visualState(5).active==12,"Native loops stopped or are silent");e->allNotesOff();for(int i=0;i<int(hostRate*.1);++i)e->next(p);check(e->visualState(5).active==0,"Native loop release hung");
  e->reset();e->selectEngine(5,false,true);p.pads[0].nativeMode=1;e->noteOn(60,1);for(int i=0;i<int(hostRate*.01);++i)e->next(p);e->noteOff(60);for(int i=0;i<int(hostRate*.02);++i)e->next(p);check(e->visualState(5).active==1,"One Shot stopped at note-off");for(int i=0;i<int(hostRate*.2);++i)e->next(p);check(e->visualState(5).active==0,"One Shot did not end");
  p.pads[0].nativeMode=0;e->noteOn(60,1,2);e->sustainPedal(2,true);e->noteOff(60,2);for(int i=0;i<int(hostRate*.02);++i)e->next(p);check(e->visualState(5).active==1,"Gated sustain lost voice");e->sustainPedal(2,false);for(int i=0;i<int(hostRate*.04);++i)e->next(p);check(e->visualState(5).active==0,"Gated note-off did not stop");e->noteOn(60,1);e->next(p);e->reset();for(int i=0;i<128;++i)check(e->next(p)==0,"Native sampler panic left audio");
 }
 auto render=[](bool tremolo,int shape,int pitch,int amplitude,bool region){auto e=std::make_unique<Engine>();e->setKit(kit(),24);e->prepare(48000);Parameters p;p.engine=5;p.drum246=true;p.pads[0].nativeMode=2;p.effect=0;p.volume=1;p.tone=.5;p.tremolo246=tremolo;p.tremolo246Shape=shape;p.tremolo246Controls={12000,int16_t(pitch),int16_t(amplitude),16000};if(region)p.drum246Dynamics={0,32767,0,30000};e->selectEngine(5,false,true);e->noteOn(60,1);std::array<float,16384> out{};for(auto& x:out){x=e->next(p);check(std::isfinite(x),"Native modulation nonfinite");}return out;};
 const auto baseline=render(false,0,0,0,false);check(baseline==render(true,0,0,0,false),"Zero-depth Tremolo changed native kit");for(int shape=0;shape<5;++shape)for(bool pitch:{false,true}){const auto changed=render(true,shape,pitch?12000:0,pitch?0:18000,false);double difference=0;for(size_t i=0;i<changed.size();++i)difference+=std::abs(changed[i]-baseline[i]);check(difference>.1,"Native kit Tremolo inaudible");}check(render(false,0,0,0,true)!=baseline,"Own dynamics is inaudible");
 std::cout<<"PASS: native kit composition, guarded PCM/cache, twelve voices, seven rates, Gated/One Shot/Loop, sustain/panic, own dynamics and five Tremolo shapes\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
