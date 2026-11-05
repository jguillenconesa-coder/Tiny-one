// SPDX-License-Identifier: AGPL-3.0-only
#include "CoreDrumSampler246.h"
#include "Drum246Tables.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<unsigned char>;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static Bytes read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing Core drum fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto v=unsigned(p[0])|(unsigned(p[1])<<8);return int16_t(v<32768?int(v):int(v)-65536);}
static void put(Bytes& b,uint32_t w,unsigned bytes=4){for(unsigned i=0;i<bytes;++i)b.push_back(static_cast<unsigned char>(w>>(8*i)));}
static void states(Bytes& b,const pocket::research::CoreDrumSampler246& m){for(const auto& s:m.pool)for(auto w:{s.tag,s.handle,uint32_t(s.position),uint32_t(s.mode),s.blocks})put(b,w);}
int main(int argc,char** argv){try{
 using namespace pocket::reference;using pocket::research::CoreDrumSampler246;
 check(argc==2,"Provide uncompressed Core drum fixtures");const std::filesystem::path root(argv[1]);const auto in=read(root/"drum-core246-input.bin"),out=read(root/"drum-core246-output.bin");
 constexpr size_t cases=4608,stride=1096,result=5156,prefix=19968;
 check(in.size()==prefix+cases*stride&&out.size()==2052+cases*result+16896,"Wrong Core drum fixture sizes");
 for(size_t i=0;i<1025;++i)check(half(out.data()+i*2)==drum246FirCoefficients[i],"Original FIR conversion differs");
 check(std::equal(in.begin(),in.begin()+16896,out.begin()+2052+cases*result),"Original Core modified PCM");
 std::array<std::array<int16_t,1024>,8> pcm{};for(size_t b=0;b<8;++b)for(size_t i=0;i<1024;++i)pcm[b][i]=half(in.data()+b*2112+32+i*2);
 std::array<std::array<std::array<int16_t,8>,24>,8> bank{};for(size_t b=0;b<8;++b)for(size_t p=0;p<24;++p)for(size_t i=0;i<8;++i)bank[b][p][i]=half(in.data()+16896+(b*24+p)*16+i*2);
 CoreDrumSampler246 model;model.addressPool();std::array<int32_t,128> derived{};derived.fill(0x6c6c6c6c);
 size_t factories=0,resets=0,releases=0,renders=0,audible=0,mismatchedSound=0;std::array<unsigned,2304> coverage{};
 for(size_t n=0;n<cases;++n){const auto* x=in.data()+prefix+n*stride;const auto* y=out.data()+2052+n*result;
  const auto flags=word(x),voice=word(x+4),sound=word(x+8),factorySound=word(x+12);check(voice<12&&sound<8,"Invalid fixture selection");
  const auto pad=word(x+24+voice*4);check(pad<24,"Invalid fixture pad");auto& controls=bank[sound][pad];for(size_t i=0;i<8;++i)controls[i]=half(x+312+i*2);
  if(flags&1)for(size_t v=0;v<12;++v){const auto* s=x+72+v*20;model.pool[v]={word(s),word(s+4),signedPhase(word(s+8)),signedPhase(word(s+12)),word(s+16)};}
  if(flags&2){check(model.factory(factorySound),"Factory rejected sound");++factories;}
  Bytes expected;expected.reserve(result);const auto address=CoreDrumSampler246::controlAddress(sound,pad);
  put(expected,flags&4?CoreDrumSampler246::receiver(voice,half(x+16)):0x13579bdf);put(expected,flags&4?address:0x13579bdf);
  put(expected,flags&8?CoreDrumSampler246::receiver(voice,half(x+20)):0x13579bdf);put(expected,flags&8?address:0x13579bdf);put(expected,((flags&4)?1u:0u)|((flags&8)?2u:0u));
  if(flags&4){check(model.dispatch(voice,sound,pad,false,controls),"Reset rejected");++resets;coverage[(sound*24+pad)*12+voice]|=1;}
  if(flags&8){check(model.dispatch(voice,sound,pad,true,controls),"Release rejected");++releases;coverage[(sound*24+pad)*12+voice]|=2;}
  for(auto pointer:model.pointers)put(expected,pointer);states(expected,model);
  std::array<uint32_t,128> pitch{};std::array<int16_t,128> gain{},audio{};for(size_t i=0;i<128;++i){pitch[i]=word(x+328+i*4);gain[i]=half(x+840+i*2);audio[i]=int16_t(i%2?0x2468:0x1357);}
  if(flags&16){auto& state=model.pool[voice];check(state.handle>=0x02010000&&(state.handle-0x02010000)%8==0,"Bad sample reference");const auto sample=(state.handle-0x02010000)/8;check(sample<8,"Bad sample bank");if(sample!=sound)++mismatchedSound;
   check(drum246Block(state,controls,pitch,gain,{pcm[sample].data(),1024},drum246FirCoefficients,audio,derived),"Render rejected");++renders;bool nonzero=false;for(auto a:audio)nonzero|=a!=0;audible+=nonzero;
  }
  states(expected,model);for(auto a:audio)put(expected,uint16_t(a),2);for(auto p:pitch)put(expected,p);for(auto g:gain)put(expected,uint16_t(g),2);for(auto d:derived)put(expected,uint32_t(d));
  for(const auto& b:bank)for(const auto& c:b)for(auto h:c)put(expected,uint16_t(h),2);
  check(expected.size()==result,"Internal result format error");if(!std::equal(expected.begin(),expected.end(),y)){std::cerr<<"case "<<n<<" first differing byte ";for(size_t i=0;i<result;++i)if(expected[i]!=y[i]){std::cerr<<i<<" expected "<<unsigned(expected[i])<<" original "<<unsigned(y[i])<<'\n';break;}throw std::runtime_error("Original Core drum contract differs");}
 }
 for(auto c:coverage)check(c==3,"Reset/release grid incomplete");check(audible>0&&mismatchedSound>0,"Fixture lacks sample-selection evidence");
 const auto before=model.pool;const auto pointers=model.pointers;const std::array<int16_t,8> c{};
 check(!model.factory(8)&&!model.dispatch(12,0,0,false,c)&&!model.dispatch(0,8,0,false,c)&&!model.dispatch(0,0,24,true,c),"Own bounded guards failed");
 Bytes prior,after;CoreDrumSampler246 snapshot;snapshot.pool=before;snapshot.pointers=pointers;states(prior,snapshot);states(after,model);check(prior==after&&model.pointers==pointers,"Own invalid-input guard mutated state");
 std::cout<<"PASS: "<<cases<<" original Core drum cases; "<<factories<<" factories, "<<resets<<" resets, "<<releases<<" releases, "<<renders<<" rendered blocks / "<<renders*128<<" samples, "<<audible<<" nonzero blocks, "<<mismatchedSound<<" bound/selected sound differences; complete 12 x 8 x 24 reset/release grid\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
