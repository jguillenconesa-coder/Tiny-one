// SPDX-License-Identifier: AGPL-3.0-only
#include "HostMidiOutput.h"
#include <iostream>
#include <stdexcept>
using M=pocket::HostMidiOutput;
static void check(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(){try{
 M m;m.begin(false);m.noteOn(M::keyboard,3,60,.5f,0);check(m.size()==0,"Default off");
 m.begin(true);m.noteOn(M::keyboard,3,60,.5f,0);m.noteOn(M::keyboard,3,60,.9f,1);m.noteOn(M::sequence,3,60,.9f,1);m.noteOff(M::keyboard,3,60,2);check(m.size()==1&&m.held()==1,"Overlapping source released early");m.noteOff(M::sequence,3,60,7);check(m.size()==2&&m.data()[1].sample==7&&m.data()[0].bytes[2]==64&&m.data()[1].bytes[2]==64&&!m.held(),"Release/velocity");
 m.begin(true);m.noteOn(M::keyboard,1,61,1,0);m.noteOn(M::keyboard,2,62,1,0);m.releaseKeyboardChannel(1,4);check(m.held()==1,"Channel release");m.begin(false);check(m.size()==1&&!m.held(),"Disable did not release");
 std::array<bool,M::noteCount> wire{};auto consume=[&](){int last=-1;for(size_t i=0;i<m.size();++i){auto e=m.data()[i];check(e.sample>=last,"Unsorted event offsets");last=e.sample;const size_t key=(e.bytes[0]&15)*128+e.bytes[1];if((e.bytes[0]&0xf0)==0x90){check(!wire[key],"Duplicate on");wire[key]=true;}else{check(wire[key],"Orphan off");wire[key]=false;}}};
 for(int block=0;block<2;++block){m.begin(true);for(int n=0;n<1024;++n){const int k=block*1024+n;m.noteOn(M::keyboard,k/128+1,k%128,.8f,0);}consume();}check(m.held()==2048,"Full held ledger");
 m.begin(true);m.flush(0);for(int n=0;n<1024;++n)m.noteOn(M::keyboard,n/128+1,n%128,1,1);m.noteOn(M::keyboard,16,127,1,2);check(m.overflowed()&&m.size()==M::capacity&&!m.held(),"Emergency reserve failed");consume();for(bool b:wire)check(!b,"Overflow left hanging note");
 std::cout<<"PASS: ownership, channel isolation, disable, ordered offsets and 2048-note/4096-event emergency cleanup\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
