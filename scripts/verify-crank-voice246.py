#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded Crank modulation observations using privately supplied rev246."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 seed=0x4352414e;data=bytearray()
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for case in range(18432):
  family=1 if case%3==2 else 0;types=[0,1,2,3,4,6,7,11,12,13] if family==0 else [8,9];frame=case//3;type=types[frame%len(types)];kinds=4 if family<2 else 2;kind=(frame//len(types))%4 if family<2 else [2,5][frame//len(types)%2]
  action=(1 if case<3072 and case%384<3 or case==3072 else 0)|(2 if family==0 else 0)|(4 if case<3072 and case%193==0 else 0)
  words=[0xa5b60000|((rand()&1)<<8)|(1 if case%7==0 else 0),[0,48,49,99,0xfffffffe][case//384%5],rand()%256,int(rand()%241)-120,rand()%10,int(rand()%241)-120,rand()%32]
  history=[int(rand()%257)-128 for _ in range(10)];previous=rand()%256;gain=rand()
  if action&1:words[2]=[0,1,63,64,65,127,128,191,192,254,255][(case//384+case%3)%11]
  # Long stream accelerates in both directions then stays quiet long enough to stop.
  phase=frame%256 if frame<1024 else ((frame-1024)*7)%256 if frame<1536 else (256-((frame-1536)*9)%256)%256 if frame<2048 else 72
  angle=(phase-128)*64
  destination=3;colour=(frame//(len(types)*4))%3;amount=int(rand()%65535)-32767
  if case%8==0:amount=[-32767,-16384,-1,0,1,16384,32767][case//8%7]
  controls=[angle,amount,destination*2048,colour*4096,int(rand()%32768),1234,2345,3456]
  targets=[int(rand()%32768) for _ in range(8)]
  data+=struct.pack('<5I7I10iiI8h8h',family,type,kind,action,rand(),*[w&0xffffffff for w in words],*history,previous,gain,*controls,*targets)
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
 # Confirm the seventh factory entry (zero-based index 6) and callback vtables.
 if struct.unpack_from('<I',image,0x018e1358-base+6*4)[0]!=0x019c9ab2:raise RuntimeError('Unexpected Crank factory selector')
 for address,callback in [(0x019107a0,0x019cb0d4),(0x019107c0,0x019cb30c),(0x019107e0,0x019cb3d4)]:
  if struct.unpack_from('<I',image,address-base)[0]!=callback:raise RuntimeError('Unexpected Crank vtable')
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs());templates=ROOT/'research/blackfin'
 def execute(stem,source,linker='voice-contract.ld',extra=()):
  def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=300,**kw)
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as stdout,(out/(stem+'.log')).open('wb') as stderr:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=stdout,stderr=stderr)
  return out/(stem+'.bin')
 for name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  path=execute(Path(name).stem,(templates/name).read_text(),'flat.ld')
  if struct.unpack('<5I',path.read_bytes())!=probe:raise RuntimeError('All three documented writeback patches required')
 size=18432*1140;source=(templates/'crank-voice-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(size))
 result=execute('crank-voice246',source,linker='crank-lfo.ld',extra=('--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000'))
 if result.stat().st_size!=size:raise RuntimeError('Incomplete output')
 raw_input=(out/'inputs.bin').read_bytes();raw_output=result.read_bytes();coverage={}
 for n in range(18432):
  family,type,kind=struct.unpack_from('<3I',raw_input,n*128);dest=struct.unpack_from('<h',raw_input,n*128+100)[0]//2048
  if struct.unpack_from('<I',raw_output,n*1140+112)[0]:
   key=f'{family}:{type}:{kind}:{dest}';coverage[key]=coverage.get(key,0)+1
 # Gzip contains numerical observations only, never executable firmware bytes.
 import gzip
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'Tests/ReferenceVectors'/f'crank-voice246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'factory_index_zero_based':6,'factory_wrapper':'0x019c9ab2','callbacks':{'synth':'0x019cb0d4','drum_rejection':'0x019cb30c'},'model':'BF524 simulator, three verified writeback patches; not hardware','calls':18432,'families':'12288 synth and 6144 drum rejection calls','accepted_matrix_counts':coverage,'source_prepare':'0x019cae18','source_reset':'0x019cae08','source_state':'0xff907acc','gain_state':'0x01dd59bc','shared_history':'0xff907ae8','previous_angle':'0x01dd5a54','artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Interleaved source state, ring history, flags, counters and control/display words under supplied manual encoder inputs; gain/pitch transforms and complete buffers; input producers are supplied.','Only synth kind 0 destination 3 targets 0/1; no FX audio or auxiliary ENV.','No complete voice, protected gain producer or acoustic equivalence claim.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 18432 original Crank voice calls, full source/gain state and buffers')
if __name__=='__main__':main()
