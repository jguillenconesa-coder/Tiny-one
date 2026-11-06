#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce original Core initialization and explicit drum factory/note-on composition."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=256
STRIDE=5880
OUT=11736
SNAPSHOT=5856
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 spec=importlib.util.spec_from_file_location('events',ROOT/'scripts/verify-drum-event246.py');events=importlib.util.module_from_spec(spec);spec.loader.exec_module(events)
 data=bytearray(events.inputs()[:3072]);seed=0x434f5245
 def rand():
  nonlocal seed
  seed=(seed*1664525+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  for i in range(SNAPSHOT//4):data+=struct.pack('<I',rand() if n>2 else [0,0xa5a5a5a5,0xffffffff][n])
  data+=struct.pack('<6I',n//24%8,((n*73+37)%256-128)&0xffffffff,53+n%24,rand(),rand(),rand())
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
 source=(templates/'core-initial-state.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('core-initial-state246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original Core initialization output')
 raw_result=result.read_bytes();raw_input=(out/'inputs.bin').read_bytes();writes=[];preserved=0
 for i in range(SNAPSHOT):
  column=raw_result[i::OUT]
  if len(set(column))==1:writes.append((i,column[0]))
  else:
   if column!=raw_input[3072+i::STRIDE]:raise RuntimeError('Nonconstant nonpreserved byte '+str(i))
   preserved+=1
 spans=[]
 for offset,value in writes:
  if spans and offset==spans[-1][0]+spans[-1][1] and value==spans[-1][2]:spans[-1][1]+=1
  else:spans.append([offset,1,value])
 table='// SPDX-License-Identifier: AGPL-3.0-only\n// Derived default writes within four observed windows; not a firmware ROM table.\n#pragma once\n#include <array>\n#include <cstdint>\nnamespace pocket::research {\nstruct CoreInitialWrite246 {unsigned offset,count;uint8_t value;};\ninline constexpr std::array<CoreInitialWrite246,'+str(len(spans))+'> coreInitialWrites246{{\n'
 for offset,count,value in spans:table+='    {'+f'{offset},{count},{value}'+'},\n'
 table+='}};\ninline void coreInitialState246(uint8_t* image) noexcept {for(auto span:coreInitialWrites246)for(unsigned i=0;i<span.count;++i)image[span.offset+i]=span.value;}\n}\n'
 table_path=ROOT/'research/core-initial-state/CoreInitialState246.h'
 if a.record:table_path.write_text(table)
 if table_path.read_text()!=table:raise RuntimeError('Derived default writes differ')
 if struct.unpack_from('<I',image,0x0100010c-base)[0]!=0x01967fcc or struct.unpack_from('<I',image,0x01000110-base)[0]!=0x01968774:raise RuntimeError('Initializer pointer entries differ')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/core-initial-state/vectors'/f'core-initial-state246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_prefix_bytes':3072,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'initializer':{'entry':'0x01967fcc','end_exclusive':'0x01968766','sha256':hashlib.sha256(image[0x1967fcc-base:0x1968766-base]).hexdigest()},'shared_workspace_initializer':'0x01968774','initializer_pointer_references':['0x0100010c'],'shared_initializer_pointer_references':['0x01000110'],'windows':[{'address':hex(x),'bytes':n} for x,n in [(0xff800ff8,2024),(0x01aaabd4,3560),(0x01bab538,16),(0xffb00000,256)]],'constant_bytes':len(writes),'preserved_bytes':preserved,'derived_spans':len(spans),'derived_table_sha256':sha(table_path),'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Complete initializer under fixed firmware ROM inputs; observed four windows, not a whole-machine write trace or general soft-float port.','Then shared workspace initializer 0x01968774 and explicit control banks, selected sound, type-8 factory, drum mode 1, cleared occupancy/list, preview -1 and null LFO/FX before original note-on.','Object pool and guards are observed; no render or produced pitch/gain streams, scheduler, hardware or plugin integration.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 256 original Core initializations and complete factory/note-on chains; '+str(len(writes))+' constant / '+str(preserved)+' preserved observed bytes')
if __name__=='__main__':main()
