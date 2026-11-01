// SPDX-License-Identifier: AGPL-3.0-only
#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(){try{
 auto engine=std::make_unique<pocket::Engine>();pocket::Parameters p;p.engine=6;p.volume=1;p.effect=0;
 for(double rate:{44100.,48000.,88200.,96000.}){
  engine->prepare(rate);engine->selectEngine(6);engine->noteOn(69,1);
  double energy=0;int crossings=0;float previous=0;
  for(int i=0;i<int(rate);++i){float s=engine->next(p);check(std::isfinite(s)&&std::abs(s)<1,"Invalid output");if(i>=int(rate*.1)){energy+=s*s;if(previous<=0&&s>0)++crossings;}previous=s;}
  check(energy>1,"Silent default patch");check(std::abs(crossings/.9-440)<3,"Host-rate tuning drift");
  engine->pitchBend(1,2);crossings=0;
  for(int i=0;i<int(rate);++i){float s=engine->next(p);if(i>=int(rate*.1)&&previous<=0&&s>0)++crossings;previous=s;}
  check(std::abs(crossings/.9-440*std::exp2(2./12))<3,"Pitch bend failed");
  engine->sustainPedal(1,true);engine->noteOff(69);for(int i=0;i<1000;++i)engine->next(p);check(engine->visualState(6).active==1,"Sustain released key early");
  p.drWave246Adsr[3]=16383;engine->sustainPedal(1,false);for(int i=0;i<int(rate*2);++i)engine->next(p);check(engine->visualState(6).active==0,"Release did not finish");p.drWave246Adsr[3]=8192;
 }
 engine->prepare(48000);engine->selectEngine(6);
 for(int note=40;note<52;++note){engine->noteOn(note,.8f);for(int i=0;i<300;++i)engine->next(p);check(engine->visualState(6).active<=6,"Exceeded six voice limit");}
 check(engine->visualState(6).active==6,"Missing voices after stealing");
 // Exercise all legal corner combinations and MIDI extremes with held voices.
 for(int corner=0;corner<16;++corner){for(size_t k=0;k<4;++k)p.drWave246Controls[k]=int16_t(corner&(1<<k)?pocket::drWave246Max[k]:0);engine->noteOn(corner&1?127:0,1);for(int i=0;i<8192;++i){float s=engine->next(p);check(std::isfinite(s)&&std::abs(s)<=1,"Unstable control corner");}}
 engine->reset();check(engine->visualState(6).active==0,"Panic retained voices");engine->selectEngine(6);engine->noteOn(69,1);engine->next(p);engine->selectEngine(0);check(engine->visualState(0).active==0,"Engine change retained incompatible voices");
 // Each coloured control must alter the rendered waveform, independently.
 std::vector<float> baseline;
 for(int control=-1;control<4;++control){engine->reset();engine->selectEngine(6);p.drWave246Controls={0,0,0,0};if(control>=0)p.drWave246Controls[size_t(control)]=int16_t(control==3?10000:4096);engine->noteOn(57,1);double difference=0;for(size_t i=0;i<16000;++i){float s=engine->next(p);if(control<0)baseline.push_back(s);else difference+=std::abs(s-baseline[i]);}if(control>=0){check(difference>1,"Colour control is inaudible");}}
 std::cout<<"PASS: Dr. Wave hybrid tuning at four rates, bend, sustain/release, six voices/steal, control corners and audible controls\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
