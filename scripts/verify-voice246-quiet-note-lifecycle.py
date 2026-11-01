#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Explicit constructor/on/off/ADSR sequences with state and list retained.
No pending-field consumer, original event clock or pitch-buffer producer.
"""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=1536
WATCHES=['0x01a84690','0x019e2ebe','0x019e2f2c','0x019e2e00','0x019e0d18','0x01919aa8','0x01a857d0','0x01a85890','0x01a8589e','0xffa00000,0xffafffff']
# Address, bytes, retain across each of the twelve 128-event streams.
REGIONS=[(0x01dfe72c,2576,True),(0xff8000d0,664,True),(0xffb00000,224,True),
 (0xff801034,776,True),(0x01b60ef0,128,False),(0xff802bc8,8,True),
 (0x01b60534,8,True),(0x01b6053c,8,False),(0xff8016d4,8,False),
 (0x01aaac0c,8,False),(0x01aaad14,32,False),(0x027fd9dc,12,False),(0x018c2180,12,False),(0x01de4c3c,12,False),(0x01dfe4d8,48,True),(0x01aaad68,32,False),(0x01aaac40,12,False),(0x01aafd5c,136,True),(0x01aadb1c,40,True),(0x018b600c,4,True),(0x01dfe038,12,True)]
IMAGE=sum(n for _,n,_ in REGIONS);RECORD=40+IMAGE+272;RESULT=IMAGE*2+380
OFF=[]
for _,n,_ in REGIONS:OFF.append(sum(x[1] for x in REGIONS[:len(OFF)]))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs(cold_pool=False,instrument=False):
 if instrument:return instrument_inputs()
 spec=importlib.util.spec_from_file_location('life',ROOT/'scripts/verify-drwave246-note-lifecycle.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 old=m.inputs();data=bytearray()
 for c in range(CASES):
  row=old[c*m.RECORD:(c+1)*m.RECORD];header=bytearray(row[:40]);image=bytearray(row[40:40+m.IMAGE])+bytearray(struct.pack('<3I',0xa55a5aa5,0,0x12345678))
  if c%16==0:struct.pack_into('<I',header,20,0)
  elif c%16==2:struct.pack_into('<I',header,20,0xffffffff)
  image[4]=1 if (c//128)%4==3 and struct.unpack_from('<I',header,12)[0]==0 else 0 # Off requires gate zero.
  image[4572:4700]=bytes([0,1,128,255][(j+c//128)%4] for j in range(128))
  if cold_pool:struct.pack_into('<I',image,4384,[0,0x10000,0xffffffff,0][c//128%4])
  data+=header+image+row[-272:]
 return data

def instrument_inputs():
 """20 fixed-selector plus four mixed-selector streams; no activity overrides."""
 old=inputs(True);data=bytearray()
 for c in range(3072):
  stream,step=divmod(c,128);profile=stream%4;cycle=step//8;event=step%8
  mode=stream//4 if stream<20 else [0,4,3,2,4,0,3,4][event]
  source=((max(1,min(mode,3))-1)*512+profile*128+step)*RECORD
  row=old[source:source+RECORD];header=bytearray(row[:40]);image=bytearray(row[40:40+IMAGE])
  reset=step==0;raw=profile>=2
  action=0 if event in (0,1,2,4) else 1
  struct.pack_into('<I',header,0,reset);struct.pack_into('<I',header,12,action|(2 if raw else 0))
  a=(profile*32+cycle*2)%128;b=(a+37)%128;d=(a+79)%128
  target=[a,a,b,a,d,d,b,(a+113)%128][event]
  struct.pack_into('<I',header,16,target if raw else (target-53)&0xffffffff)
  # Raw inputs carry nonzero pitch from the start; constructor inputs keep the
  # constructor's metadata/pitch result unchanged. No post-constructor override.
  if raw:
   header[24]=[0,1,128,255][cycle%4]
   struct.pack_into('<I',header,28,[1,0x7fffffff,0x80000000,0xffffffff,0x40000000,0x12345678][cycle%6])
  image[4]=1 if action==0 and step%32==0 and profile%2 else 0
  for i in range(8):struct.pack_into('<h',image,4240+i*16+8,(mode<<12)|(c%4096))
  struct.pack_into('<I',image,4384,[0,65536,0,0xffffffff][profile])
  struct.pack_into('<I',image,2568,0)
  image[3504:3510]=bytes([0]*6 if profile==0 and stream<20 else [1,128,255,0,0,0] if profile==1 else [1,0,0,0,0,0] if profile==2 and stream<20 else [1,128,255,1,128,255])
  # Explicit valid ages for warm startup. Subsequent state is never reloaded.
  for v in range(6):struct.pack_into('<I',image,2580+v*24+4,0xffffffff if profile==2 else 100+v)
  selected=struct.unpack_from('<I',image,4392)[0]
  # A few rejected Core starts, followed by accepted starts in the same stream.
  struct.pack_into('<I',image,4400,(selected+1)%8 if cycle%5==0 else selected)
  data+=header+image+row[-272:]
 return data

def caller(cold_pool=False):
 def copy(label,src,dst,n):
  return f'{src}\n{dst}\nP3 = {n//4};\nLSETUP({label},{label}_end) LC0 = P3;\n{label}: R0 = [P1++];\n{label}_end: [P2++] = R0;\n'
 load='';save=''
 for i,(addr,n,retain) in enumerate(REGIONS):
  retain=retain or (cold_pool and i==7)
  if retain:load+=f'R0 = [P4];\nCC = R0 == 0;\nIF CC JUMP skip_{i};\n'
  load+=copy('load_'+str(i),f'P0 = {40+OFF[i]};\nP1 = P4 + P0;',f'imm32 P2, {addr}',n)
  if retain:load+=f'skip_{i}:\n'
  save+=copy('save_'+str(i),f'imm32 P1, {addr}','P2 = P5;',n)+'P5 = P2;\n'
 return load,save

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for k in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+k,type=Path,required=True)
 p.add_argument('--record',action='store_true');p.add_argument('--cold-pool',action='store_true');p.add_argument('--instrument',action='store_true');a=p.parse_args();out=a.output.resolve()
 cases=3072 if a.instrument else CASES
 prefix='voice246-instrument-lifecycle' if a.instrument else 'voice246-cold-note-lifecycle' if a.cold_pool else 'voice246-quiet-note-lifecycle'
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a NEW private directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Only inventoried rev246 supported')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,addr,n,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=addr and addr+n<=base+len(image) and not flags&0x1000:image[addr-base:addr-base+n]=(struct.pack('<I',arg)*((n+3)//4))[:n] if flags&0x100 else raw[cursor+16:cursor+16+n]
  cursor+=16+(0 if flags&0x100 else n)
 out.mkdir(mode=0o700,parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs(a.cold_pool,a.instrument))
 load,save=caller(a.cold_pool or a.instrument);templates=ROOT/'research/blackfin'
 conversion=''.join(f'imm32 P0, {0x018c13c4+i*4}\nR0 = [P0];\nimm32 P0, 0x01aa6fd0\nCALL (P0);\nimm32 P0, {0x0106b3cc+i*4}\n[P0] = R0;\n' for i in range(256))
 template=templates/('voice-instrument-lifecycle.s.in' if a.instrument else 'voice-quiet-note-lifecycle.s.in')
 source=template.read_text()
 for k,v in {'WORK':out.as_posix(),'LOAD':load,'BEFORE':save.replace('save_','before_'),'AFTER':save.replace('save_','after_'),'COEFFICIENTS':conversion,'RECORD':str(RECORD),'IMAGE':str(IMAGE),'BYTES':str(cases*RESULT),'ACTIVITY':''}.items():source=source.replace('@'+k+'@',v)
 (out/'probe.s').write_text(source)
 def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=180,**kw)
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s'])
 run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as stdout,(out/'halfword.log').open('wb') as stderr:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=stdout,stderr=stderr)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Apply both documented GNU writeback patches')
 # Own LINK frame calibration and positive/negative PC watch checks.
 (out/'frame.s').write_text('.include "testutils.inc"\nstart\nimm32 SP, 0x03000000\nCALL frame;\nP1.H = value;\nP1.L = value;\n[P1] = R0;\nR0.H = args;\nR0.L = args;\nP0 = 5;\nEXCPT 0;\nexit 0\nframe:\nLINK 0x10;\nR0 = FP;\nR0 += -16;\nUNLINK;\nRTS;\n.data\n.align 4\nargs: .long 1,value,4\nvalue: .long 0\n')
 run([a.assembler,'-I',a.testutils,'-o',out/'frame.o',out/'frame.s'])
 run([a.linker,'-T',templates/'flat.ld','-o',out/'frame.elf',out/'frame.o'])
 with (out/'frame.bin').open('wb') as stdout:run([a.simulator,'--model','bf524',out/'frame.elf'],stdout=stdout)
 if (out/'frame.bin').read_bytes()!=struct.pack('<I',0x02ffffe8):raise RuntimeError('Own local frame calibration differs')
 # Prove PC watches stop a program of our own before trusting negative guards.
 (out/'watch.s').write_text('.text\n.global __start\n__start:\nR0.H = exit_args;\nR0.L = exit_args;\nP0 = 1;\nEXCPT 0;\n.data\n.align 4\nexit_args: .long 0\n')
 run([a.assembler,'-I',a.testutils,'-o',out/'watch.o',out/'watch.s'])
 run([a.linker,'-T',templates/'flat.ld','-o',out/'watch.elf',out/'watch.o'])
 codes=[]
 for opts in [[],['--watch-pc-int','0x1000'],['--watch-pc-int','0x1010','--watch-pc-int','0x1000'],['--watch-pc-int','0x1000,0x1008']]:
  codes.append(subprocess.run([str(a.simulator),'--model','bf524',*opts,str(out/'watch.elf')],capture_output=True,timeout=180).returncode)
 if codes!=[0,5,5,5]:raise RuntimeError('PC watch probes failed')
 watches=[w for w in WATCHES if not (a.instrument and w=='0x01a84690')]
 guards=[arg for address in watches for arg in ['--watch-pc-int',address]]
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'voice-control-input.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as stdout,(out/'probe.log').open('wb') as stderr:run([a.simulator,'--model','bf524','--memory-size','128M','--memory-region','0xff800000,0x8000','--watch-cycles-int','900000000',*guards,out/'probe.elf'],stdout=stdout,stderr=stderr)
 if (out/'output.bin').stat().st_size!=cases*RESULT+1024:raise RuntimeError('Incomplete output')
 counts={'on':0,'off':0,'appended':0,'gated':0,'duplicates':0,'removed':0,'absent':0,'empty_on':0,'held_on':0,'map_changes':0,'disabled':0,'disabled_held':0,'duplicates_disabled':0};all_output=(out/'output.bin').read_bytes()
 if all_output[:512]!=all_output[-512:] or all_output[:512]!=image[0x018c21b0-base:0x018c23b0-base]:raise RuntimeError('Note table changed')
 oracle=all_output[512:-512];input_bytes=(out/'inputs.bin').read_bytes();previous=0;previous_map=None;previous_list=None
 for c in range(cases):
  x=input_bytes[c*RECORD:(c+1)*RECORD];e=oracle[c*RESULT:(c+1)*RESULT]
  if struct.unpack_from('<I',x)[0]:previous=0;previous_map=x[40+4572:40+4700];previous_list=x[48:48+2560]
  after=struct.unpack_from('<I',e,2568)[0];action=struct.unpack_from('<I',x,12)[0]&1
  counts['on' if action==0 else 'off']+=1
  if not action:
   counts['empty_on' if previous==0 else 'held_on']+=1
   selected=struct.unpack_from('<I',x,40+4392)[0]
   mode=struct.unpack_from('<h',x,40+4240+selected*16+8)[0]//4096
   note=struct.unpack_from('<I',e,9860)[0]
   duplicate=any(struct.unpack_from('<I',previous_list,j*20)[0]==note for j in range(previous))
   counts['gated' if x[44] else 'appended' if after==previous+1 else 'disabled' if mode==4 and not duplicate else 'duplicates']+=1
   if mode==4 and not x[44]:
    if duplicate:counts['duplicates_disabled']+=1
    elif previous:counts['disabled_held']+=1
  else:counts['removed' if after+1==previous else 'absent']+=1
  counts['map_changes']+=sum(a!=b for a,b in zip(previous_map,e[4572:4700]));previous_map=e[4572:4700]
  if e[4704:4748]!=x[40+4704:40+4748] and struct.unpack_from('<I',x)[0]:raise RuntimeError('Transport sentinel changed')
  previous=after;previous_list=e[8:8+2560]
 if previous!=0:raise RuntimeError('Final list not empty')
 artifacts={}
 for name,path in [(prefix+'-input.bin',out/'inputs.bin'),(prefix+'-output.bin',out/'output.bin')]:
  fixture=ROOT/'Tests/ReferenceVectors'/name
  if a.record:fixture.write_bytes(path.read_bytes())
  if fixture.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle differs: '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path)}
 report={'ldr_sha256':expected,'entries':['0x01a85750','0x01a85840'],'notification_entries':['0x019e2e64','0x019e2dc0'],'output_getter':'0x01a69360..0x01a6936c','output_word':'0x01dfe03c','explicit_output_word':0,'cold_pool':a.cold_pool or a.instrument,'instrument':a.instrument,'context':'Original constructor metadata 1, full outer on/off, selectors 1-3, output word zero. No notification transport or erased metadata.','records':cases,'streams':cases//128,'blocks_per_stream':128,'counts':counts,'image_bytes':IMAGE,'input_bytes':RECORD,'output_bytes':RESULT,'regions':[{'address':hex(addr),'bytes':n,'retained_in_stream':r or ((a.cold_pool or a.instrument) and addr==0x01b6053c)} for addr,n,r in REGIONS],'artifacts':artifacts,'tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'model':'GNU BF524 with both documented writeback patches; not hardware or pipeline validation','limitations':['Event order, controls/tuning/selection and ADSR render voice/volume explicit; no recovered scheduler or host integration.','List, activity, registers, pools, RNG, notification map, output word and monitor sentinels retained within each sequence; inner gate explicit per event and zero for off.','Output word zero is a supplied precondition, not a factory default. No packet preparation, actual transport, pending-field consumer or pitch-buffer producer.']}
 paths=[Path(__file__).resolve(),template,ROOT/'Source/DrWave246NoteModes.h',ROOT/'Source/DrWave246HeldNoteModes.h',ROOT/'Source/Voice246QuietNoteDispatch.h',ROOT/'Source/Voice246NoteDispatch.h',ROOT/'Tests/Voice246QuietNoteLifecycleTests.cpp',ROOT/'scripts/verify-drwave246-note-lifecycle.py',ROOT/'scripts/verify-drwave246-poly-first-free.py',ROOT/'scripts/verify-drwave246-adjusted-note-chain.py',ROOT/'scripts/verify-drwave246-note-event-chain.py',ROOT/'scripts/verify-voice246-release-selectors.py']
 paths+=list((ROOT/'Source').glob('*246*.h'))
 if a.cold_pool and not a.instrument:
  report['lazy_accessor']='0x0191264c..0x019126d8';report['initial_ready_words']=[0,65536,4294967295,0]
  ready_changes=0;ready_word=0;terminal=[]
  for c in range(cases):
   x=input_bytes[c*RECORD:(c+1)*RECORD];e=oracle[c*RESULT:(c+1)*RESULT]
   if struct.unpack_from('<I',x)[0]:ready_word=struct.unpack_from('<I',x,40+4384)[0]
   after=struct.unpack_from('<I',e,4384)[0];ready_changes+=ready_word!=after;ready_word=after
   if c%128==127:terminal.append(ready_word)
  if ready_changes!=3 or terminal!=[1,65536,4294967295,0]*3:raise RuntimeError('Cold/quiet initialization coverage differs')
  report['ready_changes']=ready_changes;report['terminal_ready_words']=terminal

 if a.instrument:
  report['context']='Instrument discriminator/outer mode zero, selectors 0..4, constructor or raw nonzero pitch, cold/warm pools, metadata retained, output zero. No clock or L1 handoff.'
  report['fixed_selector_streams']=20;report['mixed_selector_streams']=4
  ready_changes=0;ready_word=0;terminal=[]
  for c in range(cases):
   x=input_bytes[c*RECORD:(c+1)*RECORD];e=oracle[c*RESULT:(c+1)*RESULT]
   if struct.unpack_from('<I',x)[0]:ready_word=struct.unpack_from('<I',x,40+4384)[0]
   after_ready=struct.unpack_from('<I',e,4384)[0];ready_changes+=ready_word!=after_ready;ready_word=after_ready
   if c%128==127:terminal.append(ready_word)
  if ready_changes!=10:raise RuntimeError('Instrument cold initialization coverage differs')
  report['ready_changes']=ready_changes;report['terminal_ready_words']=terminal
  report['inner_entry']='0x01a84604..0x01a84ba0'
  report['routine_sha256']={hex(start)+'..'+hex(end):hashlib.sha256(image[start-base:end-base]).hexdigest() for start,end in [(0x01a85750,0x01a85840),(0x01a85840,0x01a858d6),(0x01a84604,0x01a84ba0)]}
  report['written_adsr_positions']=sum(128 for c in range(cases) if 1<=struct.unpack_from('<I',oracle,c*RESULT+2580+struct.unpack_from('<I',input_bytes,c*RECORD+4)[0]*24)[0]<=8)
  report['preserved_adsr_positions']=cases*128-report['written_adsr_positions']
  report['initial_activity_profiles']=['all free; mixed stream all busy','first three occupied','first occupied; mixed stream all busy','all busy']
  report['initial_ready_words']=[0,65536,0,4294967295]
  report['explicit_pitch_words']=[1,2147483647,2147483648,4294967295,1073741824,305419896]
  paths.append(ROOT/'Tests/Voice246InstrumentLifecycleTests.cpp')
  report['limitations'].append('Raw pitch is supplied from entry, never written over constructor output. Controls/selection/gate/order remain explicit; this is not a MIDI scheduler or plugin integration.')
 report['pc_watches']=watches;report['pc_watch_probe_exit_codes']=codes;report['pc_watch_original_run_exit_code']=0
 report['source_sha256']={str(x.relative_to(ROOT)):sha(x) for x in paths}
 report['patches']={str(x.relative_to(ROOT)):sha(x) for x in [templates/'gnu-13.1-dreg-writeback.patch',templates/'gnu-13.1-halfword-writeback.patch']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS original '+prefix+':',cases,counts)
if __name__=='__main__':main()
