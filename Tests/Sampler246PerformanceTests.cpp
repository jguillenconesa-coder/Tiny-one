// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
 using namespace pocket;using namespace pocket::reference;
 Sample sample;sample.rate=44100;sample.frames.resize(44100);for(size_t i=0;i<sample.frames.size();++i)sample.frames[i]=float(std::sin(i*2*pi*440/44100)*.4);sample.prepareDrumPcm();
 // Independent block composition: verified reader plus verified ADSR block.
 // This fixes the explicit host composition, not the original full voice route.
 for(bool reverse:{false,true})for(bool loop:{false,true}){
  Sampler246Voice voice;Phase246Config config;config.prepare(88200);config.filters={};auto c=Sampler246Voice::clipControls(sample);c[0]=Sampler246Voice::encode(500);c[1]=Sampler246Voice::encode(1000);c[2]=Sampler246Voice::encode(1800);c[3]=Sampler246Voice::encode(2500);c[4]=reverse?16384:0;c[5]=12345;c[6]=23456;
  auto raw=c;if(!loop)raw[2]=raw[0];Sampler246State reader;reader.construct();reader.bind(1);reader.reset(raw);Adsr246State envelope;envelope.noteOn();Adsr246Noise noise;
  const std::array<int16_t,4> adsr{{64,8064,20000,16383}};std::array<uint32_t,128> pitch{};pitch.fill(262145);
  for(int block=0;block<64;++block){const bool released=block>=32;if(block==32){reader.release();envelope.noteOff(adsr[2]);}c[7]=raw[7]=int16_t((block%4)*8192);std::array<int16_t,128> audio{},gain{};sampler246Block(reader,raw,pitch,0x7fffffff,{sample.drumPcm.data(),sample.drumPcm.size(),-8},drum246FirCoefficients,audio);adsr246Block(envelope,adsr,32767,noise,gain);
   for(size_t i=0;i<128;++i){const auto actual=voice.source(1,sample,c,loop,config,released,&adsr);const auto expected=float(double(audio[i])/32768.*gain[i]/16384.);check(std::abs(actual-expected)<1e-7f,"Sampler reader/ADSR composition mismatch");}
   check(std::memcmp(&voice.state,&reader,16)==0,"Sampler composition changed reader state");check(voice.output.envelope.stage==envelope.stage&&voice.output.envelope.level==envelope.level&&voice.output.envelope.blocks==envelope.blocks,"Sampler ADSR state mismatch");check(voice.output.envelopeNoise.seed==noise.seed&&voice.output.envelopeNoise.previous==noise.previous,"Sampler ADSR dither mismatch");
  }
 }
 for(double rate:{22050.,32000.,44100.,48000.,88200.,96000.})for(int policy:{1,2})for(bool nativeEnvelope:{false,true}){
  auto engine=std::make_unique<Engine>();engine->prepare(rate);engine->setSample(sample);Parameters p;p.engine=3;p.sampler246=true;p.sampler246Controls=Sampler246Voice::clipControls(sample);p.sampler246Envelope=nativeEnvelope;p.sampler246Adsr[3]=16383;p.sampleLoop=true;p.sampleRoot=69;p.effect=0;p.lfoDepth=0;p.attack=0;p.release=0;p.hybridVoiceMode=policy;p.hybridGlideMs=100;
  engine->next(p);auto render=[&](int count){double energy=0;for(int i=0;i<count;++i){const auto x=engine->next(p);check(std::isfinite(x),"Nonfinite sampler performance output");energy+=x*x;}return energy;};auto state=[&]{return engine->visualState(3);};
  engine->noteOn(57,1,1);check(render(int(rate*.15))>0,"Silent sampler mono note");check(state().active==1&&state().note==57,"Sampler mono allocation failed");const auto first=state().trigger;
  engine->noteOn(69,.7f,2);check((state().trigger>first)==(policy==1),"Sampler Mono/Legato retrigger failed");render(int(rate*.05));check(std::abs(state().frequency-std::sqrt(220.*440.))<440*(std::exp2(1./(rate*.1))-1)+1e-8,"Sampler glide midpoint failed");render(int(std::lround(rate*.1))-int(rate*.05));check(std::abs(state().frequency-440)<1e-8,"Sampler glide target failed");
  {int crossings=0;float previous=0;for(int i=0;i<int(rate*.2);++i){const auto x=engine->next(p);if(previous<=0&&x>0)++crossings;previous=x;}check(std::abs(crossings/.2-440)<11,"Sampler audible glide endpoint not tuned to root");}
  engine->pitchBend(2,-2);check(std::abs(state().frequency-440*std::exp2(-2./12))<.001,"Sampler channel bend failed");engine->noteOff(69,2);render(int(rate*.1));check(state().note==57,"Sampler fallback priority failed");
  engine->sustainPedal(1,true);engine->noteOff(57,1);render(128);check(state().active==1,"Sampler sustain lost note");engine->sustainPedal(1,false);render(int(rate*2));check(state().active==0,"Sampler release left held voice");
  engine->noteOn(60,1);p.hybridVoiceMode=0;render(1);check(state().active==0,"Sampler mode automation retained old voices");for(int note=60;note<68;++note)engine->noteOn(note,.7f);render(128);check(state().active==6,"Sampler six-voice polyphony failed");engine->allNotesOff();render(int(rate*2));check(state().active==0,"Sampler panic left active voices");
 }
 std::cout<<"PASS: sampler independent loop/fine/volume plus ADSR composition, dither/state, six rates, Mono/Legato/glide, bend/sustain, mode changes and panic\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
