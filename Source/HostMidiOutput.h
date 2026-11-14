// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace pocket {
// Host adaptation: fixed event storage and source ownership, not USB timing.
class HostMidiOutput {
public:
 enum Source:uint8_t { keyboard=1,sequence=2 };
 struct Event {int sample=0;std::array<uint8_t,3> bytes{};};
 static constexpr size_t normalBudget=1024,noteCount=16*128,capacity=2*normalBudget+noteCount;
 void begin(bool enabled){used=normal=0;blocked=false;if(!enabled&&active)flush(0);active=enabled;}
 void noteOn(Source source,int channel,int note,float velocity,int frame){
  if(!active||blocked||!valid(channel,note)||!std::isfinite(velocity))return;
  if(velocity<=0){noteOff(source,channel,note,frame);return;}
  auto& owner=owners[index(channel,note)];if(owner&source)return;
  if(owner){owner|=source;return;}
  const auto value=uint8_t(std::clamp(int(std::clamp(velocity,0.f,1.f)*127.f+.5f),1,127));
  if(emit(frame,uint8_t(0x90|(channel-1)),note,value))owner=source;
 }
 void noteOff(Source source,int channel,int note,int frame){
  if(!valid(channel,note))return;auto& owner=owners[index(channel,note)];if(!(owner&source))return;
  const auto remaining=uint8_t(owner&~source);if(remaining){owner=remaining;return;}
  if(emit(frame,uint8_t(0x80|(channel-1)),note,64))owner=0;
 }
 void releaseKeyboardChannel(int channel,int frame){if(channel<1||channel>16)return;for(int n=0;n<128;++n)noteOff(keyboard,channel,n,frame);}
 void flush(int frame){for(int ch=1;ch<=16;++ch)for(int n=0;n<128;++n){auto& owner=owners[index(ch,n)];if(owner){emergency(frame,uint8_t(0x80|(ch-1)),n,64);owner=0;}}}
 const Event* data()const{return events.data();}size_t size()const{return used;}bool overflowed()const{return blocked;}
 size_t held()const{size_t n=0;for(auto v:owners)n+=v!=0;return n;}
private:
 static bool valid(int ch,int n){return ch>=1&&ch<=16&&n>=0&&n<128;}
 static size_t index(int ch,int n){return size_t((ch-1)*128+n);}
 bool emit(int frame,uint8_t status,int note,int velocity){if(blocked)return false;if(normal==normalBudget){blocked=true;flush(frame);return false;}++normal;emergency(frame,status,note,velocity);return true;}
 void emergency(int frame,uint8_t status,int note,int velocity){
  // Total offs <= initially active notes + emitted ons; normal events <= budget.
  // Therefore capacity = 2048 + 2*budget covers any number of panic/flush calls.
  if(used<events.size())events[used++]={std::max(0,frame),{{status,uint8_t(note),uint8_t(velocity)}}};
 }
 std::array<uint8_t,noteCount> owners{};std::array<Event,capacity> events{};
 size_t used=0,normal=0;bool active=false,blocked=false;
};
}
