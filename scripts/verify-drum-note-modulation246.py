#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce complete drum note-on with active modulator resets and sampler reset."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4096
STRIDE=772
OUT=7624
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 def module(name,file):
  spec=importlib.util.spec_from_file_location(name,ROOT/file);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
 source=module('resets','scripts/verify-core-modulator-reset246.py').inputs();data=bytearray(module('events','scripts/verify-drum-event246.py').inputs()[:3072]);seed=0x4e4f5445
 def rand():
  nonlocal seed
  seed=(seed*1664525+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  record=bytearray(source[(n*2+1)*740:(n*2+2)*740]);voice=n%12;sound=n//12%8;pad=n//96%24;primary=(n//12+n//96+n//576)%6;secondary=(n//18+n%17)%6;actions=int(n%17==0)|(2 if n%5==0 else 0);new_sound=(sound+1+n%7)%8
  struct.pack_into('<5I',record,0,1,voice,sound,primary,secondary);struct.pack_into('<2I',record,28,actions,new_sound)
  for bank in range(8):record[36+44+bank]=[0,1,127,255][(n//7+bank)%4]
  if actions&2:record[36+44+new_sound]=0
  note=53+pad;flags=0
  if n>=2304:
   if n%29==0:note=52
   elif n%31==0:note=77
   flags=int(n%11==0)|(2 if n%7==0 else 0)
  preview=0xffffffff if n%3==0 else pad if n%3==1 else (pad+1)%24
  data+=record+struct.pack('<8I',((n*73+37)%256-128)&0xffffffff,note,rand(),rand(),rand(),preview,flags,rand())
 assert len(data)==3072+CASES*STRIDE
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
 source=(templates/'drum-note-modulation.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('drum-note-modulation246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original drum note modulation output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/drum-note-modulation/vectors'/f'drum-note-modulation246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_prefix_bytes':3072,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'note_on':'0x01a84ba0','modulator_dispatch':'0x0196735e','sampler_reset':'0x01978926','query':'0x019788b0','artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Complete original note-on, active reset dispatcher and original modulator/sampler resets; explicit control banks, type-8 factory, references, occupancy/list, event order and normal register context.','Authored argument/order endpoints tail-call complete original reset bodies. Signed modulator this adjustments, own audio tracing vtable and synthetic selection mutation; no factory/type-selection mapping claim.','No audio render, per-sample pitch/gain producers, original scheduler, physical inputs, plugin integration or installer generation.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 4096 complete original drum note-ons with active modulators, sampler reset and event-order observations')
if __name__=='__main__':main()
