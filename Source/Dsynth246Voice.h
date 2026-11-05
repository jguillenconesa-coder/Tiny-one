// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Dsynth246.h"
#include "Phase246.h"
namespace pocket {
inline constexpr std::array<const char*,8> dsynth246Ids{{"dsynth246Blue","dsynth246Green","dsynth246White","dsynth246Orange","dsynth246BlueII","dsynth246GreenII","dsynth246WhiteII","dsynth246OrangeII"}};
inline constexpr std::array<const char*,4> dsynth246EnvelopeIds{{"dsynth246Attack","dsynth246Decay","dsynth246Sustain","dsynth246Release"}};
inline constexpr std::array<int16_t,8> dsynth246Defaults{{12000,2000,16000,5000,32767,2000,16000,16000}};
// Own 88.2kHz clock, pitch, allocation/ADSR composition and output resampling.
// Gain and Tremolo amplitude are supplied before the reconstructed core filters.
struct Dsynth246Voice {
 reference::Dsynth246State state;Phase246Voice output;
 std::array<int16_t,128> block{},gain{},second{};std::array<uint32_t,128> derived{};
 unsigned cursor=128;bool initialized=false,releaseSent=false,finished=false,silentBlock=false;
 bool done() const noexcept {return finished;}
 float source(double frequency,const std::array<int16_t,8>& controls,const std::array<int16_t,4>& adsr,reference::Dsynth246Context& context,int slot,const Phase246Config& rate,bool released) noexcept {
  if(!initialized){state={};state.construct();state.bind(slot);context.reset(state,controls);output.envelope.noteOn();output.envelopeStarted=true;initialized=true;}
  if(released&&!releaseSent){state.release();output.envelope.noteOff(adsr[2]);releaseSent=true;}
  if(cursor==128){
   std::array<uint32_t,128> pitch{};pitch.fill(uint32_t(std::clamp(frequency,0.,Phase246Config::internalRate*.49)/Phase246Config::internalRate*4294967296.+.5));output.tremolo.pitchBlock(pitch,rate.table);output.midi.pitchBlock(pitch);output.bend.pitchBlock(pitch);
   gain.fill(0);reference::adsr246Block(output.envelope,adsr,32767,output.envelopeNoise,gain);for(auto& v:gain)v=output.bend.amplitude(output.midi.amplitude(output.tremolo.amplitude(v,rate.table)));
   auto bounded=controls;for(auto& v:bounded)v=int16_t(std::clamp(int(v),0,32767));
   reference::dsynth246Block(state,context,bounded,pitch,gain,rate.table,block,derived,second);silentBlock=output.envelopeDone()&&std::all_of(block.begin(),block.end(),[](int16_t v){return v>=-1&&v<=1;});cursor=0;
  }
  double sample=block[cursor++]/32768.;if(cursor==128&&silentBlock)finished=true;for(size_t i=0;i<2;++i){const auto& c=rate.filters[i];auto& m=output.memory[i];const double out=c.b0*sample+m[0];m[0]=c.b1*sample-c.a1*out+m[1];m[1]=c.b2*sample-c.a2*out;sample=out;}return float(sample);
 }
 float next(double frequency,const std::array<int16_t,8>& controls,const std::array<int16_t,4>& adsr,reference::Dsynth246Context& context,int slot,const Phase246Config& rate,bool released=false) noexcept {
  if(!output.started){output.first=source(frequency,controls,adsr,context,slot,rate,released);output.second=source(frequency,controls,adsr,context,slot,rate,released);output.started=true;}
  const float result=output.first+float(output.position)*(output.second-output.first);output.position+=rate.step;
  while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(frequency,controls,adsr,context,slot,rate,released);}return result;
 }
};
}
