// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Dbox246.h"
#include "Phase246.h"
namespace pocket {
inline constexpr int dbox246Pads=24;
inline int dbox246Pad(int note) noexcept {return (std::clamp(note,0,127)+12)%24;}
// Authored init, note/pad mapping and logarithmic pitch adapter; no factory patches.
inline std::array<int16_t,8> dbox246Default(int pad) noexcept {
    return {{int16_t(4096+(pad%8)*512),int16_t((pad%3)*4000),int16_t(1000+(pad%4)*1000),int16_t(3000+(pad%8)*4096),int16_t(12000+(pad%4)*4000),int16_t((pad%5)*2000),int16_t(1500+(pad%3)*1000),int16_t(16000+(pad%4)*3000)}};
}
inline std::array<std::array<int16_t,8>,dbox246Pads> dbox246Defaults() noexcept {std::array<std::array<int16_t,8>,dbox246Pads> c{};for(int i=0;i<dbox246Pads;++i)c[size_t(i)]=dbox246Default(i);return c;}
struct Dbox246Voice {
    reference::Dbox246State state;Phase246Voice output;std::array<int16_t,128> block{},gain{},second{};std::array<uint32_t,128> derived{};
    unsigned cursor=128;bool initialized=false,releaseSent=false,finished=false,silentBlock=false;
    bool done() const noexcept {return finished;}
    float level() const noexcept {return float(std::clamp(double(std::max(state.envelope[0],state.envelope[1]))/2147483648.,0.,1.));}
    float source(double bend,const std::array<int16_t,8>& controls,reference::Dbox246Context& context,int slot,const Phase246Config& rate,bool released) noexcept {
        if(!initialized){state={};state.construct();state.bind(slot);context.reset(state);initialized=true;}
        if(released&&!releaseSent){state.release();releaseSent=true;}
        if(cursor==128){auto bounded=controls;for(auto& x:bounded)x=int16_t(std::clamp(int(x),0,32767));
            // The original caller's tuning producer remains outside the port. A's
            // callback divides incoming pitch by 16, B uses its squared ratio.
            const auto frequency=40.*std::exp2(double(bounded[0])/4096.)*16.*bend;
            std::array<uint32_t,128> pitch{};pitch.fill(uint32_t(std::clamp(frequency,0.,Phase246Config::internalRate*.49)/Phase246Config::internalRate*4294967296.+.5));output.tremolo.pitchBlock(pitch,rate.table);
            // Authored half-scale gain leaves headroom for additive Tremolo.
            gain.fill(16384);for(auto& v:gain)v=output.tremolo.amplitude(v,rate.table);
            reference::dbox246Block(state,context,bounded,pitch,gain,rate.table,block,derived,second);
            silentBlock=state.active()==0&&std::all_of(block.begin(),block.end(),[](int16_t v){return v>=-4&&v<=4;});cursor=0;
        }
        double sample=block[cursor++]/32768.;if(cursor==128&&silentBlock)finished=true;
        for(size_t i=0;i<2;++i){const auto& c=rate.filters[i];auto& m=output.memory[i];const double out=c.b0*sample+m[0];m[0]=c.b1*sample-c.a1*out+m[1];m[1]=c.b2*sample-c.a2*out;sample=out;}return float(sample);
    }
    float next(double bend,const std::array<int16_t,8>& controls,reference::Dbox246Context& context,int slot,const Phase246Config& rate,bool released=false) noexcept {
        if(!output.started){output.first=source(bend,controls,context,slot,rate,released);output.second=source(bend,controls,context,slot,rate,released);output.started=true;}
        const auto value=output.first+float(output.position)*(output.second-output.first);output.position+=rate.step;
        while(output.position>=1){output.position-=1;output.first=output.second;output.second=source(bend,controls,context,slot,rate,released);}return value;
    }
};
struct Dbox246Bank {
    struct Hit {Dbox246Voice voice;bool active=false,keyDown=false,released=false;int note=0,channel=1;float velocity=0;uint64_t age=0;float last=0,stolen=0;unsigned fade=0;};
    std::array<Hit,12> hits{};reference::Dbox246Context context{};uint64_t age=0;
    std::array<bool,17> pedals{};std::array<std::array<float,8>,dbox246Pads> smoothed{};bool smoothStarted=false;
    void reset() noexcept {*this={};context.initialize();}
    void noteOn(int note,float velocity,int channel,double hostRate) noexcept {
        Hit* hit=nullptr;for(auto& h:hits)if(!h.active){hit=&h;break;}if(!hit)hit=&*std::min_element(hits.begin(),hits.end(),[](const auto& a,const auto& b){return a.age<b.age;});
        const auto tail=hit->active?hit->last:0.f;*hit={};hit->active=hit->keyDown=true;hit->note=note;hit->channel=channel;hit->velocity=std::clamp(velocity,0.f,1.f);hit->age=++age;hit->stolen=tail;hit->fade=tail!=0?unsigned(hostRate*.003):0;
    }
    void noteOff(int note,int channel) noexcept {for(auto& h:hits)if(h.active&&h.note==note&&h.channel==channel){h.keyDown=false;if(!pedals[size_t(channel-1)])h.released=true;}}
    void pedal(int channel,bool down) noexcept {pedals[size_t(channel-1)]=down;if(!down)for(auto& h:hits)if(h.channel==channel&&!h.keyDown)h.released=true;}
    void allNotesOff(int channel) noexcept {for(auto& h:hits)if(channel==0||channel==h.channel){h.keyDown=false;h.released=true;}if(channel==0)pedals.fill(false);else pedals[size_t(channel-1)]=false;}
    float next(const std::array<std::array<int16_t,8>,dbox246Pads>& controls,const std::array<float,17>& bends,float smoothing,const Phase246Config& rate,bool tremolo,const std::array<int16_t,4>& tremoloControls,int shape,uint32_t tempo) noexcept {
        std::array<std::array<int16_t,8>,dbox246Pads> raw{};
        for(size_t p=0;p<raw.size();++p)for(size_t i=0;i<8;++i){auto& value=smoothed[p][i];if(!smoothStarted)value=controls[p][i];else value+=smoothing*(controls[p][i]-value);raw[p][i]=int16_t(std::lround(std::clamp(value,0.f,32767.f)));}smoothStarted=true;
        float sum=0;for(size_t i=0;i<hits.size();++i){auto& h=hits[i];if(!h.active)continue;h.voice.output.tremolo.configure(tremolo,tremoloControls,shape,tempo,true);
            auto v=h.voice.next(bends[size_t(h.channel-1)],raw[size_t(dbox246Pad(h.note))],context,int(i),rate,h.released)*h.velocity*.16f;
            if(h.fade){const auto weight=float(h.fade)/float(std::max(1u,unsigned((rate.internalRate/rate.step)*.003)));v=v*(1-weight)+h.stolen*weight;--h.fade;}h.last=v;sum+=v;if(h.voice.done())h.active=false;
        }return sum;
    }
};
}
