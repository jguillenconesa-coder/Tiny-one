// SPDX-License-Identifier: AGPL-3.0-only
// Original JUCE drawings inspired by OTTO-1's vector, parameter and transport-driven screens.
#include "ScreenAnimations.h"
#include "Engine.h"
namespace pocket {
namespace {
const std::array<juce::Colour,4> colors{juce::Colour(0xff39b6eb),juce::Colour(0xff60d2a2),juce::Colour(0xffe4eaf5),juce::Colour(0xfff08c47)};
void label(juce::Graphics& g,const juce::String& s,juce::Rectangle<int> r,float size,juce::Colour color) {g.setColour(color);g.setFont(juce::FontOptions(size));g.drawText(s,r,juce::Justification::centredLeft);}
void dot(juce::Graphics& g,float x,float y,float radius) {g.fillEllipse(x-radius,y-radius,radius*2,radius*2);}
}
void drawScreen(juce::Graphics& g,const ScreenFrame& f) {
    g.fillAll(juce::Colour(0xff070b10));
    const auto title=f.page>=5?juce::StringArray{"MASTER EQ","MASTER FX","MASTER OUT"}[std::clamp(f.page-5,0,2)]:f.page==4?(f.mode==3?"MIX":"TAPE"):f.page==1?(f.sampler246&&f.phaseEnvelope?"ADSR 246":f.drum246?"DYNAMICS":f.engine==11?"D-BOX II":"ENVELOPE"):f.page==2?"LFO":f.page==3?(f.cwo246?"CWO HYBRID":f.spring246?"SPRING 246":f.phone246?"PHONE 246":f.grid246?"GRID 246":f.delay246?"DELAY 246":f.nitro246?"NITRO HYBRID":f.filter246?"FILTER 246":f.punch246?"PUNCH 246":"ECHO / DRIVE"):juce::StringArray{f.digital246?"DIGITAL 246 CORE":"FM HYBRID / 4 OP",f.phase246?"PHASE 246 CORE":"PHASE","STRING 246",f.sampler246?"SAMPLER 246":"SAMPLER","DRUM",f.drum246?"SAMPLER 246":"SAMPLE KIT","DR. WAVE","CLUSTER","DNA HYBRID","VOLTAGE",f.dsynthSecondary?"D-SYNTH II":"D-SYNTH I","D-BOX I"}[std::clamp(f.engine,0,11)];
    label(g,title,{10,4,172,14},10,colors[2]);label(g,juce::String(f.voices)+" VOICES",{183,5,63,12},8,colors[0]);
    const float activity=std::clamp(f.level*6,0.f,1.f),phase=f.time*.7f;
    juce::Path curve;
    if(f.page>=5){for(int i=0;i<4;++i){g.setColour(colors[size_t(i)]);const float value=f.values[size_t(i)];const float normalized=f.page==5&&i<3?(value+12)/24:std::clamp(value,0.f,1.f);g.drawRoundedRectangle(18.f+i*58,27,40,59,3,1);g.fillRoundedRectangle(21.f+i*58,82-normalized*51,34,std::max(1.f,normalized*51),2);} }
    else if(f.drum246&&f.page==1){const float region=std::clamp(f.values[3]/65534.f,0.f,.5f);juce::Path shape;shape.startNewSubPath(16,88-f.values[0]/32767.f*52);shape.lineTo(16+region*218,88-f.values[1]/32767.f*52);shape.lineTo(234-region*218,88-f.values[1]/32767.f*52);shape.lineTo(234,88-f.values[2]/32767.f*52);g.setColour(colors[1]);g.strokePath(shape,juce::PathStrokeType(2));}
    else if(f.engine==11&&f.page<2){
        const int played=pocket::dbox246Pad(f.note);for(int i=0;i<24;++i){const float x=15.f+(i%8)*28,y=28.f+(i/8)*22;const bool hit=i==played&&f.voices>0;g.setColour(colors[size_t(i%4)].withAlpha(i==f.dboxPad?1.f:.35f));g.drawRoundedRectangle(x,y,23,18,2,i==f.dboxPad?2.f:1.f);if(hit){g.setColour(colors[size_t(i%4)].withAlpha(.35f+activity*.5f));g.fillRoundedRectangle(x+1,y+1,21,16,2);}label(g,juce::String(i+1),{int(x+3),int(y+2),18,12},8,colors[size_t(i%4)]);}
    }
    else if(f.page==4) {
        if(f.mode==3) {
            for(int i=0;i<4;++i){label(g,juce::String(i+1),{10,24+i*17,13,14},9,colors[size_t(i)]);g.setColour(colors[size_t(i)].withAlpha(.22f));g.fillRoundedRectangle(29,27.f+i*17,206,9,2);g.setColour(colors[size_t(i)]);g.fillRoundedRectangle(29,27.f+i*17,206*f.trackGains[size_t(i)]*(.35f+.65f*activity),9,2);}
        } else {
            const float progress=std::clamp(f.tapePosition/f.tapeLength,0.f,1.f);
            for(int reel=0;reel<2;++reel){const float x=58.f+reel*133,y=54,r=29;g.setColour(colors[2]);g.drawEllipse(x-r,y-r,r*2,r*2,1);g.drawEllipse(x-6,y-6,12,12,1);dot(g,x,y,2);
                g.setColour(colors[reel==0?0:2].withAlpha(.4f));const float wound=12+12*(reel==0?1-progress:progress);g.drawEllipse(x-wound,y-wound,wound*2,wound*2,2);
                g.setColour(colors[2]);for(int spoke=0;spoke<3;++spoke){const float angle=f.tapePosition*float(pi)+spoke*float(pi)*2/3;g.drawLine(x+std::cos(angle)*9,y+std::sin(angle)*9,x+std::cos(angle)*25,y+std::sin(angle)*25,1);}}
            g.setColour(colors[2].withAlpha(.6f));g.drawLine(87,54,162,54,1);g.drawLine(58,83,191,83,1);
            for(int track=0;track<4;++track){g.setColour(colors[size_t(track)].withAlpha(track==f.track?1.f:.4f));g.drawHorizontalLine(89+track*3,13,235);}
            g.setColour(colors[0]);g.drawVerticalLine(int(13+progress*222),88,101);
            label(g,juce::String(f.tapePosition,2)+"s",{100,67,57,13},9,colors[1]);
        }
    } else if(f.page==1) {
        const float a=13+f.attack*55,d=a+20+f.decay*40,s=211-f.release*32,end=237,y=84,top=28,sustain=y-f.sustain*56;
        curve.startNewSubPath(12,y);curve.lineTo(a,top);curve.lineTo(d,sustain);curve.lineTo(s,sustain);curve.lineTo(end,y);
        g.setColour(colors[0]);g.strokePath(curve,juce::PathStrokeType(1.4f));
        for(auto point:{juce::Point<float>{a,top},{d,sustain},{s,sustain}})dot(g,point.x,point.y,2);
        float x=f.stage==1?12+f.envelope*(a-12):f.stage==2?a+(1-f.envelope)/std::max(.001f,1-f.sustain)*(d-a):f.stage==4?s+(1-f.envelope/std::max(.001f,f.sustain))*(end-s):s;
        g.setColour(colors[3]);if(f.voices>0)dot(g,std::clamp(x,12.f,end),y-f.envelope*56,3.2f);
    } else if(f.page==2&&f.random246) {
        label(g,juce::String("RANDOM 246 / ")+(f.randomEnvelope?((f.engine==11||f.drum246)?"OFF / ADSR":"ADSR"):"INSTRUMENT"),{15,25,220,12},9,colors[0]);
        for(size_t i=0;i<4;++i){const float x=29.f+float(i)*56.f,y=61.f-std::clamp(f.random[i],-1.f,1.f)*20.f;g.setColour(colors[i]);g.drawLine(x,61.f,x,y,12.f);g.drawHorizontalLine(61,int(x-10),int(x+10));}
        label(g,juce::String(f.lfoRate,2)+" targets/s",{15,86,220,12},8,colors[2]);
    } else if(f.page==2&&f.tremolo246) {
        for(int i=0;i<225;++i){const double w=double(i)/224;const float v=float(f.lfoShape==1?2*w-1:f.lfoShape==2?2*w*w*w-1:f.lfoShape==3?(w<=.5?1:-1):f.lfoShape==4?(w<=.0625?1:-1):std::sin(2*pi*w));const float x=13.f+i,y=59-v*20;if(i==0)curve.startNewSubPath(x,y);else curve.lineTo(x,y);}
        g.setColour(colors[1]);g.strokePath(curve,juce::PathStrokeType(1.4f));g.setColour(colors[3]);g.drawVerticalLine(int(13+std::clamp(f.lfoPhase,0.f,1.f)*224),38,81);
        label(g,"TREMOLO 246 / PITCH + AMP",{15,25,220,12},9,colors[0]);
        label(g,juce::String(f.lfoRate,2)+" Hz / "+juce::StringArray{"SINE","RAMP","CURVE","SQUARE","PULSE"}[std::clamp(f.lfoShape,0,4)],{15,86,220,12},8,colors[2]);
    } else if(f.page==2&&f.value246) {
        const auto colour=colors[size_t(std::clamp(f.valueColour,0,3))];
        for(int i=0;i<225;++i){const float x=13.f+i,y=56-float(std::sin(2*pi*(double(i)/224-f.lfoPhase)))*23;if(i==0)curve.startNewSubPath(x,y);else curve.lineTo(x,y);}
        g.setColour(colour.withAlpha(f.valueDestination?1.f:.3f));g.strokePath(curve,juce::PathStrokeType(1.4f));
        g.fillEllipse(123,52-float(std::sin(2*pi*f.lfoPhase))*23,8,8);
        label(g,"VALUE 246 > "+juce::String(!f.valueDestination?"OFF":f.valueVoice?(f.valueEnvelope?"ADSR":(f.engine==11||f.drum246)?(f.drumLfoSecondary?"DRUM TARGETS II":"DRUM TARGETS I"):f.sampler246?(f.samplerSecondary?"FINE / GAIN":"START / LOOP"):f.engine==10?(f.dsynthSecondary?"INSTRUMENT II":"INSTRUMENT I"):"INSTRUMENT"):(f.cwo246?"CWO":f.spring246?"SPRING":f.phone246?"PHONE":f.grid246?"GRID":f.delay246?"DELAY":f.nitro246?"NITRO":f.filter246?"FILTER":"PUNCH")),{15,25,220,12},9,colour);
        label(g,juce::String(f.lfoRate,2)+" Hz  /  "+juce::StringArray{"BLUE","GREEN","WHITE","ORANGE"}[std::clamp(f.valueColour,0,3)],{15,86,220,12},8,colors[2]);
    } else if(f.page==2) {
        for(int i=0;i<225;++i){const double p=double(i)/224*2,w=p-std::floor(p);const float v=float(f.lfoShape==1?1-4*std::abs(w-.5):f.lfoShape==2?(w<.5?1:-1):std::sin(2*pi*p));const float x=13.f+i,y=56-v*25;if(i==0)curve.startNewSubPath(x,y);else curve.lineTo(x,y);}
        g.setColour(colors[1]);g.strokePath(curve,juce::PathStrokeType(1.4f));g.setColour(colors[3]);g.drawVerticalLine(int(13+f.lfoPhase*224),26,85);label(g,juce::String(f.lfoRate,2)+" Hz",{15,86,100,12},8,colors[2]);
    } else if(f.page==3&&f.cwo246) {
        for(int branch=0;branch<2;++branch){juce::Path path;for(int x=0;x<220;++x){const float y=57.f+float(std::sin(x*.11+f.time*2+branch*1.5707963))*(16+activity*7);if(x==0)path.startNewSubPath(15.f+x,y);else path.lineTo(15.f+x,y);}g.setColour(colors[size_t(branch)]);g.strokePath(path,juce::PathStrokeType(1.3f));}label(g,"SIDEBAND / OUTPUT",{15,84,220,12},9,colors[2]);
    } else if(f.page==3&&f.spring246) {
        for(int line=0;line<3;++line){juce::Path path;for(int x=0;x<220;++x){const float y=48.f+line*15+float(std::sin(x*.19+f.time*(1+line*.2)))*(4+f.level*8);if(x==0)path.startNewSubPath(15.f+x,y);else path.lineTo(15.f+x,y);}g.setColour(colors[size_t(line)]);g.strokePath(path,juce::PathStrokeType(1.2f));}
        label(g,"REVERB / OUTPUT",{15,87,220,12},9,colors[2]);
    } else if(f.page==3&&f.phone246) {
        for(int i=0;i<21;++i){const float x=20.f+i*10,h=5+activity*(8+15*std::abs(std::sin(phase+i*.7f)));g.setColour(colors[size_t(i%4)].withAlpha(.4f+activity*.6f));g.drawLine(x,54-h,x,54+h,2);}label(g,"TELEPHONIC / OUTPUT",{15,83,220,11},8,colors[2]);
    } else if(f.page==3&&f.grid246) {
        for(int y=0;y<5;++y)for(int x=0;x<10;++x){g.setColour(colors[size_t((x+y)%4)].withAlpha(.25f+activity*.65f));dot(g,20.f+x*23,33.f+y*10,1.5f+activity*std::abs(std::sin(float(x+y)+phase))*2);}label(g,"RESONANT PLATE / OUTPUT",{15,83,220,11},8,colors[2]);
    } else if(f.page==3&&f.delay246) {
        for(int i=0;i<7;++i){const float x=22.f+i*32;const float h=24*std::pow(.75f,float(i));g.setColour(colors[size_t(i%4)]);g.drawLine(x,63-h,x,63+h,2);if(i<6){g.setColour(colors[2].withAlpha(.4f));g.drawArrow({x+4,63,x+27,63},1,4,4);}}label(g,"SIZE / SPEED / FEEDBACK / SEND",{15,83,225,11},8,colors[2]);
    } else if(f.page==3&&f.nitro246) {
        for(int i=0;i<225;++i){const float u=float(i)/224,x=13.f+i;const float high=1/(1+std::exp(-(u-f.tone)*18)),low=1/(1+std::exp((u-f.color)*18));const float y=76-40*high*low;if(i==0)curve.startNewSubPath(x,y);else curve.lineTo(x,y);}g.setColour(colors[3]);g.strokePath(curve,juce::PathStrokeType(1.4f));label(g,"DUAL FILTER / OUTPUT BUS",{15,83,220,11},8,colors[2]);
    } else if(f.page==3&&f.filter246) {
        for(int i=0;i<225;++i){const float x=13.f+i,u=float(i)/224;const float cutoff=.1f+f.tone*.8f;const float y=36+35/(1+std::exp(-(u-cutoff)*16))-f.color*13*std::exp(-100*(u-cutoff)*(u-cutoff));if(i==0)curve.startNewSubPath(x,y);else curve.lineTo(x,y);}g.setColour(colors[0]);g.strokePath(curve,juce::PathStrokeType(1.4f));label(g,"FILTER / OUTPUT BUS",{15,83,210,11},8,colors[2]);
    } else if(f.page==3&&(f.cwo246||f.spring246||f.phone246||f.grid246||f.delay246||f.nitro246||f.filter246||f.punch246)) {
        const int stages=std::clamp(int(f.values[2])/1024,1,24);
        for(int i=0;i<24;++i){const auto x=15.f+i*9.2f;g.setColour(colors[size_t(i%4)].withAlpha(i<stages?.4f+activity*.6f:.12f));const auto h=8+activity*(10+10*std::sin(phase+i*.4f));g.fillRoundedRectangle(x,56-h,6,2*h,2);}
        label(g,juce::String(stages)+" STAGES",{15,83,100,10},8,colors[2]);
    } else if(f.page==3) {
        const std::array<float,3> x{32,126,220};for(int i=0;i<3;++i){g.setColour(colors[size_t(i)]);g.drawEllipse(x[size_t(i)]-12,44,24,24,1);dot(g,x[size_t(i)],56,2+activity*4);}
        g.setColour(colors[2].withAlpha(.4f));g.drawLine(44,56,114,56,1);g.drawLine(138,56,208,56,1);
        for(int i=0;i<8;++i){const float progress=std::fmod(f.time/std::max(.05f,f.delay)+i*.125f,1.f);g.setColour(colors[3].withAlpha((.1f+activity*.9f)*std::pow(f.feedback,float(i))));dot(g,45+progress*163,56,2.2f);}
        label(g,"DRIVE",{17,75,45,11},8,colors[0]);label(g,"ECHO",{111,75,45,11},8,colors[1]);label(g,"OUT",{210,75,35,11},8,colors[2]);
    } else if(f.engine==3) {
        for(size_t i=0;i<f.sample.size();++i){const float x=13+float(i)*224/127,amplitude=f.sample[i]*26;g.setColour(colors[2].withAlpha(.7f));g.drawLine(x,56-amplitude,x,56+amplitude,1);}
        if(f.sampler246){for(size_t i=0;i<4;++i){const int16_t fine=i==1?f.samplerControls[5]:i==2?f.samplerControls[6]:0;const float position=std::clamp(float(reference::sampler246Position(f.samplerControls[i],fine)/(4096.*std::max(1,f.samplerFrames))),0.f,1.f);g.setColour(colors[i].withAlpha((i==1||i==2)&&!f.samplerLoop?.35f:1.f));g.drawVerticalLine(int(13+position*224),i==1||i==2?35:27,86);}}
        else {const float start=f.tone*.9f,end=start+(1-start)*f.color;g.setColour(colors[0]);g.drawVerticalLine(int(13+start*224),27,86);g.setColour(colors[1]);g.drawVerticalLine(int(13+end*224),27,86);}
        if(f.voices>0){g.setColour(colors[3]);g.drawVerticalLine(int(13+f.samplePosition*224),24,89);}
    } else if(isDrumEngine(f.engine)) {
        const int pad=f.engine==5?Engine::kitSlot(f.note,f.kitPads):f.note%12==0||f.note%12==7?0:f.note%12==2||f.note%12==4?1:2;
        if(f.engine==5&&f.kitPads==24){for(int i=0;i<24;++i){const float x=15.f+(i%8)*28,y=29.f+(i/8)*23;const bool hit=i==pad&&f.voices>0;g.setColour(colors[size_t(i%4)].withAlpha(hit?.5f+activity*.5f:.3f));g.drawRoundedRectangle(x,y,23,19,2,1);if(hit)g.fillRoundedRectangle(x+1,y+1,21,17,2);label(g,juce::String(i+1),{int(x+2),int(y+3),19,12},8,hit?juce::Colour(0xff071015):colors[size_t(i%4)]);}}
        else for(int i=0;i<8;++i){const float x=15.f+i*28;const bool hit=i==pad&&f.voices>0;g.setColour(colors[size_t(i%4)].withAlpha(hit?.5f+activity*.5f:.23f));g.drawRoundedRectangle(x,33,23,43,4,1);if(hit){g.fillRoundedRectangle(x+2,57-activity*20,19,17+activity*20,2);g.drawEllipse(x+4-activity*2,39-activity*2,15+activity*4,15+activity*4,1);}label(g,juce::String(i+1),{int(x+8),79,16,12},8,colors[size_t(i%4)]);}
    } else if(f.engine==10) {
        for(int v=0;v<2;++v){juce::Path p;for(int i=0;i<220;++i){const float x=16.f+i,y=42.f+v*28+float(std::sin(i*(.055+v*.007)+phase))*(8+activity*7);if(i==0)p.startNewSubPath(x,y);else p.lineTo(x,y);}g.setColour(colors[size_t(v)]);g.strokePath(p,juce::PathStrokeType(v==(f.dsynthSecondary?1:0)?2.f:1.f));}label(g,f.dsynthSecondary?"OSC II / FOUR CONTROLS":"OSC I / FOUR CONTROLS",{16,84,220,10},7,colors[2]);
    } else if(f.engine==9) {
        for(int v=0;v<4;++v){juce::Path trace;for(int i=0;i<220;++i){const float x=17.f+i,y=34.f+v*13+float(std::sin(i*(.05+v*.006)+phase*(1+v*.1)))*(3+activity*4);if(i==0)trace.startNewSubPath(x,y);else trace.lineTo(x,y);}g.setColour(colors[size_t(v)]);g.strokePath(trace,juce::PathStrokeType(1.1f));}
        label(g,"FOUR OSC / XOR",{16,86,224,10},7,colors[2]);
    } else if(f.engine==8) {
        for(int i=0;i<32;++i){const float x=17.f+i*6.8f;const float a=48+float(std::sin(i*.43+phase))*17,b=48-float(std::sin(i*.43+phase))*17;g.setColour(colors[size_t(i%4)].withAlpha(.6f));g.drawLine(x,a,x,b,1);dot(g,x,a,1.7f);dot(g,x,b,1.7f);}
        label(g,"VIRTUAL DNA",{16,82,224,10},7,colors[2]);
    } else if(f.engine==2) {
        for(int string=0;string<4;++string){juce::Path wire;const float y=35.f+string*13;for(int i=0;i<180;++i){const float u=float(i)/179;const float x=25+u*200,v=y+std::sin(u*float(pi)*(1+string))*std::sin(phase*12+u*14)*activity*(3+f.color*5);if(i==0)wire.startNewSubPath(x,v);else wire.lineTo(x,v);}g.setColour(colors[size_t(string)]);g.strokePath(wire,juce::PathStrokeType(1));dot(g,25,y,2);dot(g,225,y,2);}
    } else if(f.cluster246) {
        const int count=std::clamp(int(f.values[0])/2048,1,8);
        for(int voice=0;voice<count;++voice){juce::Path wave;for(int i=0;i<225;++i){const float y=56+float(std::sin(2*pi*i/224+voice*.17+phase*.3))*(8+activity*14)+float(voice-count/2)*3;if(i==0)wave.startNewSubPath(13,y);else wave.lineTo(13+i,y);}g.setColour(colors[size_t(voice%4)].withAlpha(.6f));g.strokePath(wave,juce::PathStrokeType(1));}
        label(g,juce::String(count)+" OSC / VOICE",{15,83,160,10},8,colors[2]);
    } else if(f.drWave246) {
        for(int i=0;i<225;++i){const double p=double(i)/224;const float y=57-float(std::sin(2*pi*p)+.25*std::sin(2*pi*p*(2+std::floor(f.tone*3))))*(18+activity*6);if(i==0)curve.startNewSubPath(13,y);else curve.lineTo(13+i,y);}
        g.setColour(colors[0]);g.strokePath(curve,juce::PathStrokeType(1.5f));
    } else if(f.engine==1) {
        const double pivot=.08+f.tone*.84;
        for(int i=0;i<225;++i){const double p=double(i)/224,w=p<pivot?.5*p/pivot:.5+.5*(p-pivot)/(1-pivot);const float x=13.f+i,y=57-float(std::cos(2*pi*w))*(16+activity*9);if(i==0)curve.startNewSubPath(x,y);else curve.lineTo(x,y);}
        g.setColour(colors[1]);g.strokePath(curve,juce::PathStrokeType(1.5f));const float marker=std::fmod(phase*(.1f+f.color*.5f),1.f);g.setColour(colors[0]);g.drawVerticalLine(int(13+marker*224),27,86);
    } else if(f.digital246) {
        // Our feedback/XOR visualization, driven by note activity and raw controls.
        for(int column=0;column<24;++column){const auto bits=unsigned(int(32767*std::sin(column*.3f+phase*(.2f+f.color))))^unsigned(f.tone*32767);
            for(int bit=0;bit<8;++bit){g.setColour(colors[size_t(bit%4)].withAlpha(bits&(1u<<bit)?.35f+activity*.6f:.08f));g.fillRect(17+column*9,29+bit*7,7,5);}}
    } else {
        // Own topology diagram, showing actual four-operator connections.
        const int algorithm=std::clamp(f.fmAlgorithm,0,8);
        const auto x=[](int op){return 203.f-55.f*op;};
        auto connect=[&](int from,int to){const float start=x(from),end=x(to);juce::Path path;path.startNewSubPath(start,44);path.cubicTo(start,20,end,20,end,44);g.setColour(colors[size_t(from)].withAlpha(.7f));g.strokePath(path,juce::PathStrokeType(1));g.drawLine(end-2,40,end,44,1);g.drawLine(end+2,40,end,44,1);};
        switch(algorithm){
            case 0:connect(3,2);connect(2,1);connect(1,0);break;
            case 1:connect(3,1);connect(2,1);connect(1,0);break;
            case 2:connect(3,2);connect(2,0);connect(1,0);break;
            case 3:connect(3,2);connect(3,1);connect(2,0);connect(1,0);break;
            case 4:connect(3,2);connect(2,1);connect(2,0);break;
            case 5:connect(3,0);connect(2,0);connect(1,0);break;
            case 6:connect(3,1);connect(2,0);break;
            case 7:connect(3,2);connect(3,1);connect(3,0);break;
            case 8:break;
        }
        for(int op=0;op<4;++op){g.setColour(colors[size_t(op)]);g.drawRoundedRectangle(x(op)-12,44,24,23,3,1);label(g,juce::String::charToString(juce::juce_wchar('A'+op)),{int(x(op)-3),48,16,13},10,colors[size_t(op)]);if(op==0||((algorithm==4||algorithm==6)&&op==1)||(algorithm==7&&op<3)||algorithm==8){g.drawVerticalLine(int(x(op)),68,76);label(g,"OUT",{int(x(op)-9),77,23,10},7,colors[size_t(op)]);}}
        label(g,"ROUTE "+juce::String(algorithm+1)+" / 9",{91,89,125,10},8,colors[2]);
    }
    if(f.page!=4)for(int step=0;step<16;++step){g.setColour(step==f.step?colors[3]:juce::Colour(0xff27394a));dot(g,14.f+step*14.7f,95,step==f.step?2.f:1.f);}
    for(int i=0;i<4;++i){label(g,f.sampler246&&f.page==0?(f.samplerSecondary?juce::StringArray{"DIR","IN","OUT","GAIN"}:juce::StringArray{"START","IN","OUT","END"})[i]:f.labels[size_t(i)].substring(0,5),{10+i*59,102,58,9},7,colors[size_t(i)]);juce::String value;
        if(f.sampler246&&f.page==0){if(!f.samplerSecondary)value=juce::String(sampler246Seconds(f.samplerControls[size_t(i)])*1000,2)+" ms";else if(i==0)value=f.samplerControls[4]?"REV":"FWD";else if(i<3)value=juce::String(sampler246Seconds(0,f.samplerControls[size_t(i)+4])*1000000,2)+" us";else value=juce::String(std::pow(f.samplerControls[7]/16384.,2)*100,1)+" %";}
        else value=juce::String(f.values[size_t(i)],(f.page==2&&(i>=2||f.tremolo246||f.random246))||((f.page==1&&f.drum246)||(f.page==0&&f.drum246&&i==3)||(f.page<2&&f.engine==11)||(f.page==0&&(f.engine==0||f.engine==2||f.engine==8||f.engine==9||f.engine==10||f.cluster246||f.drWave246||f.phase246||f.digital246)))||(f.page==1&&f.phaseEnvelope)||(f.page==3&&(f.cwo246||f.spring246||f.phone246||f.grid246||f.delay246||f.nitro246||f.filter246||f.punch246))?0:2);label(g,value,{10+i*59,111,58,11},9,colors[size_t(i)]);}

}
}
