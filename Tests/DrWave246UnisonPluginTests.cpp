// SPDX-License-Identifier: AGPL-3.0-only
#include "PluginProcessor.h"
#include <iostream>
#include <memory>
#include <vector>
#include <stdexcept>
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
static void unison(PocketProcessor& p,bool on){p.phaseParameters().getParameter("drWave246Unison")->setValueNotifyingHost(on?1.f:0.f);}
static std::vector<float> render(double sr,int block,const juce::File& sounds,int mode){
 auto p=std::make_unique<PocketProcessor>(sounds);p->applyDrWave246Init();unison(*p,true);auto* choice=p->phaseParameters().getParameter("hybridVoiceMode");choice->setValueNotifyingHost(choice->convertTo0to1(float(mode)));p->phaseParameters().getParameter("drWave246Release")->setValueNotifyingHost(1);p->prepareToPlay(sr,block);std::vector<float> out;
 for(int start=0;start<12000;start+=block){const int frames=std::min(block,12000-start);juce::AudioBuffer<float> audio(2,frames);audio.clear();juce::MidiBuffer midi;
  auto at=[&](int frame,const juce::MidiMessage& m){if(frame>=start&&frame<start+frames)midi.addEvent(m,frame-start);};
  at(0,juce::MidiMessage::noteOn(1,60,.8f));at(333,juce::MidiMessage::noteOn(2,72,.6f));at(335,juce::MidiMessage::noteOff(2,72));at(600,juce::MidiMessage::controllerEvent(1,64,127));at(650,juce::MidiMessage::noteOff(1,60));at(900,juce::MidiMessage::controllerEvent(1,64,0));at(1300,juce::MidiMessage::allSoundOff(1));
  p->processBlock(audio,midi);for(int i=0;i<frames;++i){const auto v=audio.getSample(0,i);check(std::isfinite(v)&&std::abs(v)<1,"Unstable Unison processor output");if(start+i>=1300)check(v==0,"Panic revived Unison");out.push_back(v);}
 }check(p->visualVoices.load()==0,"Unison processor left notes");return out;
}
int main(int argc,char** argv){try{
 juce::ScopedJuceInitialiser_GUI init;const auto sounds=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("pocket-unison-test",{},false);sounds.createDirectory();struct Cleanup{juce::File f;~Cleanup(){f.deleteRecursively();}} cleanup{sounds};
 auto p=std::make_unique<PocketProcessor>(sounds);p->applyDrWave246Init();check(p->phaseParameters().getRawParameterValue("drWave246Unison")->load()==0,"Unison not default off");unison(*p,true);juce::MemoryBlock saved;p->getStateInformation(saved);unison(*p,false);p->setStateInformation(saved.getData(),int(saved.getSize()));check(p->phaseParameters().getRawParameterValue("drWave246Unison")->load()==1,"Unison state lost");
 juce::String error;check(p->assignSoundSlot(false,0)&&p->copySound(error),"Unison slot/clipboard save");p->applyPreset(0);check(p->recallSoundSlot(false,0,error)&&p->phaseParameters().getRawParameterValue("drWave246Unison")->load()==1,"Unison slot lost");p->applyPreset(0);check(p->pasteSound(error)&&p->phaseParameters().getRawParameterValue("drWave246Unison")->load()==1,"Unison clipboard lost");
 auto tree=juce::ValueTree::fromXml(*juce::AudioProcessor::getXmlFromBinary(saved.getData(),int(saved.getSize())));auto phase=tree.getChildWithName("Phase246");phase.removeChild(phase.getChildWithProperty("id","drWave246Unison"),nullptr);juce::MemoryBlock legacy;juce::AudioProcessor::copyXmlToBinary(*tree.createXml(),legacy);p->setStateInformation(legacy.getData(),int(legacy.getSize()));check(p->phaseParameters().getRawParameterValue("drWave246Unison")->load()==0,"Legacy state inherited Unison");
 for(double sr:{44100.,48000.,96000.})for(int mode:{0,1,2})check(render(sr,64,sounds,mode)==render(sr,257,sounds,mode),"Unison depends on callback partition");
 for(double sr:{44100.,48000.,96000.})for(int frames:{64,257}){
  auto synth=std::make_unique<PocketProcessor>(sounds);synth->applyDrWave246Init();unison(*synth,true);synth->prepareToPlay(sr,frames);juce::AudioBuffer<float> a(2,frames);juce::MidiBuffer m;m.ensureSize(65536);a.clear();synth->processBlock(a,m);heapCalls=0;
  for(int i=0;i<256;++i){a.clear();m.clear();if(i%4==0)m.addEvent(juce::MidiMessage::noteOn(1+i%3,48+i%36,.8f),0);if(i%4==2)m.addEvent(juce::MidiMessage::allNotesOff(1+i%3),0);if(i%64==48)synth->requestPanic();if(i%64==32)unison(*synth,false);if(i%64==40)unison(*synth,true);tracking=true;synth->processBlock(a,m);tracking=false;}check(heapCalls==0,"Heap in prepared Unison callbacks");
 }
 p->applyDrWave246Init();p->setScreenPage(1);std::unique_ptr<juce::AudioProcessorEditor> editor(p->createEditor());editor->setVisible(true);juce::TextButton* button=nullptr;for(auto* child:editor->getChildren())if(child->getComponentID()=="drwave-unison")button=dynamic_cast<juce::TextButton*>(child);check(button!=nullptr,"Unison control missing");
 if(argc>1){editor->addToDesktop(juce::ComponentPeer::windowHasTitleBar);editor->toFront(true);}
 bool selected=false,captured=argc==1;juce::Timer::callAfterDelay(150,[button]{button->triggerClick();});juce::Timer::callAfterDelay(500,[&]{selected=button->isVisible()&&button->getToggleState()&&p->phaseParameters().getRawParameterValue("drWave246Unison")->load()>.5f;if(argc>1){juce::FileOutputStream stream(juce::File(juce::String::fromUTF8(argv[1])));juce::PNGImageFormat png;captured=png.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds()),stream);}juce::MessageManager::getInstance()->stopDispatchLoop();});juce::MessageManager::getInstance()->runDispatchLoop();check(selected&&captured,"Unison UI did not toggle parameter");
 std::cout<<"PASS: Unison automation/state/legacy/slots/clipboard, nine 64-257 processor pairs, UI toggle and six prepared workloads /1536 callbacks without tracked heap\n";
}catch(const std::exception& e){tracking=false;std::cerr<<e.what()<<'\n';return 1;}}
