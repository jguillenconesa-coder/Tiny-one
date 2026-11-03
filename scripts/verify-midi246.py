#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce isolated MIDI modulation observations using privately supplied rev246."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 seed=0x4d494449; data=bytearray()
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for family in range(3):
  for case in range(2048):
   types=[0,1,2,3,4,6,7,11,12,13] if family==0 else [8,9] if family==1 else list(range(8))
   type=types[(case//4 if family<2 else case)%len(types)];kind=case%4 if family<2 else [2,5][(case//8)%2]
   # Directed cases cover every CC value, destination, stream and colour;
   # later cases mix repeated destinations with boundary target words.
   destination=[-1,0,1,2,3,4][(case//128)%6]
   controls=[((case//16+i)%8)*4096 for i in range(4)]+[destination*2048]*4
   if type in [6,8,9,12]:controls[:4]=[((case//16+i)%8)*2048 for i in range(4)]
   if case>=768:
    controls=[int(rand()%32768) for _ in range(4)]+[[0,2048,4096,6144,-2048][rand()%5] for _ in range(4)]
   if type in [6,8,9,12]:controls[:4]=[v%16384 for v in controls[:4]]
   for i in range(4):
    if controls[4+i]==2048:controls[i]%=16384 # Auxiliary ENV colours are outside the bounded API.
   cc=[(case+i*31)%128 for i in range(4)]
   if case%7==0:cc=[64]*4
   display=[int(rand()%65536)-32768 for _ in range(4)]
   targets=[int(rand()%32768) for _ in range(8)]
   if case%4==0:targets=[0,32767,1,16384,8192,24576,0,32767]
   pitch=[rand() for _ in range(128)];gain=[int(rand()%65536)-32768 for _ in range(128)];audio=[int(rand()%65536)-32768 for _ in range(128)]
   data+=struct.pack('<4I8h4I4h8h128I128h128h',family,type,kind,rand(),*controls,*cc,*display,*targets,*pitch,*gain,*audio)
 return data

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,count,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+count<=base+len(image) and not flags&0x1000:image[address-base:address-base+count]=(struct.pack('<I',arg)*((count+3)//4))[:count] if flags&0x100 else raw[cursor+16:cursor+16+count]
  cursor+=16+(0 if flags&0x100 else count)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs());templates=ROOT/'research/blackfin'
 def execute(stem,source,linker='voice-contract.ld',extra=()):
  def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=300,**kw)
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as stdout,(out/(stem+'.log')).open('wb') as stderr:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=stdout,stderr=stderr)
  return out/(stem+'.bin')
 for name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  path=execute(Path(name).stem,(templates/name).read_text(),'flat.ld')
  if struct.unpack('<5I',path.read_bytes())!=probe:raise RuntimeError('All three documented writeback patches required')
 size=6144*1072;source=(templates/'midi-lfo-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(size))
 result=execute('midi246',source,extra=('--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000'))
 if result.stat().st_size!=size:raise RuntimeError('Incomplete output')
 # Gzip contains numerical observations only, never executable firmware bytes.
 import gzip
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'Tests/ReferenceVectors'/f'midi246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'callbacks':{'synth':'0x019cce68','drum':'0x019cd048','fx':'0x019cd104'},'model':'BF524 simulator, three verified writeback patches; not hardware','calls':6144,'calls_per_family':2048,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Supplied globals, controls, CC, targets, pitch, gain and FX audio; independent calls, not receiver or scheduler.','FX kinds 2/5 only; no kind-4 auxiliary envelope path.','No complete voice, protected gain producer or acoustic equivalence claim.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 6144 original MIDI callbacks, supplied input/global/output streams')
if __name__=='__main__':main()
