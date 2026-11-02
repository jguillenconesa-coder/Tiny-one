#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded CWO control and phase building blocks with supplied firmware.
Original code/ELF stay private. Four protected Hilbert coefficients are explicitly
substituted by the authored phase model; this is not original CWO audio validation.
"""
import argparse,hashlib,importlib.util,json,math,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES,RECORD,RESULT=65536,196,196
OUTPUT_BYTES=CASES*RESULT

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 out=bytearray();seed=0x43574f46
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for i in range(CASES):
  state=([rnd()&1]+[rnd() for _ in range(33)]) if i%4096==0 else [0]*34
  if i%4096==0 and i>=32768:state=[(i//4096)&1]+[0x80000000 if j%2 else 0x7fffffff for j in range(33)]
  c=[i%32768,(i*8191)%32768,(i*17)%32768,(i*31)%32768]
  out+=struct.pack('<4H4h2I34II',*c,i-32768,int(rnd()%65536)-32768,int((i*13)%65536)-32768,int((i*7)%65536)-32768,rnd(),rnd(),*state,int(i%4096==0))
  filter_state=[rnd(),rnd(),rnd()];smooth_state=[rnd(),rnd()];filter_coefficient=rnd()&0x7fffffff;smooth_coefficient=rnd()
  if i%4096==0:
   filter_state=[0x80000000,0x7fffffff,0x80000000];smooth_state=[0x80000000 if i%8192==0 else 0x7fffffff,0x7fffffff]
   filter_coefficient=(0,0x7fffffff,0x80000000)[i//4096%3];smooth_coefficient=(0,0x7fffffff,0x80000000)[i//4096%3]
  out+=struct.pack('<3IhH3I',*filter_state,int(rnd()%65536)-32768,0,filter_coefficient,*smooth_state)+struct.pack('<I',smooth_coefficient)
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
 source=(ROOT/'research/blackfin/cwo-foundation.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(OUTPUT_BYTES));(out/'probe.s').write_text(source)
 def run(args,**kw):subprocess.run([str(x) for x in args],check=True,timeout=120,**kw)
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',ROOT/'research/blackfin/cwo-foundation.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as stdout,(out/'probe.log').open('wb') as stderr:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','1500000000',out/'probe.elf'],stdout=stdout,stderr=stderr)
 if (out/'output.bin').stat().st_size!=22684+OUTPUT_BYTES:raise RuntimeError('Incomplete CWO foundation oracle')
 artifacts={}
 for name,path in [('cwo246-foundation-input.bin',out/'inputs.bin'),('cwo246-foundation-output.bin',out/'output.bin')]:
  fixture=ROOT/'research/cwo-foundation/vectors'/name
  if a.record:fixture.write_bytes(path.read_bytes())
  if fixture.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle differs: '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 evidence={'ldr_sha256':expected,'routines':['0x01946228 static init with explicit substitutions','0x019463b0 interpolation producer','0x01945da8 control table producer','0x01944528 frequency mapping','0x019445a8 feedback mapping','0x01944660 delay mapping','0x0194444e cubic clipper','0x01943b2e allpass stage','0x01943e0a dual phase network','0x01943bea conditioning filter','0x0194435c control smoother'],'cases':CASES,'table_prefix_bytes':22684,'phase_state_bytes':136,'phase_state_reloads':16,'phase_source_words':source_words,'model_sha256':sha(a.phase_model),'tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'artifacts':artifacts,'limitations':['Four protected Hilbert coefficients explicitly substituted by the authored model.','Original root callback, delay workspace, smoother cadence, routing and host integration not established.','Raw control domain 0..32767; default ASTAT rounding and valid phase parity only.']}
 (out/'evidence.json').write_text(json.dumps(evidence,indent=2)+'\n');print('PASS: CWO foundation tables, 65536 control/clip/allpass/filter/smoother/dual-phase cases reproduced')
if __name__=='__main__':main()
