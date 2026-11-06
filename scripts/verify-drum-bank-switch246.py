#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce complete drum bank changes with authored L1 service and real factories."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=2048
STRIDE=8828
OUT=8928
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x44535732
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  family=n%7;target=n//7%8;old=n//56%8;oldType=family if old==target else (family+old+1)%7
  ui=1 if n<448 or n%8 else 2;held=n%25;delay=1+n%4;flags=1 if n<1024 or n%32==1 else 0
  image=bytearray(struct.pack('<2199I',*[rand() for _ in range(2199)]))
  if n%4<3:image=bytearray(bytes([[0,255,165][n%4]])*8796)
  def put(offset,value):struct.pack_into('<I',image,offset,value&0xffffffff)
  if ui==2:image[8704]=165
  for v in range(12):struct.pack_into('<5I',image,224+v*20,0x0190fac4,0x02010000+old*8,(16+v)*2048,(n+v)%3,(0x80000000+n*1009+v*7919)&0xffffffff)
  put(4,old);put(2032,old);put(8716,ui);put(4816,1);put(4812,held)
  for bank in range(8):
   image[44+bank]=[0,1,165][(bank+n)%3];put(1164+48+bank*4,(family+bank)%7);put(2188+bank*4,0x02010000+bank*8)
  put(1164+48+old*4,oldType);put(1164+48+target*4,family)
  for i in range(held):
   for j,value in enumerate([53+(n+i)%24,rand(),rand(),rand(),(n+i*5)%12]):put(2252+i*20+j*4,value)
  for v in range(12):image[4872+v]=165 if (n*37)&(1<<v) else 0;put(5580+v*4,(n+v*7)%24)
  for bank in range(8):
   for pad in range(24):
    c=[(n+pad)%24*500,pad%6,32+pad%8,4096+(pad+bank)%3*8192,8192+(pad+bank)%2*16384,rand()%32768,rand()%32768,[8192,16384,32767][pad%3]]
    struct.pack_into('<8h',image,5628+bank*384+pad*16,*c)
  data+=struct.pack('<8I',flags,ui,old,target,family,oldType,held,delay)+image
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
 source=(templates/'drum-bank-switch.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('drum-bank-switch246',source,'drum-bank-switch.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--memory-region','0xffa0bd2c,0x20','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete drum bank change output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/drum-bank-switch/vectors'/f'drum-bank-switch246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'observed_window_bytes':8796,'routine':{'entry':'0x01a86420','end_exclusive':'0x01a86660','sha256':hashlib.sha256(image[0x1a86420-base:0x1a86660-base]).hexdigest()},'profile':{'ui_modes':[1,2],'audio_type':8,'modulator_types':list(range(7)),'held_count_max':24},'authored_L1_endpoint':'0xffa0bd2c','windows':[('0x1aaac0c', 68), ('0x1aaad18', 156), ('0xffb00000', 256), ('0xffb005f8', 48), ('0xffb009f8', 588), ('0xff907aa8', 48), ('0xff801374', 1024), ('0x1cc10cc', 32), ('0x1df6f78', 32), ('0x1dfe734', 2564), ('0xff801038', 812), ('0x1aaadb4', 3072), ('0x1ab3148', 12), ('0x1de4c3c', 12), ('0x1dd5a88', 32), ('0x1dd5d04', 32), ('0x1dd5c6c', 4), ('0xff802bc8', 4)],'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Complete original bank changer in UI drum mode 1 and no-op mode 2, original wait loop, setter, factories, modulator/reader resets. No synth mode-0 branch exercised.','L1 service is authored: delays one to four calls, writes actual bank, invokes original drum audio and modulator factories. Its service order and completion are not recovered L1 behavior.','Opaque nonzero references and valid supplied rows/pad maps/controls; no PCM rendering, normal scheduler, gain/pitch producer or acoustic recovery. No plugin changes or installers.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 2048 original drum bank-change scenarios with explicit L1 reconstruction')
if __name__=='__main__':main()
