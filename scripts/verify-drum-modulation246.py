#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Execute isolated drum Value/Random callbacks with supplied private rev246 firmware.
Only synthetic inputs and numerical observations may be recorded in the checkout.
"""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(name,path):
 spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def inputs(family,type):
 if family=='value':data=bytearray(module('value_data',ROOT/'scripts/verify-value-voice246.py').inputs((type,type)));stride,offset,meta=96,60,84
 else:data=bytearray(module('random_data',ROOT/'scripts/verify-random-voice246.py').inputs());stride,offset,meta=88,68,84
 seed=0x4452554d
 for case in range(4096):
  if family=='random':v=struct.unpack_from('<I',data,case*stride+meta)[0];struct.pack_into('<I',data,case*stride+meta,(v&65535)|(type<<16))
  if case%2==0:
   seed=(1664525*seed+1013904223)&0xffffffff;start=seed%15000;end=start+1+(seed>>16)%15000
   points=[16000,start,end,(case//2%3)*8192,8192,12345,23456,16384]
   if case%16==0:points[1]=points[2]
   if case%16==2:points[1:3]=[0,32767]
   if type==9:points=[int((seed>>(i%4*8))%32768) for i in range(8)]
   struct.pack_into('<8h',data,case*stride+offset,*points)
 return data

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware')
 mapper=module('ldr_mapper',ROOT/'scripts/map-op1-ldr.py');raw=a.ldr.read_bytes();meta,_=mapper.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,count,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+count<=base+len(image) and not flags&0x1000:image[address-base:address-base+count]=(struct.pack('<I',arg)*((count+3)//4))[:count] if flags&0x100 else raw[cursor+16:cursor+16+count]
  cursor+=16+(0 if flags&0x100 else count)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);templates=ROOT/'research/blackfin'
 def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=180,**kw)
 def execute(stem,source,linker='voice-contract.ld',extra=()):
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as stdout,(out/(stem+'.log')).open('wb') as stderr:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=stdout,stderr=stderr)
  return out/(stem+'.bin')
 for name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  path=execute(Path(name).stem,(templates/name).read_text(),'flat.ld')
  if struct.unpack('<5I',path.read_bytes())!=probe:raise RuntimeError('All three documented writeback patches required')
 artifacts={};counts={}
 for family in ['value','random']:
  for type in [8,9]:
   stem=f'{family}-drum{type}-246';(out/'inputs.bin').write_bytes(inputs(family,type))
   source=(templates/('value-voice-stream.s.in' if family=='value' else 'random-voice-stream.s.in')).read_text()
   source=source.replace('0x019cfaf8','0x019cfdac').replace('0x019cfd5a','0x019cff16').replace('0x019cea70','0x019cedc0').replace('0x019ced74','0x019cef66').replace('0xff8016e4','0xff801704')
   source=source.replace('0x01de58d0','0x01de5a90').replace('0x01de5a50','0x01de5ad0')
   setup='imm32 P0, 0x01a1e914\nCALL (P0);\n'
   for i in range(24):setup+=f'imm32 P0, {0x18bef50+i*4}\nR0 = [P0];\nimm32 P0, 0x01aa6fd0\nCALL (P0);\nimm32 P0, {0x106a8f4+i*4}\n[P0] = R0;\n'
   size=4096*(68 if family=='value' else 72);source=source.replace('@SETUP@',setup).replace('@WORK@',out.as_posix()).replace('@BYTES@',str(size))
   result=execute(stem,source,extra=('--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000'))
   prefix=152 if family=='value' else 0;stride=68 if family=='value' else 72
   if result.stat().st_size!=prefix+size:raise RuntimeError('Incomplete output: '+stem)
   accepted=sum(struct.unpack_from('<I',result.read_bytes(),prefix+i*stride+stride-(8 if family=='value' else 4))[0]==1 for i in range(4096));counts[stem]={'accepted':accepted,'rejected':4096-accepted}
   for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
    target=ROOT/'Tests/ReferenceVectors'/f'{stem}-{suffix}.bin'
    if a.record:target.write_bytes(path.read_bytes())
    if target.read_bytes()!=path.read_bytes():raise RuntimeError('Fixture differs: '+target.name)
    artifacts[target.name]={'bytes':path.stat().st_size,'sha256':sha(path)}
 report={'ldr_sha256':expected,'model':'GNU 13.1 BF524 with three verified writeback patches, not hardware','callbacks':{'value':'0x019cfdac','value_reset':'0x019cff16','random':'0x019cedc0','random_reset':'0x019cef66','helper':'0x01a1e57c'},'types':[8,9],'calls_per_callback_and_type':4096,'total_calls':16384,'counts':counts,'range_prefix':['0x01de5a90','0x01de5ad0'],'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Isolated drum dispatch under supplied globals, target words, clock and state.','No original gain producer, scheduler, PCM source mapping, complete voice or acoustic fidelity claim.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 16384 original drum Value/Random calls, types 8/9; numerical observations recorded/reproduced, C++ comparison is separate')
if __name__=='__main__':main()
