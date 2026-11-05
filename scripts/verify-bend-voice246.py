#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded Bend modulation observations using privately supplied rev246."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 seed=0x42454e44;data=bytearray()
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for case in range(6144):
  family=1 if case%3==2 else 0;types=[0,1,2,3,4,6,7,11,12,13] if family==0 else [8,9];frame=case//3;type=types[frame%len(types)];kind=(frame//len(types))%4 if family<2 else [2,5][(frame//len(types))%2]
  # Interleaved synth/drum/FX calls share the polled flag, counter and input history.
  action=(1 if case<3072 and case%128==0 or case==3072 else 0)|(2 if case%3==0 else 0)|(4 if case<3072 and case%193==0 else 0)
  if case<3072 and case%128<3:action|=1
  current=rand()%128;initial=rand()&1;direction=case//256%2 if case<3072 else 1
  words=[current,0xa5b60000|initial,rand()%2,([0,8,9,19,0xfffffffe][case//128%5] if case%128<3 else rand()),rand()%256,rand()%128,rand()%61,direction]
  ring=[int(rand()%257)-128 for _ in range(2)];display=(0x5a000000|((case%7==0)<<16)|(rand()&65535))
  destination=3;colour=(frame//(len(types)*4))%3;unit=4096
  angle=((case//3*13)%256 if case<3456 and case%512<256 else 120)*64
  amount=int(rand()%65535)-32767
  if case%8==0:amount=[-32767,-16384,-1,0,1,16384,32767][case//8%7]
  controls=[colour*unit,destination*2048,amount,angle,int(rand()%32768),1234,2345,direction*2048]
  targets=[int(rand()%32768) for _ in range(8)]
  data+=struct.pack('<5I8I2iI8h8h',family,type,kind,action,rand(),*words,*ring,display,*controls,*targets)
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
 size=6144*1108;source=(templates/'bend-voice-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(size))
 result=execute('bend-voice246',source,extra=('--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000'))
 if result.stat().st_size!=size:raise RuntimeError('Incomplete output')
 raw_input=(out/'inputs.bin').read_bytes();raw_output=result.read_bytes();coverage={}
 for n in range(6144):
  family,type,kind=struct.unpack_from('<3I',raw_input,n*96);dest=struct.unpack_from('<h',raw_input,n*96+66)[0]//2048
  if struct.unpack_from('<I',raw_output,n*1108+80)[0]:
   key=f'{family}:{type}:{kind}:{dest}';coverage[key]=coverage.get(key,0)+1
 # Gzip contains numerical observations only, never executable firmware bytes.
 import gzip
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'Tests/ReferenceVectors'/f'bend-voice246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'callbacks':{'synth':'0x019c9d44','drum_rejection':'0x019ca0ec'},'model':'BF524 simulator, three verified writeback patches; not hardware','calls':6144,'families':'4096 synth and 2048 drum rejection calls','accepted_matrix_counts':coverage,'source_prepare':'0x019c9d04','source_reset':'0x019c9cf4','source_state':'0xff907aac','shared_history':'0x01dd59b4','shared_display_flag':'0x01dd591c','artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Interleaved source state, ring history, flags, counters and control/display words under supplied manual encoder inputs; gain/pitch transformations, complete buffers and state; no parameter routes in this oracle.','Only synth kind 0 destination 3 targets 0/1; drum and wrong kind/target rejection; no FX or physical source reader.','No complete voice, protected gain producer or acoustic equivalence claim.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 6144 original Bend voice calls, gain/pitch, shared source and full buffers')
if __name__=='__main__':main()
