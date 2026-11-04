#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded Element modulation observations using privately supplied rev246."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 seed=0x454c454d;data=bytearray()
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 def signed():return int(rand()%65536)-32768
 patterns=[([int(rand()%65536)-32768 for _ in range(64)],[int(rand()%65536)-32768 for _ in range(128)]) for _ in range(16)]
 for case in range(18432):
  family=case%3;types=[0,1,2,3,4,6,7,11,12,13] if family==0 else [8,9] if family==1 else list(range(8));frame=case//3;type=types[frame%len(types)];kinds=4 if family<2 else 2;kind=frame//len(types)%4 if family<2 else [2,5][frame//len(types)%2]
  destination=[0,1,2,-1][frame//(len(types)*kinds)%4];source=frame//(len(types)*kinds*4)%4;colour=frame//(len(types)*kinds*4*4)%8
  if destination==1:colour%=4
  extended=type in [6,8,9,12] and destination==0;amount=signed()
  if case%8==0:amount=[-32768,-32767,-16384,-1,0,1,16384,32767][case//8%8]
  c=[source*2048,amount,destination*2048,colour*(2048 if extended else 4096),signed(),signed(),signed(),signed()];targets=[int(rand()%32768) for _ in range(8)]
  sensor=signed();env=signed();level=signed();drum=rand()
  if case%8==0:sensor=[-32768,-1,0,1,32767][case//8%5]
  external,gain=[list(v) for v in patterns[case%16]];pattern=case%6
  if pattern<3:
   external=[-abs(v) for v in external];gain=[-abs(v) for v in gain]
   if pattern==1:external=[0]*64;gain=[0]*128
   if pattern==2:external[case//6%64]=[1,8191,8192,32767][case//384%4];gain[case//6%128]=[1,8191,8192,32767][case//768%4]
  elif pattern==3:external=[v//16 for v in external];gain=[v//16 for v in gain]
  data+=struct.pack('<4I8h8h4hI64h128h',family,type,kind,rand(),*c,*targets,sensor,env,level,0,drum,*external,*gain)
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
 # Element is factory index 3; its original source file is LfoReRoute.
 if struct.unpack_from('<I',image,0x018e1358-base+3*4)[0]!=0x019c9a88:raise RuntimeError('Unexpected Element factory selector')
 for address,callback in [(0x019108b0,0x019cda14),(0x019108d0,0x019cde28),(0x019108f0,0x019cdf22)]:
  if struct.unpack_from('<I',image,address-base)[0]!=callback:raise RuntimeError('Unexpected Element vtable')
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs());templates=ROOT/'research/blackfin'
 def execute(stem,source,linker='voice-contract.ld',extra=()):
  def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=300,**kw)
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as stdout,(out/(stem+'.log')).open('wb') as stderr:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=stdout,stderr=stderr)
  return out/(stem+'.bin')
 for name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  path=execute(Path(name).stem,(templates/name).read_text(),'flat.ld')
  if struct.unpack('<5I',path.read_bytes())!=probe:raise RuntimeError('All three documented writeback patches required')
 size=18432*1204;source=(templates/'element-lfo-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(size))
 result=execute('element246',source,linker='element-lfo.ld',extra=('--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000'))
 if result.stat().st_size!=size:raise RuntimeError('Incomplete output')
 raw_input=(out/'inputs.bin').read_bytes();raw_output=result.read_bytes();coverage={}
 for n in range(18432):
  family,type,kind=struct.unpack_from('<3I',raw_input,n*444);dest=struct.unpack_from('<h',raw_input,n*444+20)[0]//2048
  if struct.unpack_from('<I',raw_output,n*1204+36)[0]:
   key=f'{family}:{type}:{kind}:{dest}';coverage[key]=coverage.get(key,0)+1
 for family,types,kinds,dest in [(0,[0,1,2,3,4,6,7,11,12,13],[0],0),(0,[0,1,2,3,4,6,7,11,12,13],[1],1),(1,[8,9],[0,1,2,3],0),(2,list(range(8)),[2,5],2)]:
  for type in types:
   for kind in kinds:
    if not coverage.get(f'{family}:{type}:{kind}:{dest}'):raise RuntimeError('Missing accepted matrix case')
 # Gzip contains numerical observations only, never executable firmware bytes.
 import gzip
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'Tests/ReferenceVectors'/f'element246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'factory_index_zero_based':3,'factory_wrapper':'0x019c9a88','callbacks':{'synth':'0x019cda14','drum':'0x019cde28','fx':'0x019cdf22'},'model':'BF524 simulator, three verified writeback patches; not hardware','calls':18432,'calls_per_family':6144,'accepted_matrix_counts':coverage,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Explicit sensor/external/gain/envelope/level inputs; bounded parameter dispatch and unchanged complete source/pitch/gain/FX buffers.','Only parameter destinations 0/1/2 and Off, first four ADSR; no audio/pitch routes or FX kind 4.','No complete voice, protected gain producer or acoustic equivalence claim.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 18432 original bounded Element calls, explicit source inputs and parameter dispatch')
if __name__=='__main__':main()
