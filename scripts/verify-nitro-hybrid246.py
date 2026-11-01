#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce the bounded mono Nitro hybrid rev246 kernel with operator-supplied firmware.
Original code/ELF stay private. Explicit authored mix/pointer substitutions are
required: this is conditional arithmetic validation, NOT decrypted original data.
Only the shared frequency table comes from the legible conversion fragment.
"""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES,RECORD,RESULT=4096,264,252

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 out=bytearray();seed=0x4e495452
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for i in range(CASES):
  state=[rnd() for _ in range(15)];state[6]=i%4
  c=[64+rnd()%16385,int(rnd()%65536)-32768,rnd()%20644,64+rnd()%16385]
  if i<512:
   c[0]=(64,127,128,4096,8192,16448)[i%6]
   c[1]=(-32768,-16384,0,16384,32767)[i//6%5]
   c[2]=(0,1,8192,16384,20643)[i//30%5]
   c[3]=(64,8192,16448)[i//20%3]
  if 2048<=i<3072:c=[512,12000,1024*((i//32)%12),14000]
  samples=[int(rnd()%65536)-32768 for _ in range(64)]
  if i%8==0:samples=[0]*64
  elif i%8==1:samples=[-32768]*64
  elif i%8==2:samples=[32767 if j%2 else -32768 for j in range(64)]
  elif i%8==3:samples=[32767]+[0]*63
  elif i%8==4:samples=[(j-32)*512 for j in range(64)]
  action=1 if i<2048 or i%128==0 else 0
  out+=struct.pack('<15I4h64h16II',*state,*c,*samples,*[rnd() for _ in range(16)],action)
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
 source=(ROOT/'research/blackfin/nitro-hybrid-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*RESULT));(out/'probe.s').write_text(source)
 def run(args,**kw):subprocess.run([str(x) for x in args],check=True,timeout=120,**kw)
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',ROOT/'research/blackfin/voice-contract.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as stdout,(out/'probe.log').open('wb') as stderr:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','1500000000',out/'probe.elf'],stdout=stdout,stderr=stderr)
 if (out/'output.bin').stat().st_size!=536+CASES*RESULT:raise RuntimeError('Incomplete Nitro hybrid oracle')
 artifacts={}
 for name,path in [('nitro-hybrid246-input.bin',out/'inputs.bin'),('nitro-hybrid246-output.bin',out/'output.bin')]:
  fixture=ROOT/'Tests/ReferenceVectors'/name
  if a.record:fixture.write_bytes(path.read_bytes())
  if fixture.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle differs: '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 evidence={'ldr_sha256':expected,'kernel':'0x019a62a4','cases':CASES,'samples':CASES*64,'state_bytes':60,'display_bytes':64,'conditional_model':'Authored mix [1/6,-2/3,1,-2/3,1/6] and four explicit coefficient pointers substituted; NOT original decrypted tables','frequency':'Same legible conversion fragment as Filter','tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'artifacts':artifacts,'limitations':['Conditional instruction-path equivalence only; protected coefficients and pointers remain unknown.','Original stereo routing, clock and host output adaptation unverified.']}
 (out/'evidence.json').write_text(json.dumps(evidence,indent=2)+'\n');print('PASS: Nitro hybrid 4096 blocks / 262144 samples, complete states/meters reproduced under explicit substitutions')
if __name__=='__main__':main()
