// SPDX-License-Identifier: AGPL-3.0-only
#include "Drum246Voice.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{using namespace pocket;using namespace pocket::reference;
 // The reference suite compares the complete original callback. These checks
 // cover the authored bridge's domain, continuity and velocity normalization.
 auto bank=std::make_unique<Drum246Bank>();
 for(unsigned code=1;code<128;++code){bank->reset();const float input=float(code)/127.f;check(bank->noteOn(60,input,1,0,48000),"MIDI velocity rejected");check(bank->hits[0].velocityWord==int(code*256),"Event velocity word not used by host");check(bank->hits[0].velocity==input,"MIDI-normalized amplitude changed");bank->noteOff(60,1);}
 check(drum246HostVelocityWord(.8f)==102*256,"GUI velocity not quantized to MIDI");check(drum246HostVelocityWord(1e-6f)==0&&drum246HostVelocityWord(2)==32512,"Host velocity bounds failed");
 check(drum246HostVelocityWord(std::numeric_limits<float>::quiet_NaN())==0,"Nonfinite velocity not bounded");
 for(int semi=-24;semi<=24;++semi){uint32_t word=0;check(drum246EventPitchWord(semi,word),"Coarse pitch missing");check(drum246HostPitchIncrement(semi,1)==word*2u,"Integer pitch lost original event word");}
 size_t cases=0;
 for(double bend:{.25,.5,1.,2.,4.}){uint32_t previous=0;for(int step=-24*1024;step<=24*1024;++step){const double semi=step/1024.;const auto actual=drum246HostPitchIncrement(semi,bend);const double analytic=std::clamp(.5*std::exp2(semi/12.)*bend*268435456.,0.,2147483647.);check(actual>=previous,"Fine host pitch is not monotone");check(std::abs(double(actual)-analytic)<=std::max(2.,analytic*3e-6),"Authored fine bridge diverges from semitone scale");previous=actual;++cases;}}
 for(int semi=-23;semi<=24;++semi){const auto center=drum246HostPitchIncrement(semi,1);check(drum246HostPitchIncrement(semi-1e-8,1)<=center&&drum246HostPitchIncrement(semi+1e-8,1)>=center,"Coarse/fine boundary jumps backwards");}
 check(drum246HostPitchIncrement(0,1)==0x08000000&&drum246HostPitchIncrement(-100,1)==drum246HostPitchIncrement(-24,1)&&drum246HostPitchIncrement(100,1)==drum246HostPitchIncrement(24,1),"Host range/neutral changed");
 check(drum246HostPitchIncrement(0,-1)==0&&drum246HostPitchIncrement(0,std::numeric_limits<double>::quiet_NaN())==0&&drum246HostPitchIncrement(0,1e300)==0x7fffffffu,"Host bend/saturation bounds failed");
 Sample sample;sample.drumPcm.resize(4410);for(size_t i=0;i<sample.drumPcm.size();++i)sample.drumPcm[i]=int16_t(std::sin(i*.061)*24000);PadSettings pad;pad.nativeMode=2;
 for(double rate:{8000.,22050.,44100.,48000.,88200.,96000.,192000.})for(float semi:{-24.f,-7.75f,-1.f,0.f,.25f,1.f,11.5f,24.f}){
  Phase246Config cfg;cfg.prepare(rate);cfg.filters={};Drum246Voice voice;pad.pitch=semi;
  const auto controls=drum246BaseControls(pad,sample);Drum246State ref;ref.construct();ref.bind(1);ref.reset(controls);
  for(int block=0;block<3;++block){std::array<uint32_t,128> pitch{};pitch.fill(drum246HostPitchIncrement(semi,1));std::array<int16_t,128> gain{},audio{};gain.fill(16384);std::array<int32_t,128> derived{};check(drum246Block(ref,controls,pitch,gain,{sample.drumPcm.data(),sample.drumPcm.size(),-8},drum246FirCoefficients,audio,derived),"Direct bounded callback failed");for(int i=0;i<128;++i)check(voice.source(1,pad,sample,drum246EnvelopeDefaults,cfg,false,&controls)==audio[size_t(i)]/32768.f,"Reader ignored new event-derived host pitch");}
 }
 std::cout<<"PASS: event velocity words for all 127 MIDI note-ons, GUI quantization, integer word anchors, "<<cases<<" fine/bend checks and 56 seven-rate direct reader compositions\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
