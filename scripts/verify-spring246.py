#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce the mono Spring rev246 kernel with operator-supplied firmware.
Original code/ELF stay private. Shared tables use legible conversion fragments;
Spring static initialization is executed. Explicit workspace is not the allocator.
"""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES,RECORD,RESULT=4096,312,300
OUTPUT_BYTES=CASES*RESULT+8*29136*2

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 out=bytearray();seed=0x53505247
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 lengths=[382,1272,5330,3418,1848,5902,596,3490,6898]
 for i in range(CASES):
  state=struct.pack('<14I4h10I',*[rnd() for _ in range(14)],*[int(rnd()%65536)-32768 for _ in range(4)],*[rnd()%n for n in lengths],0)
  c=[(1344,16448,8192,14000)[i//128%4],(7744,16448,12000,16000)[i//32%4],(0,16384,8192,rnd()%16385)[i%4],(0,32767,8192,rnd()%32768)[i%4]]
  audio=[int(rnd()%65536)-32768 for _ in range(64)]
  if i%8==0:audio=[0]*64
  elif i%8==1:audio=[32767]+[0]*63
  elif i%8==2:audio=[32767 if j%2 else -32768 for j in range(64)]
  elif i%8==3:audio=[-32768]*64
  action=(2 if i%1024==0 else 1) if i%512==0 else 0
  out+=state+struct.pack('<4h64h16I2I',*c,*audio,*[rnd() for _ in range(16)],action,rnd())
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
 source=(ROOT/'research/blackfin/spring-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(OUTPUT_BYTES));(out/'probe.s').write_text(source)
 def run(args,**kw):subprocess.run([str(x) for x in args],check=True,timeout=120,**kw)
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',ROOT/'research/blackfin/voice-contract.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as stdout,(out/'probe.log').open('wb') as stderr:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','1500000000',out/'probe.elf'],stdout=stdout,stderr=stderr)
 if (out/'output.bin').stat().st_size!=790+OUTPUT_BYTES:raise RuntimeError('Incomplete Spring oracle')
 artifacts={}
 for name,path in [('spring246-input.bin',out/'inputs.bin'),('spring246-output.bin',out/'output.bin')]:
  fixture=ROOT/'Tests/ReferenceVectors'/name
  if a.record:fixture.write_bytes(path.read_bytes())
  if fixture.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle differs: '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 evidence={'ldr_sha256':expected,'kernel':'0x01a326e8','reset':'0x01a32e9a, explicit slot 0','cases':CASES,'samples':CASES*64,'state_bytes':104,'display_bytes':64,'workspace':'Nine lines, 29136 int16 cells, slot 0; eight complete snapshots','tables':'129 Q31 frequency, 129 Q15 low-pass; original static initializer produces packed Spring constants','tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'artifacts':artifacts,'limitations':['Mono callback, valid line cursors, explicit workspace slot 0.','Original allocator/routing/clock and complete instrument startup are not established.']}
 (out/'evidence.json').write_text(json.dumps(evidence,indent=2)+'\n');print('PASS: Spring 4096 blocks, complete states/meters, pending counters and workspaces reproduced')
if __name__=='__main__':main()
