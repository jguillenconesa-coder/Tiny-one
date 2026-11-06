#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce original selection synchronizers under authored L1 response schedules."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4096
STRIDE=308
OUT=392
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x53594e32
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 modes=[0,1,2,0xffffffff,0x80000000]
 for n in range(CASES):
  route=n%2;count=n%7;actual=[n%8,n//8%8];edit=[(n//16)%8,(n//128)%8];edit[route]=(actual[route]+(1 if count else 0))%8;mode=modes[n%5]
  image=struct.pack('<33I',*[rand() for _ in range(33)])
  schedule=[]
  for step in range(6):
   nextEdit=[(edit[0]+step+1)%8,(edit[1]+2*step+1)%8]
   if n%3==0:nextEdit=edit.copy()
   nextActual=[(actual[0]+step+3)%8,(actual[1]+step+5)%8]
   nextActual[route]=nextEdit[route] if step+1==count else (nextEdit[route]+1+step%5)%8
   schedule.extend([*nextActual,*nextEdit,(mode if n%4<2 else modes[(n+step+1)%5]),rand()])
  data+=struct.pack('<8I',route,count,mode,*actual,*edit,rand())+image+struct.pack('<36I',*schedule)
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
 source=(templates/'core-selection-sync.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('core-selection-sync246',source,'core-selection-sync.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--memory-region','0xffa0bd2c,0x20','--memory-region','0xffa017ec,0x20','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original Core signal output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/core-selection-sync/vectors'/f'core-selection-sync246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'routines':{'synth_sync':'0x01967f68','drum_sync':'0x01967e90'},'authored_L1_endpoints':['0xffa0bd2c','0xffa017ec'],'windows':[['0x01aaac0c',68],['0xff8016cc',32],['0xff801038',32]],'max_authored_callbacks_per_case':6,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Complete original outer synchronizers; both L1 entries are authored schedule endpoints, not original code or recovered reactivation.','Explicit finite callback schedules model bank/mode/selection changes and R2 clobbering. Original loops have no demonstrated timeout; own harness/native budgets are not original behavior.','No audio/FX/modulator reconstruction, rendering, host integration, physical scheduling or gain/pitch producer recovery. No installers.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 4096 original selection synchronizer scenarios, authored L1 responses')
if __name__=='__main__':main()
