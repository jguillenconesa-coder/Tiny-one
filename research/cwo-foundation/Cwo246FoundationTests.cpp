// SPDX-License-Identifier: AGPL-3.0-only
#include "Cwo246Foundation.h"
#include "Cwo246Tables.h"
#include <cstring>
#include <cmath>
#include <complex>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace pocket::reference;
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static auto read(const std::string& p){std::ifstream f(p,std::ios::binary);check(bool(f),"Missing CWO foundation fixture");return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),{});}
static uint32_t word(const uint8_t* p){uint32_t x;std::memcpy(&x,p,4);return x;}
static int16_t half(const uint8_t* p){int16_t x;std::memcpy(&x,p,2);return x;}
int main(int argc,char** argv){try{
 check(argc==2,"Expected fixture directory");const std::string dir=argv[1];const auto in=read(dir+"/cwo246-foundation-input.bin"),out=read(dir+"/cwo246-foundation-output.bin");
 check(in.size()==65536*196&&out.size()==22684+65536*196,"Incomplete CWO foundation oracle");
 std::array<uint8_t,22684> tables{};std::memcpy(tables.data(),cwo246Constants.data(),100);std::memcpy(tables.data()+100,cwo246Sine.data(),2050);std::memcpy(tables.data()+2152,cwo246Frequency.data(),1028);std::memcpy(tables.data()+3180,cwo246Feedback.data(),2056);std::memcpy(tables.data()+5236,cwo246Delay.data(),1028);std::memcpy(tables.data()+6264,cwo246PhaseHybrid.data(),32);std::memcpy(tables.data()+6296,cwo246Interpolation.data(),16388);check(std::memcmp(tables.data(),out.data(),tables.size())==0,"Runtime CWO tables differ from original producers / declared phase model");
 // Analytic quality budget for the authored coefficients, not OP-1 sound matching.
 double maximumError=0;constexpr double pi=3.14159265358979323846;
 for(int i=0;i<10000;++i){const double f=30*std::pow(18000./30,double(i)/9999),w=2*pi*f/44100;const auto z=std::exp(std::complex<double>(0,-2*w));auto h=std::exp(std::complex<double>(0,-w));
  for(size_t j=0;j<4;++j){const double a=cwo246PhaseHybrid[j]/2147483648.,b=cwo246PhaseHybrid[j+4]/2147483648.;check(a>0&&a<1&&b>0&&b<1,"Unstable authored phase coefficient");h*=((b-z)/(1.-b*z))/((a-z)/(1.-a*z));}
  check(std::abs(std::abs(h)-1)<1e-8,"Phase model no longer allpass");maximumError=std::max(maximumError,std::asin(std::min(1.,std::abs(h.real())))*180/pi);
 }
 check(maximumError<4.3,"Authored phase model exceeds its analytic design-band error budget");
 Cwo246PhaseState phase;size_t failures=0;
 for(size_t i=0;i<65536;++i){const auto* p=in.data()+i*196;const auto* e=out.data()+22684+i*196;std::array<uint8_t,196> result{};
  auto a=cwo246Map32(uint16_t(half(p)),cwo246Frequency);std::memcpy(result.data(),&a,4);auto b=cwo246MapFeedback(uint16_t(half(p+4)),cwo246Feedback);std::memcpy(result.data()+4,b.data(),8);a=cwo246Map32(uint16_t(half(p+2)),cwo246Delay);std::memcpy(result.data()+12,&a,4);
  auto input=half(p+8);const auto clipped=cwo246Clip(input);std::memcpy(result.data()+16,&clipped,2);std::memcpy(result.data()+18,&input,2);
  auto state=half(p+10);const auto output=cwo246Allpass(state,half(p+12),half(p+14));std::memcpy(result.data()+20,&output,2);std::memcpy(result.data()+22,&state,2);
  if(word(p+160))std::memcpy(&phase,p+24,136);std::array<int32_t,2> pair;check(cwo246Phase(phase,signedPhase(word(p+16)),signedPhase(word(p+20)),cwo246PhaseHybrid,pair),"Valid phase state rejected");std::memcpy(result.data()+24,pair.data(),8);std::memcpy(result.data()+32,&phase,136);
  std::array<int32_t,3> filter;std::memcpy(filter.data(),p+164,12);const auto filtered=cwo246Filter(filter,half(p+176),signedPhase(word(p+180)));std::memcpy(result.data()+168,filter.data(),12);std::memcpy(result.data()+180,&filtered,2);
  std::array<int32_t,2> smooth;std::memcpy(smooth.data(),p+184,8);const auto smoothed=cwo246Smooth(smooth,uint16_t(half(p+6)),signedPhase(word(p+192)));std::memcpy(result.data()+184,smooth.data(),8);std::memcpy(result.data()+192,&smoothed,2);
  if(std::memcmp(result.data(),e,196)&&failures++<8){std::cerr<<"case "<<i;for(size_t j=0;j<49;++j)if(word(result.data()+j*4)!=word(e+j*4))std::cerr<<" word "<<j<<" got "<<signedPhase(word(result.data()+j*4))<<" expected "<<signedPhase(word(e+j*4));std::cerr<<'\n';}if(failures>100)break;
 }
 check(failures==0,"CWO building blocks differ from instruction oracle");
 const auto previous=phase;phase.parity=2;std::array<int32_t,2> pair{};check(!cwo246Phase(phase,0,0,cwo246PhaseHybrid,pair),"Invalid phase parity accepted");check(phase.cells==previous.cells,"Rejected phase state modified histories");
 std::cout<<"PASS: CWO original table producers, exhaustive control maps and signed clipper; 65536 allpass/filter/smoother and conditioned dual-phase calls, complete states match\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
