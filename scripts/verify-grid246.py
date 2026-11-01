#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce the mono Grid rev246 kernel with operator-supplied firmware.
Original code/ELF stay private; the verified sine is copied from its legible source. The external workspace binding is explicit and not an original allocator.
"""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES,RECORD,RESULT=4096,248,236
OUTPUT_BYTES=CASES*RESULT+8*135200*2

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 out=bytearray();seed=0x47524944
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for i in range(CASES):
  sx=(1344,2560,8192,16704,4096,14080,10240,6144)[i//512];sy=(16704,1344,8192,2560,14080,4096,6144,10240)[i//512]
  width=2*(sx//128);height=2*(sy//128)
  state=[rnd(),rnd()%2,rnd()%2,rnd()%2,1+rnd()%(width-2),1+2*(rnd()%((height-2)//2)),1+rnd()%(width-2),1+2*(rnd()%((height-2)//2)),rnd()%259,rnd()%259,2]
  c=[sx,sy,rnd()%32768,rnd()%32768]
  if i%16<4:c[2]=(0,1,16384,32767)[i%4];c[3]=(0,1,16384,32767)[i%4]
  samples=[int(rnd()%65536)-32768 for _ in range(64)]
  if i%8==0:samples=[0]*64
  elif i%8==1:samples=[32767]+[0]*63
  elif i%8==2:samples=[32767 if j%2 else -32768 for j in range(64)]
  action=(2 if i%1024==0 else 1) if i%512==0 else 0
  out+=struct.pack('<11I4h64h16II',*state,*c,*samples,*[rnd() for _ in range(16)],action)
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
 source=(ROOT/'research/blackfin/grid-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(OUTPUT_BYTES));(out/'probe.s').write_text(source)
 def run(args,**kw):subprocess.run([str(x) for x in args],check=True,timeout=120,**kw)
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',ROOT/'research/blackfin/voice-contract.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as stdout,(out/'probe.log').open('wb') as stderr:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','1500000000',out/'probe.elf'],stdout=stdout,stderr=stderr)
 if (out/'output.bin').stat().st_size!=OUTPUT_BYTES:raise RuntimeError('Incomplete Grid oracle')
 artifacts={}
 for name,path in [('grid246-input.bin',out/'inputs.bin'),('grid246-output.bin',out/'output.bin')]:
  fixture=ROOT/'Tests/ReferenceVectors'/name
  if a.record:fixture.write_bytes(path.read_bytes())
  if fixture.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle differs: '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 evidence={'ldr_sha256':expected,'kernel':'0x019af338','reset':'0x019af27e, explicit slot 2 workspace','cases':CASES,'samples':CASES*64,'state_bytes':44,'display_bytes':64,'workspace':'Two 260x260 int16 planes, 135200 cells; complete snapshot after each 512-block segment','sine':'8192 low halfwords from legible 0x018b6688; already verified sine fixture','tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'artifacts':artifacts,'limitations':['Mono callback, valid bounded scan coordinates, explicit workspace slot 2.','Original allocator/routing/clock and abrupt invalid resize states are not established.']}
 (out/'evidence.json').write_text(json.dumps(evidence,indent=2)+'\n');print('PASS: Grid 4096 blocks / 262144 samples, complete states/meters and two-plane workspaces reproduced')
if __name__=='__main__':main()
