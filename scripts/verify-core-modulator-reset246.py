#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce Core modulation reset dispatchers, clears and five real reset endpoints."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=8192
STRIDE=740
OUT=732
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x4d4f4452
 def rand():
  nonlocal seed
  seed=(seed*1664525+1013904223)&0xffffffff;return seed
 deltas=[-16,-8,0,8,16]
 for n in range(CASES):
  route=n%2;i=n//2;slot=(i//6)%(12 if route else 6);sound=(i//72)%8;primary=i%6;secondary=(i//6+i//72)%6;actions=int(n%17==0)|(2 if n%5==0 else 0);new_sound=(sound+1+n%7)%8
  payload=bytearray(struct.pack('<176I',*[rand() for _ in range(176)]))
  for bank in range(8):
   payload[28+bank]=[0,1,127,255][(n//7+bank)%4];payload[44+bank]=[0,1,127,255][(n//7+bank)%4]
  if actions&2:payload[(44 if route else 28)+new_sound]=0
  for bank in range(16):
   struct.pack_into('<h',payload,440+bank*16+4,[0,2048,-1][(n+bank)%3]);struct.pack_into('<h',payload,440+bank*16+6,[14744,14745,16384,-32768,32767][(n+bank)%5])
  data+=struct.pack('<9I',route,slot,sound,primary,secondary,deltas[n%5]&0xffffffff,deltas[(n//5)%5]&0xffffffff,actions,new_sound)+payload
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
 source=(templates/'core-modulator-reset.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('core-modulator-reset246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original Core modulator reset output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/core-modulator-reset/vectors'/f'core-modulator-reset246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'dispatchers':{'synth':'0x019673dc','drum':'0x0196735e'},'clears':{'synth':'0x0196745a','drum':'0x01967330'},'resets':['0x019cfd5a','0x019cff16','0x019ced74','0x019cef66','0x019cbdca'],'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Complete original six/twelve-slot dispatchers and clears with explicit pointer tables, selected sounds, enable bytes, valid indices and controls.','Authored tracing endpoints preserve R0/R1 and tail-call the five complete original Value/Random synth/drum and Tremolo resets. Signed this adjustments and synthetic selector mutation are supplied; no factory/type-choice claim.','No audio render, modulation-step cadence, physical inputs, pitch/gain buffer producers or original full note-on composition claimed; research-only.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 8192 complete Core reset dispatch cases with five original reset callbacks and clear routes')
if __name__=='__main__':main()
