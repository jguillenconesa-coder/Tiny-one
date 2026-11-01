// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ReferenceDsp.h"
namespace pocket::reference {
struct Grid246State {
    uint32_t tag=0;int32_t readPlane=0,injectPlane=0,clearPlane=0;
    int32_t readX=1,readY=1,injectX=7,injectY=7,clearX=0,clearY=0,slot=2;
    bool scanFits(int width,int height) const noexcept {return readX>=1&&readX<width-1&&injectX>=1&&injectX<width-1&&readY>=1&&readY<height-1&&injectY>=1&&injectY<height-1;}
    void reset() noexcept {readPlane=injectPlane=clearPlane=0;readX=readY=1;injectX=injectY=7;clearX=clearY=0;}
};
static_assert(sizeof(Grid246State)==44);
using Grid246Memory=std::array<int16_t,135200>;
// Slot 2 is the explicit bounded workspace used by the legible original reset.
// The caller owns the two 260x260 planes; no original allocator is implied.
inline bool grid246Block(Grid246State& s,const std::array<int16_t,4>& c,
                         const std::array<int16_t,8192>& sine,Grid246Memory& memory,
                         std::array<int16_t,64>& audio,std::array<uint32_t,16>& meters) noexcept {
    if(c[0]<1344||c[0]>16704||c[1]<1344||c[1]>16704||c[2]<0||c[3]<0||s.slot!=2)return false;
    const int width=2*(c[0]/128),height=2*(c[1]/128),xLimit=width-1,yLimit=height-1,xPeriod=width-2,yPeriod=height-2,half=xPeriod/2;
    if(!s.scanFits(width,height)||s.readPlane<0||s.readPlane>1||s.injectPlane<0||s.injectPlane>1||s.clearPlane<0||s.clearPlane>1||s.clearX<0||s.clearX>258||s.clearY<0||s.clearY>258)return false;
    auto at=[&](int p,int y,int x)->int16_t& {return memory[size_t(p)*67600+size_t(y)*260+size_t(x)];};
    auto shifted=[&](int x){x+=half;if(x>=xLimit)x-=xPeriod;return x;};
    const int increment=4194304/(width-3);
    int readPhase=(s.readX-1)*increment,readShiftPhase=(shifted(s.readX)-1)*increment;
    int injectPhase=(s.injectX-1)*increment,injectShiftPhase=(shifted(s.injectX)-1)*increment;
    const auto feedback=multiplyQ15(c[2],c[2]),dry=saturate16(2*int(saturate16(32767-int(c[3]))));
    auto weight=[&](int& phase){const auto w=saturate16(2*int(sine[size_t(phase/1024)]));phase+=increment;if(phase>=4194304)phase=0;return w;};
    auto opposite=[&](int x,int y,int p){const bool wrap=x+half>=xLimit;x=shifted(x);y+=wrap?3:1;if(y>=yLimit){y-=yPeriod;p=!p;}return std::array<int,3>{{x,y,p}};};
    auto advance=[&](int& x,int& y,int& p){if(++x>=xLimit){x=1;y+=2;if(y>=yLimit){y=1;p=!p;}}};
    auto neighbours=[&](int p,int y,int x){auto sum=saturate16(int(at(p,y+1,x))+at(p,y-1,x));sum=saturate16(int(sum)+at(p,y,x-1));sum=saturate16(int(sum)+at(p,y,x+1));return multiplyQ15(sum,16384);};
    for(auto& input:audio){
        const auto injected=multiplyQ15(input,c[3]);const auto a=weight(injectPhase),b=weight(injectShiftPhase);
        const auto target=opposite(s.injectX,s.injectY,s.injectPlane);
        auto& first=at(s.injectPlane,s.injectY,s.injectX);first=saturate16(int(first)+multiplyQ15(injected,a));
        auto& second=at(target[2],target[1],target[0]);second=saturate16(int(second)+multiplyQ15(injected,b));
        advance(s.injectX,s.injectY,s.injectPlane);
        const auto other=opposite(s.readX,s.readY,s.readPlane);const auto wa=weight(readPhase),wb=weight(readShiftPhase);
        auto& r=at(s.readPlane,s.readY,s.readX);const auto n=neighbours(!s.readPlane,s.readY,s.readX);r=multiplyQ15(multiplyQ15(wa,feedback),saturate16(int(saturate16(-int(r)))+n));
        auto& t=at(other[2],other[1],other[0]);const auto m=neighbours(!other[2],other[1],other[0]);t=multiplyQ15(multiplyQ15(wb,feedback),saturate16(int(saturate16(-int(t)))+m));
        input=saturate16(int(saturate16(int(multiplyQ15(t,wb))+multiplyQ15(r,wa)))+multiplyQ15(input,dry));
        advance(s.readX,s.readY,s.readPlane);
        if(s.clearX>=xLimit||s.clearY>=yLimit)at(s.clearPlane,s.clearY,s.clearX)=0;
        if(++s.clearX>=259){s.clearX=1;if(++s.clearY>=259){s.clearY=1;s.clearPlane=!s.clearPlane;}}
    }
    const auto pos=size_t(s.slot)*4;auto meter=int16_t(meters[pos]&65535u);meter=std::max(meter,saturate16(int(audio[0])*2));meter=multiplyQ15(meter,32512);meters[pos]=(meters[pos]&0xffff0000u)|uint16_t(meter);
    return true;
}
}
