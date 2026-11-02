#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded CWO control and phase building blocks with supplied firmware.
Original code/ELF stay private. Four protected Hilbert coefficients are explicitly
substituted by the authored phase model; this is not original CWO audio validation.
"""
import argparse,hashlib,importlib.util,json,math,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES,RECORD,RESULT=8192,44,272
OUTPUT_BYTES=CASES*RESULT+16*131072

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 out=bytearray();seed=0x43574f54
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for i in range(CASES):
  state=struct.pack('<IhhI',65530 if i%1024==0 else rnd()%65536,int(rnd()%65536)-32768,int(rnd()%65536)-32768,(rnd()&0xffffff00)|(i//512%2))
  delay=[1,2047,2048,2049,4095,4096,0x7ffffff,rnd()%0x7ffffff+1][i%8]
  delta=[0,60,-60,61,-61,2147483647,-2147483648,int(rnd()%1000)-500][i//8%8]
  phases=[-1073741824,-1073741823,-1,0,1,1073741823]
  phase=phases[i%6] if i%16<6 else int(rnd()%2147483648)-1073741824
  step=phases[(i//6)%6] if i%16<6 else int(rnd()%2147483648)-1073741824
  out+=state+struct.pack('<hHIiIIiihH',int(rnd()%65536)-32768,0,delay,delta,int(i%512==0),rnd(),phase,step,int(rnd()%65536)-32768,0)
 return out

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--phase-model',type=Path,default=ROOT/'research/blackfin/cwo246-phase-model.json');p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Choose a NEW private output directory outside the checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Only inventoried rev246 supported')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');mapper=importlib.util.module_from_spec(spec);spec.loader.exec_module(mapper)
 raw=a.ldr.read_bytes();meta,_=mapper.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);offset=0
 while offset<len(raw):
  flags,address,count,argument=struct.unpack_from('<4I',raw,offset)
  if base<=address and address+count<=base+len(image) and not flags&0x1000:image[address-base:address-base+count]=(struct.pack('<I',argument)*((count+3)//4))[:count] if flags&0x100 else raw[offset+16:offset+16+count]
  offset+=16+(0 if flags&0x100 else count)
 model=json.loads(a.phase_model.read_text());coefficients=model['float32_coefficients']
 if len(coefficients)!=8 or any(not math.isfinite(v) or not 0<v<1 for v in coefficients):raise ValueError('Expected eight stable finite phase coefficients')
 phase_words=struct.pack('<8f',*coefficients);source_words=[];offset=0
 while offset<len(raw):
  flags,address,count,argument=struct.unpack_from('<4I',raw,offset)
  for index in range(8):
   target=0xff9004b0+index*4
   if address<=target and target+4<=address+count:
    payload_offset=target-address;protected=payload_offset%24<8
    if not protected and phase_words[index*4:index*4+4]!=raw[offset+16+payload_offset:offset+20+payload_offset]:raise ValueError('Phase model altered a legible coefficient')
    source_words.append({'index':index,'address':hex(target),'ldr_block':hex(address),'flags':hex(flags),'payload_offset':payload_offset,'offset_mod24':payload_offset%24,'protected_under_partial_model':protected})
  offset+=16+(0 if flags&0x100 else count)
 if len(source_words)!=8:raise ValueError('Missing inventoried phase coefficient payload')
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs());(out/'phase-floats.bin').write_bytes(phase_words)
 source=(ROOT/'research/blackfin/cwo-time.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(OUTPUT_BYTES));(out/'probe.s').write_text(source)
 def run(args,**kw):subprocess.run([str(x) for x in args],check=True,timeout=120,**kw)
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',ROOT/'research/blackfin/cwo-foundation.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as stdout,(out/'probe.log').open('wb') as stderr:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','1500000000',out/'probe.elf'],stdout=stdout,stderr=stderr)
 if (out/'output.bin').stat().st_size!=22684+OUTPUT_BYTES:raise RuntimeError('Incomplete CWO time oracle')
 artifacts={}
 for name,path in [('cwo246-time-input.bin',out/'inputs.bin'),('cwo246-time-output.bin',out/'output.bin')]:
  fixture=ROOT/'research/cwo-foundation/vectors'/name
  if a.record:fixture.write_bytes(path.read_bytes())
  if fixture.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle differs: '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 evidence={'ldr_sha256':expected,'routines':['0x01943744 variable delay','0x01944480 phase lookup','0x019446e0 64-sample oscillator'],'cases':CASES,'table_prefix_bytes':22684,'delay_state_bytes':12,'workspace_bytes':131072,'complete_workspace_snapshots':16,'explicit_slot':0,'model_sha256':sha(a.phase_model),'tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'artifacts':artifacts,'limitations':['Same explicit phase-model initializer; delay and oscillator do not consume Hilbert coefficients.','Valid cursor and delay 1..0x7ffffff, explicit pointer binding to slot 0 workspace.','Phase and increment in [-2^30,2^30), explicit signed16 amplitude; original allocator/root callback not established.']}
 (out/'evidence.json').write_text(json.dumps(evidence,indent=2)+'\n');print('PASS: CWO delay and block oscillator, 8192 calls and 16 complete workspaces reproduced')
if __name__=='__main__':main()
