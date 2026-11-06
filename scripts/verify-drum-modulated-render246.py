#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Compose original drum note/reset, retained modulation and explicitly scheduled render."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4608
STRIDE=988
OUT=8728
FRAMES=529200
PCM_BYTES=8*(FRAMES+32)*2
PREFIX=3072+PCM_BYTES
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 seed=0x444d5232
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 data=bytearray(struct.pack('<8h',0,0,40,4096,8192,0,0,32767)*192)
 for b in range(8):
  pcm=[((i*(17+b*6)+b*3271)%65536)-32768 if b%3==0 else (32767 if i%37==0 else -32768 if i%53==0 else 0) if b%3==1 else int(rand()%65536)-32768 for i in range(FRAMES)]
  data+=struct.pack('<'+str(FRAMES+32)+'h',*([0]*16),*pcm,*([0]*16))
 for n in range(CASES):
  session=n//16;frame=n%16;v=session%12;family=session//12%3;sound=session//36%8;pad=(session*5+session//8)%24
  flags=(1 if frame==0 else 0)|(2 if frame==12 else 0)|(4 if frame==15 and session%2 else 0)
  base=[(n%9)*1000,session%6,40+session%8,4096+(session%3)*8192,8192+(session%2)*16384,n*311%32768,n*733%32768,[8192,16384,32767][session%3]]
  reject=n%7==0
  if family==0:mod=[[1000,7000,8192,12288,16384][session%5],[-20000,12000,30000][session%3],4096 if reject else (session%2)*2048,(n%8)*2048,0,0,0,0]
  elif family==1:mod=[[0,16000,32767][session%3],16000+session%14000,2048 if reject else 0,16000+(session%2)*4000,0,0,0,0]
  else:mod=[[1000,8192,23000][session%3],[-16000,14000,28000][session%3],12000+(session%3)*7000,16000+(session%2)*4000,0,0,0,(session%5)*6554]
  kind=1 if family==2 and reject else (0 if session%2 else 3)
  state=bytes(rand()%256 for _ in range(156))
  pitch=[[0x04000000,0x08000000,0x10000000][(session+i//32)%3] for i in range(128)]
  gain=[0 if session%5==0 else 16384 if session%5==1 else ((i*257+n*17)%65536)-32768 for i in range(128)]
  data+=struct.pack('<8I8h8h',flags,v,sound,pad,family,[-12,0,12][session%3]&0xffffffff,0x10000000+(session%7)*0x1000000,kind,*base,*mod)+state+struct.pack('<128I128h',*pitch,*gain)
 assert len(data)==PREFIX+CASES*STRIDE
 return data
def assembly():
 lines=['// SPDX-License-Identifier: AGPL-3.0-only', '// Authored explicit composition; private original code loaded separately.', '.include "testutils.inc"', 'start', 'imm32 SP,0x07f00000']
 def emit(s):lines.extend(s.splitlines())
 def ptr(reg,value):
  if isinstance(value,int):emit(f'imm32 {reg},{value}')
  elif value.startswith('input+'):
   emit(f'imm32 P0,{int(value[6:])}\n{reg}=P4+P0;')
  else:emit(f'{reg}.H={value};\n{reg}.L={value};')
 def copy(name,source,dest,count):
  ptr('P1',source)
  if dest=='result':emit('P2=P5;')
  else:ptr('P2',dest)
  emit(f'P0={count//4};\nLSETUP({name},{name}_end) LC0=P0;\n{name}: R0=[P1++];\n{name}_end: [P2++]=R0;')
  if dest=='result':emit('P5=P2;')
 def zero(name,dest,count,value=0):
  ptr('P1',dest);emit(f'imm32 R0,{value}\nP0={count//4};\nLSETUP({name},{name}_end) LC0=P0;\n{name}: [P1++]=R0;\n{name}_end: NOP;')
 def call(address):emit(f'imm32 P0,{address}\nCALL(P0);')
 # Runtime original table conversion, before results begin.
 for i in range(1025):emit(f'imm32 P0,{0x018bf010+i*4}\nR0=[P0];');call(0x01aa7014);emit(f'imm32 P0,{0xff802154+i*2}\nW[P0]=R0.L;')
 for i in range(24):emit(f'imm32 P0,{0x018bef50+i*4}\nR0=[P0];');call(0x01aa6fd0);emit(f'imm32 P0,{0x0106a8f4+i*4}\n[P0]=R0;')
 emit('imm32 P1,0x018b6688\nimm32 I0,0x0101e5e8\nP0=8192;\nLSETUP(sines,sines_end) LC0=P0;\nsines: R0=[P1++];\nsines_end: W[I0++]=R0.L;')
 call(0x01a1e914)
 for name in ['coeff_args','sine_args','decay_args','range_args']:ptr('R0',name);emit('P0=5;\nEXCPT 0;')
 emit('imm32 P1,0x01cc10cc\nimm32 R0,0x02010000\nR1=8;\nP0=8;\nLSETUP(refs,refs_end) LC0=P0;\nrefs: [P1++]=R0;\nrefs_end: R0=R0+R1;')
 copy('init_controls','initial_controls',0x01aaadb4,3072)
 zero('init_derived',0xff801b50,512,0x6c6c6c6c)
 zero('init_types',0xff801704,32,8)
 ptr('P4','inputs');ptr('P5','results')
 emit('next_case:')
 # Base bank entry pointer stored in data, because all call-clobbered regs get reused.
 emit('R0=[P4+8];\nR1=384;\nR0*=R1;\nR1=[P4+12];\nR1<<=4;\nR0=R0+R1;\nimm32 R1,0x01aaadb4\nR0=R0+R1;')
 ptr('P1','control_ptr');emit('[P1]=R0;\nP2=R0;\nP1=P4;\nP1+=32;\nP0=4;\nLSETUP(base,base_end) LC0=P0;\nbase: R0=[P1++];\nbase_end: [P2++]=R0;')
 copy('workspace','input+32',0xff801724,16)
 emit('R0=[P4+8];\nR0<<=4;\nimm32 R1,0xff801644\nR0=R0+R1;')
 ptr('P1','mod_ptr');emit('[P1]=R0;\nP2=R0;\nP1=P4;\nP1+=48;\nP0=4;\nLSETUP(mods,mods_end) LC0=P0;\nmods: R0=[P1++];\nmods_end: [P2++]=R0;')
 emit('R0=[P4+8];\nimm32 P1,0xff8016d8\n[P1]=R0;\nimm32 P1,0x01aaac10\n[P1]=R0;\nR0=[P4+24];\nimm32 P1,0x01aadad4\n[P1]=R0;\nR0=[P4];\nCC=BITTST(R0,0);\nIF !CC JUMP reset_done;')
 zero('core_clear',0x01aaac0c,68)
 zero('mod_table_clear',0x01aaad60,84)
 zero('mod_banks_clear',0xff801644,128)
 # Restore current mod params after clearing all banks.
 emit('P1=P4;\nP1+=48;');ptr('P2','mod_ptr');emit('P2=[P2];\nP0=4;\nLSETUP(mod_reset,mod_reset_end) LC0=P0;\nmod_reset: R0=[P1++];\nmod_reset_end: [P2++]=R0;')
 zero('globals_clear',0xff801038,812)
 zero('pool_clear',0xffb00000,240)
 zero('held_clear',0x01dfe734,2564,0x5a5a5a5a)
 emit('imm32 P1,0x01dff134\nR0=0;\n[P1]=R0;\nimm32 P1,0xff801038\nR0=1;\n[P1]=R0;\nimm32 P1,0xff801070\nimm32 R0,0x01010101\n[P1++]=R0;\n[P1++]=R0;\n[P1]=R0;\nR0=[P4+4];\nP0=R0;\nimm32 P1,0xff801070\nP1=P1+P0;\nR0=0;\nB[P1]=R0;\nR0=[P4+8];\nimm32 P1,0x01aaac10\n[P1]=R0;\nR0=8;');call(0x01967984)
 copy('value_seed','input+64',0x02010100,12)
 copy('random_seed','input+76',0x02010120,44)
 copy('trem_seed','input+120',0x02010150,28)
 copy('value_display','input+148',0x01dd5d04,32)
 copy('trem_display','input+180',0x01dd5a88,32)
 copy('meter_seed','input+212',0x01dd5c6c,4)
 copy('rng_seed','input+216',0xff802bc8,4)
 for obj,vt in [(0x02010100,0x02010180),(0x02010120,0x020101a0),(0x02010150,0x020101c0)]:emit(f'imm32 P1,{obj}\nimm32 R0,{vt}\n[P1]=R0;')
 emit('R0=[P4+8];\nimm32 P1,0x01aaac10\n[P1]=R0;\nP0=R0;\nimm32 P1,0x01aaac38\nP1=P1+P0;\nR0=1;\nB[P1]=R0;\nR0=[P4+16];\nCC=R0==0;\nIF CC JUMP value_object;\nCC=R0==1;\nIF CC JUMP random_object;\nimm32 R0,0x02010150\nJUMP object_done;\nvalue_object: imm32 R0,0x02010100\nJUMP object_done;\nrandom_object: imm32 R0,0x02010120\nobject_done: R1=[P4+4];\nR1<<=2;\nP0=R1;\nimm32 P1,0x01aaad84\nP1=P1+P0;\n[P1]=R0;')
 # Event words written every reset; note-off later uses same note.
 ptr('P1','event');emit('R0=[P4+12];\nR0+=53;\n[P1]=R0;\nR0=100;\n[P1+4]=R0;\nR0=171;\n[P1+8]=R0;\nimm32 R0,0x24681357\n[P1+12]=R0;\nimm32 P1,0x018c2180\nR0=-1;\n[P1]=R0;')
 ptr('R0','event');emit('R1=[P4+20];');call(0x01a84ba0)
 emit('reset_done: R0=[P4];\nCC=BITTST(R0,1);\nIF !CC JUMP off_done;');ptr('R0','event');call(0x01a84dac)
 emit('off_done:')
 copy('pitch','input+220',0xff801950,512)
 copy('gain','input+732',0xff801d50,256)
 emit('R0=[P4+16];\nCC=R0==0;\nIF CC JUMP value_step;\nCC=R0==1;\nIF CC JUMP random_step;\nimm32 R0,0x02010150\nimm32 P3,0x019cbe48\nJUMP step_ready;\nvalue_step: imm32 R0,0x02010100\nimm32 P3,0x019cfdac\nJUMP step_ready;\nrandom_step: imm32 R0,0x02010120\nimm32 P3,0x019cedc0\nstep_ready:')
 ptr('P1','mod_ptr');emit('R1=[P1];\nR2=[P4+28];\nCALL(P3);');ptr('P1','step_return');emit('[P1]=R0;')
 zero('poison_audio','audio',256,0x24681357)
 emit('R0=[P4];\nCC=BITTST(R0,2);\nIF CC JUMP rendered;\nR0=[P4+4];\nR1=20;\nR0*=R1;\nimm32 R1,0xffb00000\nR0=R0+R1;');ptr('R1','audio');emit('imm32 R2,0xff801724');call(0x019789ec)
 emit('rendered:')
 for name,address,count in [('pool',0xffb00000,240),('globals',0xff801038,812),('value',0x02010100,12),('random',0x02010120,44),('trem',0x02010150,28),('vdisp',0x01dd5d04,32),('tdisp',0x01dd5a88,32),('meter',0x01dd5c6c,4),('rng',0xff802bc8,4),('targets',0xff801724,16),('return','step_return',4),('audio','audio',256),('pitch',0xff801950,512),('gain',0xff801d50,256),('derived',0xff801b50,512),('held',0x01dfe734,2564),('mods',0xff801644,128),('pointers',0x01aaad30,48),('base',0x01aaadb4,3072),('core',0x01aaac0c,68),('modptrs',0x01aaad60,84)]:copy('save_'+name,address,'result',count)
 emit(f'imm32 P0,{STRIDE}\nP4=P4+P0;');ptr('P0','inputs_end');emit('CC=P4==P0;\nIF CC JUMP cases_done;\nJUMP.L next_case;\ncases_done:')
 for name in ['io_args','pcm_args']:ptr('R0',name);emit('P0=5;\nEXCPT 0;')
 emit('exit 0\n.data\n.align 4\nevent: .space 16\ncontrol_ptr: .space 4\nmod_ptr: .space 4\nstep_return: .space 4\naudio: .space 256\ncoeff_args: .long 1,0xff802154,2052\nsine_args: .long 1,0x0101e5e8,16384\ndecay_args: .long 1,0x0106a8f4,96\nrange_args: .long 1,0x01de5a90,64')
 emit(f'io_args: .long 1,results,{CASES*OUT}\npcm_args: .long 1,samples,{PCM_BYTES}\ninitial_controls: .incbin "@WORK@/inputs.bin",0,3072\nsamples: .incbin "@WORK@/inputs.bin",3072,{PCM_BYTES}\ninputs: .incbin "@WORK@/inputs.bin",{PREFIX}\ninputs_end:')
 emit('.section .handles,"aw"')
 for b in range(8):emit(f'.long {0x02010004+b*8},samples+{b*(FRAMES+32)*2+32}')
 emit('.space 192\n.space 128') # offset 0x180: objects reside in preceding space
 for reset in [0x019cff16,0x019cef66,0x019cbdca]:emit(f'.space 24\n.long 0,{reset}')
 emit(f'.section .results,"aw",@nobits\nresults: .space {CASES*OUT}\n.section .sdram,"awx"\n.incbin "@WORK@/sdram.bin"')
 return '\n'.join(lines)+'\n'
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,count,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+count<=base+len(image) and not flags&0x1000:image[address-base:address-base+count]=(struct.pack('<I',arg)*((count+3)//4))[:count] if flags&0x100 else raw[cursor+16:cursor+16+count]
  cursor+=16+(0 if flags&0x100 else count)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs());templates=ROOT/'research/blackfin'
 def execute(stem,source,linker='flat.ld',extra=()):
  def run(cmd,**kw):subprocess.run(list(map(str,cmd)),check=True,timeout=300,**kw)
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as f,(out/(stem+'.log')).open('wb') as log:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=f,stderr=log)
  return out/(stem+'.bin')
 for name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  if struct.unpack('<5I',execute(Path(name).stem,(templates/name).read_text()).read_bytes())!=probe:raise RuntimeError('All three verified writeback patches required')
 source=assembly().replace('@WORK@',out.as_posix())
 result=execute('drum-modulated-render246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=18596+CASES*OUT+PCM_BYTES:raise RuntimeError('Incomplete modulated render output: '+str(result.stat().st_size))
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/drum-modulated-render/vectors'/f'drum-modulated-render246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'blocks':CASES,'sessions':288,'input_prefix_bytes':PREFIX,'input_record_bytes':STRIDE,'output_prefix_bytes':18596,'output_record_bytes':OUT,'output_suffix_bytes':PCM_BYTES,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Explicit 16-block cadence: note-on, retained modulation each block, note-off at block 12, render except final block of odd sessions. Not recovered original scheduling.','Pitch and gain buffers, PCM, controls and kinds are supplied. Original pitch/gain producers and Tremolo factory routing are not recovered.','No plugin changes, acoustic equivalence, installers or presets claimed.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 4608 original modulated render blocks in 288 retained sessions')
if __name__=='__main__':main()
