// SPDX-License-Identifier: AGPL-3.0-only
#include "Fm246Foundation.h"
#include "Fm246Model.h"
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static std::vector<unsigned char> read(const std::filesystem::path& path){std::ifstream f(path,std::ios::binary);check(bool(f),"Missing FM foundation fixture");return {std::istreambuf_iterator<char>(f),{}};}
static uint32_t word(const unsigned char* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static int16_t half(const unsigned char* p){const auto x=uint16_t(p[0])|(uint16_t(p[1])<<8);return int16_t(x<32768?int(x):int(x)-65536);}
int main(int argc,char** argv){try{
 check(argc==2,"Provide FM fixture directory");const std::filesystem::path root(argv[1]);const auto in=read(root/"fm246-foundation-input.bin"),out=read(root/"fm246-foundation-output.bin");check(in.size()==8192*128&&out.size()==2244+8192*128,"Wrong FM fixture sizes");
 std::array<int16_t,8192> sine{};for(size_t i=0;i<sine.size();++i)sine[i]=int16_t(std::lround(std::sin(double(i)*3.14159265358979323846/4096)*32767));
 const auto converted=pocket::reference::fm246ConvertTables(pocket::reference::fm246ModelBits);check(std::memcmp(converted.data(),out.data(),60)==0,"Original FM model conversion differs");
 std::array<pocket::reference::Fm246State,6> pool{};std::fill_n(reinterpret_cast<unsigned char*>(pool.data()),sizeof(pool),0xa5);for(size_t i=0;i<6;++i){pool[i].construct();const auto* y=out.data()+60+i*364;check(std::memcmp(pool.data(),y,360)==0&&word(y+360)==0xffb00000u+uint32_t(i)*60,"FM factory changed pool/neighbor/padding/return");}
 pocket::reference::Fm246State state;int mismatches=0;
 for(size_t n=0;n<8192;++n){const auto* x=in.data()+n*128;const auto* y=out.data()+2244+n*128;const auto action=word(x+64);
  if(action)std::memcpy(&state,x,60);
  if(action==2)state.reset();else if(action==3)state.construct();
  std::array<int16_t,30> source{},table{};for(size_t i=0;i<30;++i)source[i]=half(x+68+i*2);pocket::reference::fm246CopyTables(table,source);
  pocket::reference::fm246AdvanceEnvelopes(state,half(x+60),table,sine);
  std::array<unsigned char,128> actual{};std::memcpy(actual.data(),&state,60);std::memcpy(actual.data()+60,table.data(),60);const uint32_t guards[]={0x13579bdf,0x2468ace0};std::memcpy(actual.data()+120,guards,8);
  if(std::memcmp(actual.data(),y,128)!=0){if(mismatches++<10)for(size_t k=0;k<128;++k)if(actual[k]!=y[k]){std::cerr<<"case "<<n<<" byte "<<k<<" actual "<<int(actual[k])<<" expected "<<int(y[k])<<'\n';break;}}
 }
 check(mismatches==0,"FM foundation state/table mismatch");std::cout<<"PASS: 8192 original FM constructor/reset/table/six-envelope records, full 60-byte states and table/guards; 30 boot conversions and six complete factory pools\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
