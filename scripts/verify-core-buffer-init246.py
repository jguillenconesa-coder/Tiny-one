#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce shared buffer initialization and explicit Core kernel composition."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=1024
STRIDE=1764
OUT=1760
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x43424932
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  flags=(1 if n<512 or n%32==0 else 0)|(2 if n%4 in (0,1) else 0)|(4 if n%4 in (0,2) else 0)
  values=[rand() for _ in range(440)]
  if n%8==0:values=[0]*440
  elif n%8==1:values=[0xffffffff]*440
  elif n%8==2:values=[0xa5a5a5a5]*440
  values[402]=[0,0x00a3d700,0x7fffffff,0x80000000,0xffffffff,0x40000000][n%6]
  data+=struct.pack('<441I',flags,*values)
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
 if struct.unpack_from('<I',image,0x33c)[0]!=0x01a3b34c:raise RuntimeError('Buffer initializer startup pointer differs')
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs());templates=ROOT/'research/blackfin'
 def execute(stem,source,linker='flat.ld',extra=()):
  def run(cmd,**kw):subprocess.run(list(map(str,cmd)),check=True,timeout=300,**kw)
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as f,(out/(stem+'.log')).open('wb') as log:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=f,stderr=log)
  return out/(stem+'.bin')
 for name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  if struct.unpack('<5I',execute(Path(name).stem,(templates/name).read_text()).read_bytes())!=probe:raise RuntimeError('All three verified writeback patches required')
 source=(templates/'core-buffer-init.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('core-buffer-init246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original Core signal output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/core-buffer-init/vectors'/f'core-buffer-init246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'startup_pointer':{'address':'0x0100033c','value':'0x01a3b34c'},'initializer_sha256':hashlib.sha256(image[0x1a3b34c-base:0x1a3b3dc-base]).hexdigest(),'routines':{'buffer_initializer':'0x01a3b34c','vector_constructor':'0x01aa3862','zero_word':'0x01a3b308','zero_halfword':'0x01a3b310','signal_kernel':'0x019674f8'},'windows':[['0x01df4dec',24],['0xff801948',1552],['0x01aaac0c',28],['0x01bab538',12],['0xff800bac',144]],'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Complete buffer initializer and optional Core signal kernel under supplied state and call order; windows include guards.','Zero initial buffers do not establish active gain/pitch production, scheduler, rates or whole-machine reset. Core kernel history is separate.','Research-only; no plugin changes or installers.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 1024 original Core buffer initialization/kernel scenarios')
if __name__=='__main__':main()
