#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce original synth bank changes with retained notes and authored L1 completion."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=2048
STRIDE=4164
OUT=4256
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
WINDOWS=[(27962380, 68), (4286584524, 32), (4286582840, 32), (27996480, 16), (31450328, 40), (25911816, 4), (4286578900, 144), (28706108, 4), (31345728, 4), (31453492, 4), (28708592, 128), (27962648, 24), (27962732, 24), (4289724416, 224), (31450932, 2564), (27983196, 136), (31449148, 4), (4286582876, 24), (4286583508, 24), (4286589896, 8), (31450416, 512), (25960836, 4), (41933280, 4), (4286583748, 128)]
def inputs():
 data=bytearray();seed=0x53484232
 table=(ROOT/'Tests/ReferenceVectors/drwave246-activation-output.bin').read_bytes()[:512]
 assert len(table)==512
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  image=bytearray(struct.pack('<1039I',*[rand() for _ in range(1039)]))
  def put(o,v):struct.pack_into('<I',image,o,v&0xffffffff)
  old=n%8;target=n//8%8;count=n%129;mode=n//64%5
  put(0,old);put(76,old);put(108,[0,1,2,0xffffffff][n//64%4])
  image[133]=[0,1,165][n//256%3];put(136,2);image[144]=165
  init=n//512%2;deferred=[0,1,165][n//128%3]
  put(148,init);image[152]=deferred
  for b in range(8):put(156+b*4,((b+n)%9)-4 if n%5 else 0)
  initialDeferred=deferred if init else image[133]
  delta=(struct.unpack_from('<i',image,156+target*4)[0]-struct.unpack_from('<i',image,156+old*4)[0])*12
  put(188,[0,0xffffffff,0x80000000,0x7fffffff][n%4]);put(336,1);put(340,0);put(344,count)
  for b in range(8):struct.pack_into('<h',image,348+b*16+8,((mode+b-target)%5)*4096+(n+b)%4096)
  for v in range(6):
   put(476+v*4,0xffb00000+v*36);put(500+v*4,0)
   struct.pack_into('<9I',image,524+v*36,0x0190fb2c,rand(),*[rand() for _ in range(5)],v,rand())
  for i in range(128):
   meta=[0,1,165][(n+i)%3];lo=max(0,-delta) if meta and not initialDeferred else 0;hi=min(127,127-delta) if meta and not initialDeferred else 127
   note=lo+(n+i*7)%(hi-lo+1);velocity=[0,1,127,128,255,32767,32768,65535][(n+i)%8]|(rand()&0xffff0000)
   if (n+i)%8==0:velocity=0 if ((n+i)//8)%2 else 0x10000
   explicit=[0,1,0x7fffffff,0x80000000,0xffffffff,rand()][(n+i)%6]
   row=748+i*20;put(row,note);put(row+4,velocity);image[row+8]=meta;put(row+12,explicit);put(row+16,(n+i)%8-2)
  put(3308,count)
  for i in range(128):image[3316+i]=[0,1,165][(n+i)%3]
  put(3448,0)
  for v in range(6):image[3452+v]=[0,1,165][(n+v)%3]
  image[3508:4020]=table
  put(4020,1200);put(4024,[0,1,0x7fffffff,0x80000000,0xffffffff,rand()][n%6])
  data+=struct.pack('<2I',target,1+n%4)+image
 assert len(data)==CASES*STRIDE
 return data
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,count,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+count<=base+len(image) and not flags&0x1000:image[address-base:address-base+count]=(struct.pack('<I',arg)*((count+3)//4))[:count] if flags&0x100 else raw[cursor+16:cursor+16+count]
  cursor+=16+(0 if flags&0x100 else count)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs());templates=ROOT/'research/blackfin'
 def execute(stem,source,linker='flat.ld',extra=()):
  def run(cmd,**kw):subprocess.run(list(map(str,cmd)),check=True,timeout=300,**kw)
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as f,(out/(stem+'.log')).open('wb') as log:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=f,stderr=log)
  return out/(stem+'.bin')
 for name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  if struct.unpack('<5I',execute(Path(name).stem,(templates/name).read_text()).read_bytes())!=probe:raise RuntimeError('All three verified writeback patches required')
 source=(templates/'synth-held-bank-switch.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('synth-held-bank-switch246',source,'synth-held-bank-switch.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--memory-region','0xffa0bd2c,0x20','--memory-region','0xffa017ec,0x20','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original synth bank changer output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/synth-held-bank-switch/vectors'/f'synth-held-bank-switch246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'routine':{'entry':'0x01a86420','end_exclusive':'0x01a86660','sha256':hashlib.sha256(image[0x1a86420-base:0x1a86660-base]).hexdigest()},'windows':WINDOWS,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Original complete synth changer with supplied held registry, Dr. Wave type2, common selectors0..4; both L1 completions authored.','Both L1 endpoints authored: complete active-bank write after 1..4 calls, no factories or protected payload.','Original MIDI note helpers run with output disabled; no invalid translated notes, mode-type4, PCM, scheduler, product integration or acoustic equivalence.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 2048 original synth held bank changes, authored L1 completion')
if __name__=='__main__':main()
