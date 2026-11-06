#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce the complete original Core 64-word signal callback."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4864
STRIDE=332
OUT=328
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x43534947
 edges=[0,0x00a3d700,1,0xffffffff,0x7fffffff,0x80000000,0x40000000,0xc0000000,0x10000,0xffff0000]
 def rand():
  nonlocal seed
  seed=(seed*1664525+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  coef=edges[n%len(edges)] if n<512 or n>=4096 else rand()
  state=[rand() for _ in range(7)]
  if n<64:state[2:5]=[edges[n%len(edges)],edges[(n//2)%len(edges)],edges[(n//3)%len(edges)]]
  signal=[]
  for i in range(64):
   raw=rand()>>16
   if n%8==0:raw=0
   elif n%8==1:raw=32767 if i==0 else 0
   elif n%8==2:raw=0x8000 if i==0 else 0
   elif n%8==3:raw=32767 if i%2 else 0x8000
   elif n%8==4:raw=16384
   elif n%8==5:raw=(i*1024-32768)&65535
   signal.append(raw)
  data+=struct.pack('<I3I7I2I64H2I36I',(1 if n<4096 else 0 if n<4608 else 2),rand(),coef,rand(),*state,rand(),rand(),*signal,rand(),rand(),*[rand() for _ in range(36)])
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
 source=(templates/'core-signal-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('core-signal246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original Core signal output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/core-signal/vectors'/f'core-signal246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'independent':4096,'retained':512,'initializer_composed':256,'samples':CASES*64,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'routine':{'entry':'0x019674f8','end_exclusive':'0x01967614','sha256':hashlib.sha256(image[0x19674f8-base:0x1967614-base]).hexdigest()},'coefficient':'0x01bab53c','state':'0x01aaac14','input':'0xff801e50','output':'0xff800bb4','artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Complete original callback with explicitly supplied coefficient, three history words, input buffer and call order; all buffers/guards captured.','No recovered input producer, original caller, clock/sample rate, semantic role of buffers or routing to drum voices; no per-sample drum pitch/gain producer recovered.','Research-only; no plugin or installer generation.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 4864 complete original Core 64-word callbacks, 311296 numeric output samples')
if __name__=='__main__':main()
