#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce complete six/twelve-voice modulator factories and real resets."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=3072
STRIDE=1088
OUT=1068
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x4d464132
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  drum=n//384%2;kind=n//128%3;voice=n%12 if drum else n%6;sound=n//12%8
  flags=11 if n<768 else (1 if n<1536 or n%32==1 else 0)|(2 if n%4!=3 else 0)|(4 if n%17==0 else 0)|(8 if n%5!=0 else 0)
  image=bytearray(struct.pack('<267I',*[rand() for _ in range(267)]))
  image[56:64]=bytes(8);image[68:152]=bytes(84)
  if n%4<3:image[152:740]=bytes([[0,255,165][n%4]])*588
  data+=struct.pack('<5I',flags,drum,kind,voice,sound)+image
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
 source=(templates/'core-modulator-factory.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('core-modulator-factory246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original Core signal output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/core-modulator-factory/vectors'/f'core-modulator-factory246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'routines':{'synth_factory':'0x01967a58','drum_factory':'0x01967ac0','shared_selector':'0x019c99ac','synth_selector':'0x019c9a24','drum_selector':'0x019c9ac0'},'types':[0,1,2],'voices':{'synth':6,'drum':12},'windows':[['0x01aaac0c',68],['0x01aaad60',84],['0xffb009f8',588],['0xff8014c4',128],['0xff801644',128],['0x01dd5d04',32],['0x01dd5a88',32],['0x01dd5c6c',4],['0xff802bc8',4]],'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Complete original factories/selectors/constructors and optional clear/reset dispatchers under explicit selection, enable flags and controls. Types 0..2 only.','Synth and drum constructors reuse the same physical primary pool and shared object; this is not simultaneous independent storage.','No original render scheduler, active pitch/gain producers, complete audio composition or plugin integration established here. No installers.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 3072 original Core modulator factory/clear/reset scenarios')
if __name__=='__main__':main()
