#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce only Spring rev246 reset and its two deferred-clear callbacks.
Requires operator-supplied firmware; original code and generated ELF stay private.
No active Spring audio renderer, allocator or complete startup is established.
"""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES,RECORD,RESULT=24,316,116848

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 out=bytearray();seed=0x53505247
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for group in range(8):
  state=struct.pack('<26I',*[rnd() for _ in range(26)])
  for action in (0,1,1):
   out+=state+struct.pack('<4h64h16I3I',1344+group*128,7744+group*128,group*2048,group*4096,*[int(rnd()%65536)-32768 for _ in range(64)],*[rnd() for _ in range(16)],action,rnd(),group%2)
 return out

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Choose a NEW private output directory outside the checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Only inventoried rev246 supported')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');mapper=importlib.util.module_from_spec(spec);spec.loader.exec_module(mapper)
 raw=a.ldr.read_bytes();meta,_=mapper.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);offset=0
 while offset<len(raw):
  flags,address,count,argument=struct.unpack_from('<4I',raw,offset)
  if base<=address and address+count<=base+len(image) and not flags&0x1000:image[address-base:address-base+count]=(struct.pack('<I',argument)*((count+3)//4))[:count] if flags&0x100 else raw[offset+16:offset+16+count]
  offset+=16+(0 if flags&0x100 else count)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs())
 source=(ROOT/'research/blackfin/spring-lifecycle.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*RESULT));(out/'probe.s').write_text(source)
 def run(args,**kw):subprocess.run([str(x) for x in args],check=True,timeout=120,**kw)
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',ROOT/'research/blackfin/voice-contract.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as stdout,(out/'probe.log').open('wb') as stderr:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','1500000000',out/'probe.elf'],stdout=stdout,stderr=stderr)
 if (out/'output.bin').stat().st_size!=CASES*RESULT:raise RuntimeError('Incomplete Spring lifecycle oracle')
 artifacts={}
 for name,path in [('spring246-lifecycle-input.bin',out/'inputs.bin'),('spring246-lifecycle-output.bin',out/'output.bin')]:
  fixture=ROOT/'Tests/ReferenceVectors'/name
  if a.record:fixture.write_bytes(path.read_bytes())
  if fixture.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle differs: '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 evidence={'ldr_sha256':expected,'reset':'0x01a32e9a','callback':'0x01a326e8 pending branch only','cases':CASES,'original_resets':8,'deferred_clear_calls':16,'state_bytes':104,'display_bytes':64,'workspace_bytes_per_slot':58272,'explicit_slots':[0,1],'full_dual_slot_snapshots':24,'tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'artifacts':artifacts,'limitations':['Synthetic workspace contents and state; no original allocator or startup.','Active audio branch, static initializer and coefficients are not executed or validated.','This is a lifecycle building block, not an integrated Spring effect.']}
 (out/'evidence.json').write_text(json.dumps(evidence,indent=2)+'\n');print('PASS: Spring lifecycle 8 resets, 16 deferred callbacks and 24 complete dual-slot snapshots reproduced')
if __name__=='__main__':main()
