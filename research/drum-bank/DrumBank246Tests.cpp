// SPDX-License-Identifier: AGPL-3.0-only
#include "DrumBank246.h"
#include "Drum246Tables.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<unsigned char>;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing drum VoiceCtl fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto v=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(v<32768?int(v):int(v)-65536);}
static void put(Bytes& b,uint32_t w,unsigned bytes=4){for(unsigned i=0;i<bytes;++i)b.push_back(static_cast<unsigned char>(w>>(8*i)));}
static void states(Bytes& b,const pocket::research::DrumBank246& m){for(const auto& s:m.audio.pool)for(auto w:{s.tag,s.handle,uint32_t(s.position),uint32_t(s.mode),s.blocks})put(b,w);}
int main(int argc,char** argv){try{
 using namespace pocket::reference;using pocket::research::DrumBank246;
 check(argc==2,"Provide uncompressed drum VoiceCtl fixtures");const std::filesystem::path root(argv[1]);const auto in=read(root/"drum-bank246-input.bin"),out=read(root/"drum-bank246-output.bin");
 constexpr size_t cases=8192,stride=1156,result=5292,prefix=19968;
 check(in.size()==prefix+cases*stride&&out.size()==2052+cases*result+16896,"Wrong drum VoiceCtl sizes");
 for(size_t i=0;i<1025;++i)check(half(out.data()+i*2)==drum246FirCoefficients[i],"Original FIR conversion differs");
 check(std::equal(in.begin(),in.begin()+16896,out.begin()+2052+cases*result),"Original modified PCM");
 std::array<std::array<int16_t,1024>,8> pcm{};for(size_t b=0;b<8;++b)for(size_t i=0;i<1024;++i)pcm[b][i]=half(in.data()+b*2112+32+i*2);
 std::array<std::array<std::array<int16_t,8>,24>,8> bank{};for(size_t b=0;b<8;++b)for(size_t p=0;p<24;++p)for(size_t i=0;i<8;++i)bank[b][p][i]=half(in.data()+16896+(b*24+p)*16+i*2);
 DrumBank246 model;std::array<int32_t,128> derived{};derived.fill(0x6c6c6c6c);
 size_t bankBegins=0,bankFinishes=0,heldResets=0,resetRows=0,clears=0,clearRows=0,externalCleared=0,pureClears=0,busyPositive=0,started=0,restarted=0,ignored=0,released=0,stolen=0,stale=0,unchangedModes=0,queries=0,renders=0,audible=0;std::array<unsigned,12> selected{};std::array<bool,4096> maskCoverage{};std::array<bool,24> seen{};
 for(size_t n=0;n<cases;++n){const auto* x=in.data()+prefix+n*stride;const auto* y=out.data()+2052+n*result;
  const auto flags=word(x),sound=word(x+4),factory=word(x+8),voice=word(x+12);check(sound<8&&factory<8&&voice<12,"Invalid fixture selection");
  if(flags&1){model.reset(factory);seen.fill(false);}
  if(flags&2){for(size_t v=0;v<12;++v){const auto* p=x+40+v*20;model.audio.pool[v]={word(p),word(p+4),signedPhase(word(p+8)),signedPhase(word(p+12)),word(p+16)};*model.at(0x1070+unsigned(v))=x[280+v];DrumBank246::put(model.at(0x10f4+unsigned(v)*4),word(x+292+v*4));DrumBank246::put(model.at(0x1334+unsigned(v)*4),word(x+340+v*4));}}
  if(n<4096){unsigned mask=0;for(unsigned v=0;v<12;++v)if(x[280+v])mask|=1u<<v;maskCoverage[mask]=true;}
  if(flags&256){check(model.beginBank(),"Invalid supplied bank begin order");++bankBegins;}
  if(flags&512){check(model.finishBank(sound),"Invalid supplied factory finish order");++bankFinishes;}
  if(flags&64){resetRows+=DrumBank246::get(model.held.data()+2560);check(model.retriggerHeld(sound,bank),"Held reset rejected");++heldResets;}
  if((flags&128)&&!(flags&(1|2|4|8|16|32|64|256|512))){check(n>0&&std::equal(y+4,y+244,y-result+4),"Original clear unexpectedly changed reader pool");++pureClears;}
  if(flags&128){clearRows+=DrumBank246::get(model.held.data()+2560);const auto before=model.externalNotes;model.clearRegistry();for(size_t i=0;i<128;++i)externalCleared+=before[i]!=model.externalNotes[i];++clears;}
  const std::array<uint32_t,4> event{word(x+24),word(x+28),word(x+32),word(x+36)};
  if(flags&4){const auto busy=model.globals;const auto slot=model.on(event,signedPhase(word(x+16)),signedPhase(word(x+20)),sound,bank);if(slot<0)++ignored;else{++started;restarted+=seen[event[0]-53];seen[event[0]-53]=true;++selected[size_t(slot)];stolen+=busy[0x1070-0x1038+size_t(slot)]!=0;}}
  if(flags&8){auto previous=model.audio.pool;bool staleOwner=false;for(unsigned i=0;i<DrumBank246::get(model.held.data()+2560);++i){const auto* row=model.held.data()+i*20;if(DrumBank246::get(row)==event[0]){const auto slot=DrumBank246::get(row+16);staleOwner=model.value(0x10f4+slot*4)!=event[0]-53;break;}}
   const auto found=model.off(event[0],sound,bank);released+=found;stale+=found&&staleOwner;if(found){bool unchanged=true;for(size_t v=0;v<12;++v)unchanged&=previous[v].mode==model.audio.pool[v].mode;unchangedModes+=unchanged;}}
  uint32_t query=0x13579bdf;if(flags&32){query=model.query(voice);++queries;}
  const uint32_t busyQuery=(flags&1024)?model.busy():0x2468ace0;busyPositive+=busyQuery==1;
  std::array<uint32_t,128> pitch{};std::array<int16_t,128> gain{},audio{};for(size_t i=0;i<128;++i){pitch[i]=word(x+388+i*4);gain[i]=half(x+900+i*2);audio[i]=int16_t(i%2?0x2468:0x1357);}
  if(flags&16){auto& state=model.audio.pool[voice];const auto sample=(state.handle-0x02010000)/8,pad=model.value(0x1334+voice*4);check(sample<8&&pad<24,"Invalid bound reference or pad");check(drum246Block(state,bank[sound][pad],pitch,gain,{pcm[sample].data(),1024},drum246FirCoefficients,audio,derived),"Render rejected");++renders;bool nonzero=false;for(auto a:audio)nonzero|=a!=0;audible+=nonzero;}
  Bytes expected;expected.reserve(result);put(expected,query);states(expected,model);expected.insert(expected.end(),model.globals.begin(),model.globals.end());expected.insert(expected.end(),model.held.begin(),model.held.end());
  for(auto a:audio)put(expected,uint16_t(a),2);for(auto p:pitch)put(expected,p);for(auto g:gain)put(expected,uint16_t(g),2);for(auto d:derived)put(expected,uint32_t(d));put(expected,model.phaseWord);put(expected,busyQuery);expected.insert(expected.end(),model.externalNotes.begin(),model.externalNotes.end());
  check(expected.size()==result,"Internal result format error");if(!std::equal(expected.begin(),expected.end(),y)){std::cerr<<"case "<<n<<" first differing byte ";for(size_t i=0;i<result;++i)if(expected[i]!=y[i]){std::cerr<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(y[i])<<'\n';break;}throw std::runtime_error("Original drum VoiceCtl differs");}
 }
 for(auto covered:maskCoverage)check(covered,"Busy mask grid incomplete");check(bankBegins==bankFinishes&&bankBegins>0&&heldResets>0&&resetRows>0&&clears>0&&pureClears==clears&&clearRows>0&&externalCleared>0,"Missing bank evidence");

 check(started>0&&stolen>0&&ignored>0&&released>0&&stale>0&&restarted>0&&audible>0,"Missing lifecycle evidence");for(auto count:selected)check(count>0,"Voice not selected");
 std::cout<<"PASS: "<<cases<<" original drum bank cases; "<<bankBegins<<" begins / "<<bankFinishes<<" factory finishes, "<<heldResets<<" held resets / "<<resetRows<<" rows, "<<clears<<" registry-only clears / "<<clearRows<<" removed rows, "<<externalCleared<<" external flags cleared; "<<busyPositive<<" positive busy queries; "<<started<<" started ("<<restarted<<" restarted; "<<stolen<<" occupied), "<<ignored<<" ignored, "<<released<<" held notes removed ("<<stale<<" stale owners; "<<unchangedModes<<" without mode change), "<<queries<<" explicit activity queries, "<<renders<<" rendered blocks / "<<renders*128<<" samples, "<<audible<<" nonzero blocks; all twelve voices selected\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
