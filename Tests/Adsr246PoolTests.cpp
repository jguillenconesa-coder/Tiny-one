// SPDX-License-Identifier: AGPL-3.0-only
#include "Adsr246Pool.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace pocket::reference;
namespace {
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
uint32_t word(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
void put(uint8_t* p,uint32_t w){for(unsigned j=0;j<4;++j)p[j]=uint8_t(w>>(j*8));}
std::vector<uint8_t> read(const std::string& path){std::ifstream f(path,std::ios::binary);check(bool(f),"Missing ADSR pool fixture");return {std::istreambuf_iterator<char>(f),{}};}
Adsr246State state(const uint8_t* p){Adsr246State s;s.stage=word(p);s.blocks=word(p+4);s.level=signedPhase(word(p+8));s.field12=word(p+12);s.field16=word(p+16);s.gain=word(p+20);return s;}
void save(uint8_t* p,const Adsr246State& s){const std::array<uint32_t,6> words{{s.stage,s.blocks,uint32_t(s.level),s.field12,s.field16,s.gain}};for(size_t i=0;i<6;++i)put(p+i*4,words[i]);}
}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string d=argv[1];
 const auto input=read(d+"/adsr246-pool-input.bin"),output=read(d+"/adsr246-pool-output.bin");
 constexpr size_t cases=1024,record=188,result=176;
 check(input.size()==cases*record&&output.size()==cases*result,"Invalid pool fixture dimensions");
 std::array<uint8_t,172> image{};size_t cold=0,warm=0,mutations=0;std::array<bool,6> indices{};
 for(size_t c=0;c<cases;++c){const auto* p=input.data()+c*record;const auto* e=output.data()+c*result;
  if(word(p))std::memcpy(image.data(),p+16,image.size());
  if(word(p+8)<36){put(image.data()+20+word(p+8)*4,word(p+12));++mutations;}
  const auto index=word(p+4);check(index<6,"Invalid original index");indices[index]=true;
  std::array<Adsr246State,6> pool;for(size_t v=0;v<6;++v)pool[v]=state(image.data()+20+v*24);
  auto ready=word(image.data()+4);if(ready==0)++cold;else ++warm;
  auto* selected=adsr246Get(pool,ready,int(index));check(selected==&pool[index],"Wrong selected state");
  put(image.data()+4,ready);for(size_t v=0;v<6;++v)save(image.data()+20+v*24,pool[v]);
  if(std::memcmp(image.data(),e,image.size())!=0)throw std::runtime_error("ADSR pool image differs at case "+std::to_string(c));
  check(word(e+172)==0xff8000d4u+index*24,"Original returned address differs");
 }
 for(bool covered:indices)check(covered,"Missing valid index");check(cold>100&&warm>500&&mutations>500,"Missing cold/warm/retained coverage");
 // Invalid-index behavior belongs to the adapter, never to the original assert.
 std::array<Adsr246State,6> pool{};pool[0].stage=9;uint32_t ready=0;
 for(int i:{-1,6,127,2147483647})check(adsr246Get(pool,ready,i)==nullptr&&ready==0&&pool[0].stage==9,"Rejected adapter call mutated pool");
 std::cout<<"PASS: "<<cases<<" original ADSR accesses, "<<cold<<" cold / "<<warm<<" warm; "<<mutations<<" retained-state mutations; six returned pointers and full guarded pool/ready images\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
