#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce original out-of-range note removal, MIDI producers and busy-pump queue."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=2048
STRIDE=8332
OUT=8424
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
WINDOWS=[(27962380, 68), (4286584524, 32), (4286582840, 32), (27996480, 16), (31450328, 40), (25911816, 4), (4286578900, 144), (28706108, 4), (31345728, 4), (31453492, 4), (28708592, 128), (27962648, 24), (27962732, 24), (4289724416, 224), (31450932, 2564), (27983196, 136), (31449148, 4), (4286582876, 24), (4286583508, 24), (4286589896, 8), (31450416, 512), (25960836, 4), (41933280, 4), (4286583748, 128), (31450928, 4), (27974432, 40), (25911308, 4), (27990840, 4120)]
def inputs():
 spec=importlib.util.spec_from_file_location('held_switch',ROOT/'scripts/verify-synth-held-bank-switch246.py');prior=importlib.util.module_from_spec(spec);spec.loader.exec_module(prior)
 base=prior.inputs();data=bytearray();seed=0x53524232
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  oldrow=base[n*4164:(n+1)*4164];image=bytearray(oldrow[8:]);image.extend(struct.pack('<1042I',*[rand() for _ in range(1042)]))
  if n%4<3:image[4204:8324]=bytes([[0,255,165][n%4]])*4120
  def put(o,v):struct.pack_into('<I',image,o,v&0xffffffff)
  old=struct.unpack_from('<I',image,76)[0];target=struct.unpack_from('<I',oldrow,0)[0];count=struct.unpack_from('<I',image,344)[0]
  put(100,0);put(4156,0)
  for bank in range(8):struct.pack_into('<h',image,348+bank*16+4,(n+bank)*97%32768)
  oldMode=struct.unpack_from('<h',image,348+old*16+8)[0]//4096
  if oldMode==4 and old!=target:struct.pack_into('<h',image,348+old*16+8,n%4096)
  for i in range(128):put(748+i*20,[0,1,11,12,60,115,116,126,127,(n+i)%128][(n+i)%10])
  put(3448,n//64%2);image[4160]=n%16;image[4201]=[0,1,165][n%3]
  ring=4204;read=[0,1000,0xfffffff0,0x7ffffff0][n%4];used=[0,1,1020,1022,1023][n//4%5]
  put(ring+4,read);put(ring+8,[1,2,165][n%3]);put(ring+4108,read+used);image[ring+4112]=[0,1,165][n//128%3]
  data+=oldrow[:8]+image
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
 source=(templates/'synth-range-bank-switch.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('synth-range-bank-switch246',source,'synth-range-bank-switch.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--memory-region','0xffa0bd2c,0x20','--memory-region','0xffa017ec,0x20','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original synth bank changer output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/synth-range-bank-switch/vectors'/f'synth-range-bank-switch246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'routine':{'entry':'0x01a86420','end_exclusive':'0x01a86660','sha256':hashlib.sha256(image[0x1a86420-base:0x1a86660-base]).hexdigest()},'windows':WINDOWS,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Original complete synth changer with supplied held registry, Dr. Wave type2, common selectors0..4; both L1 completions authored.','Both L1 endpoints authored: complete active-bank write after 1..4 calls, no factories or protected payload.','Original release/removal, MIDI producer and ring queue run; MIDI enable and transport gate supplied. DMA pump stays busy (>=1), no consumer or physical transport. No type4 module, auxiliary graph, PCM or plugin integration.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 2048 original range/removal/MIDI queue bank changes')
if __name__=='__main__':main()
