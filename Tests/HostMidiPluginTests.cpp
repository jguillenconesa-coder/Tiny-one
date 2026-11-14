// SPDX-License-Identifier: AGPL-3.0-only
#include "PluginProcessor.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <new>
namespace {thread_local bool tracking=false;thread_local size_t heapCalls=0;}
void* operator new(size_t n){if(tracking)++heapCalls;if(auto* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{if(tracking&&p)++heapCalls;std::free(p);}
void operator delete[](void* p)noexcept{::operator delete(p);}
void operator delete(void* p,size_t)noexcept{::operator delete(p);}
void operator delete[](void* p,size_t)noexcept{::operator delete(p);}
#if POCKET_TRACK_C_HEAP
extern "C" {
void* __real_malloc(size_t);void* __real_calloc(size_t,size_t);void* __real_realloc(void*,size_t);void __real_free(void*);
void* __wrap_malloc(size_t n){if(tracking)++heapCalls;return __real_malloc(n);}
void* __wrap_calloc(size_t a,size_t b){if(tracking)++heapCalls;return __real_calloc(a,b);}
void* __wrap_realloc(void* p,size_t n){if(tracking)++heapCalls;return __real_realloc(p,n);}
void __wrap_free(void* p){if(tracking&&p)++heapCalls;__real_free(p);}
}
#endif
static void check(bool b,const char* why){if(!b)throw std::runtime_error(why);}
static void parameter(PocketProcessor& p,const char* id,float value){auto* a=p.state.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(value));}
struct Event {int sample;int status,note,velocity;bool operator==(const Event& e)const{return sample==e.sample&&status==e.status&&note==e.note&&velocity==e.velocity;}};
static std::vector<Event> events(const juce::MidiBuffer& m,int offset=0){std::vector<Event> e;for(auto x:m){check(x.numBytes==3,"Output size");e.push_back({offset+x.samplePosition,x.data[0],x.data[1],x.data[2]});}return e;}
static std::vector<Event> sequence(int block,bool enabled){auto p=std::make_unique<PocketProcessor>();parameter(*p,"sequencer",1);parameter(*p,"tempo",120);parameter(*p,"seqRate",2);parameter(*p,"seqGate",.5f);for(auto& n:p->steps)n.store(60);p->hostMidiOutputParameter().setValueNotifyingHost(enabled?1.f:0.f);p->prepareToPlay(48000,block);std::vector<Event> result;for(int start=0;start<48000;start+=block){juce::AudioBuffer<float> audio(2,std::min(block,48000-start));audio.clear();juce::MidiBuffer midi;midi.ensureSize(65536);p->processBlock(audio,midi);auto next=events(midi,start);result.insert(result.end(),next.begin(),next.end());}return result;}
struct Transport:juce::AudioPlayHead{bool playing=true;juce::Optional<PositionInfo> getPosition()const override{PositionInfo p;p.setBpm(120.);p.setPpqPosition(0.);p.setIsPlaying(playing);return p;}};
int main(int argc,char** argv){try{
 juce::ScopedJuceInitialiser_GUI init;auto p=std::make_unique<PocketProcessor>();check(p->producesMidi()&&!p->hostMidiOutputParameter().get(),"Capability/default");p->prepareToPlay(48000,64);juce::AudioBuffer<float> audio(2,64);juce::MidiBuffer midi;midi.ensureSize(65536);
 p->hostMidiOutputParameter().setValueNotifyingHost(1);midi.addEvent(juce::MidiMessage::noteOn(4,60,.8f),17);p->processBlock(audio,midi);check(midi.isEmpty(),"Host input echoed");
 p->keyboard.noteOn(3,62,.5f);p->processBlock(audio,midi);auto e=events(midi);check(e.size()==1&&e[0]==Event{0,0x92,62,64},"Local keyboard output");
 midi.clear();p->hostMidiOutputParameter().setValueNotifyingHost(0);p->processBlock(audio,midi);e=events(midi);check(e.size()==1&&e[0]==Event{0,0x82,62,64},"Disable cleanup");
 p->hostMidiOutputParameter().setValueNotifyingHost(1);p->keyboard.noteOn(2,65,.8f);midi.clear();p->processBlock(audio,midi);midi.clear();p->requestPanic();p->processBlock(audio,midi);e=events(midi);check(e.size()==1&&e[0].status==0x81,"Panic cleanup");
 p->keyboard.noteOn(2,67,.8f);midi.clear();p->processBlock(audio,midi);juce::MemoryBlock saved;p->getStateInformation(saved);midi.clear();p->setStateInformation(saved.getData(),int(saved.getSize()));p->processBlock(audio,midi);check(p->hostMidiOutputParameter().get()&&midi.getNumEvents()==1&&events(midi)[0].status==0x81,"State restore/cleanup");
 auto tree=juce::ValueTree::fromXml(*juce::AudioProcessor::getXmlFromBinary(saved.getData(),int(saved.getSize())));tree.removeProperty("hostMidiOutput",nullptr);juce::MemoryBlock legacy;juce::AudioProcessor::copyXmlToBinary(*tree.createXml(),legacy);p->setStateInformation(legacy.getData(),int(legacy.getSize()));check(!p->hostMidiOutputParameter().get(),"Missing flag inherited output");
 p->hostMidiOutputParameter().setValueNotifyingHost(1);p->keyboard.noteOn(1,70,.8f);juce::AudioBuffer<float> empty(2,0);midi.clear();p->processBlock(empty,midi);check(midi.isEmpty(),"Zero-block timestamp");p->processBlock(audio,midi);check(midi.getNumEvents()==1&&events(midi)[0].note==70,"Zero block lost queued UI event");
 auto a=sequence(64,true),b=sequence(257,true);check(!a.empty()&&a==b&&sequence(64,false).empty(),"Sequence partition/default");check(a.front()==Event{0,0x90,60,102},"Sequence onset/velocity");check(a[1].status==0x80&&a[1].sample>=2999&&a[1].sample<=3001,"Sequence gate timestamp");
 auto t=std::make_unique<PocketProcessor>();Transport host;t->setPlayHead(&host);parameter(*t,"sequencer",1);t->hostMidiOutputParameter().setValueNotifyingHost(1);t->prepareToPlay(48000,64);midi.clear();t->processBlock(audio,midi);check(midi.getNumEvents()==1,"Host sequencer start");host.playing=false;midi.clear();t->processBlock(audio,midi);check(midi.getNumEvents()==1&&events(midi)[0].status==0x80,"Host stop left note");
 // A lost UI key-up must release already published notes instead of hanging them.
 for(int k=0;k<1024;++k)p->keyboard.noteOn(9+k/128,k%128,.8f);
 midi.clear();p->processBlock(audio,midi);check(midi.getNumEvents()==1&&events(midi)[0].status==0x80,"UI overflow cleanup");
 // Outgoing MIDI must not alter the synth's rendered samples.
 auto renderAudio=[](bool output){auto synth=std::make_unique<PocketProcessor>();synth->applyPreset(0);synth->hostMidiOutputParameter().setValueNotifyingHost(output?1.f:0.f);synth->prepareToPlay(48000,256);std::vector<float> result;for(int b=0;b<8;++b){juce::AudioBuffer<float> a(2,256);a.clear();juce::MidiBuffer m;if(b==0)m.addEvent(juce::MidiMessage::noteOn(1,60,.8f),7);if(b==6)m.addEvent(juce::MidiMessage::noteOff(1,60),1);synth->processBlock(a,m);result.insert(result.end(),a.getReadPointer(0),a.getReadPointer(0)+256);}return result;};
 auto dry=renderAudio(false),sending=renderAudio(true);check(dry==sending&&std::any_of(dry.begin(),dry.end(),[](float v){return v!=0;}),"MIDI output changed audio");
 size_t workloads=0;
 for(double sr:{44100.,48000.,96000.})for(int frames:{64,257}){
  auto synth=std::make_unique<PocketProcessor>();synth->applyPreset(0);parameter(*synth,"sequencer",1);parameter(*synth,"tempo",300);synth->hostMidiOutputParameter().setValueNotifyingHost(1);synth->prepareToPlay(sr,frames);juce::AudioBuffer<float> a(2,frames);juce::MidiBuffer m;m.ensureSize(pocket::HostMidiOutput::capacity*16);synth->processBlock(a,m);
  heapCalls=0;
  for(int i=0;i<256;++i){m.clear();a.clear();if(i%4==0)synth->keyboard.noteOn(2,65,.8f);if(i%4==2)synth->keyboard.noteOff(2,65,0);if(i%64==48)synth->requestPanic();if(i%32==16)synth->hostMidiOutputParameter().setValueNotifyingHost(0);if(i%32==24)synth->hostMidiOutputParameter().setValueNotifyingHost(1);tracking=true;synth->processBlock(a,m);tracking=false;}
  check(heapCalls==0,"Heap activity in prepared MIDI callback");++workloads;
 }
 check(workloads==6,"Realtime workload coverage");
 if(argc>1){
  auto visual=std::make_unique<PocketProcessor>();std::unique_ptr<juce::AudioProcessorEditor> ui(visual->createEditor());ui->addToDesktop(juce::ComponentPeer::windowHasTitleBar);ui->setVisible(true);ui->toFront(true);juce::TextButton* com=nullptr;for(auto* child:ui->getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()=="COM")com=button;check(com!=nullptr,"COM control missing");bool captured=false,selected=false;juce::Timer::callAfterDelay(150,[com]{com->onClick();});
  juce::Timer::callAfterDelay(350,[&]{auto* menu=juce::Component::getCurrentlyModalComponent();if(!menu)return;auto image=menu->createComponentSnapshot(menu->getLocalBounds());juce::PNGImageFormat png;juce::FileOutputStream stream(juce::File(juce::String::fromUTF8(argv[1])));captured=png.writeImageToStream(image,stream);menu->keyPressed(juce::KeyPress(juce::KeyPress::downKey));menu->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));});
  juce::Timer::callAfterDelay(650,[&]{selected=visual->hostMidiOutputParameter().get();juce::MessageManager::getInstance()->stopDispatchLoop();});juce::MessageManager::getInstance()->runDispatchLoop();check(captured&&selected,"COM menu did not enable output");
 }
 std::cout<<"PASS: host MIDI keys, no input echo, sequencer offsets/64-257 partition, host stop, panic, disable, zero blocks, audio equality, state migration and six prepared callback workloads without tracked heap activity\n";
}catch(const std::exception& e){tracking=false;std::cerr<<e.what()<<'\n';return 1;}}
