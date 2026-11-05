#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded Core drum-sampler factory/reset/release/render contracts."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4608
STRIDE=1096
OUT=5156
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 seed=0x4344524d
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 data=bytearray()
 for bank in range(8):
  pcm=[((i*(17+bank*6)+bank*3271)%65536)-32768 if bank%3==0 else (32767 if i%37==0 else -32768 if i%53==0 else 0) if bank%3==1 else int(rand()%65536)-32768 for i in range(1024)]
  data+=struct.pack('<1056h',*([0]*16),*pcm,*([0]*16))
 controls=[[(p%8)*2, p%6,32+p%8,((p+b)%3)*8192,((p+b)%2)*16384,0,12345,32767] for b in range(8) for p in range(24)]
 data+=b''.join(struct.pack('<8h',*c) for c in controls)
 for n in range(CASES):
  voice=n%12;bank=n//12%8;pad=n//96%24;factory=(bank+1+n//192)%8
  flags=(1 if n<256 or n%96==0 else 0)|(2 if n%29==0 else 0)|(4 if n<2304 or n%11==0 else 0)|(8 if n>=2304 or n%11==0 else 0)|(16 if n%4 else 0)
  adj=[-32768,-1024,-1,0,1,1024,32767][n%7];other=[32767,32,0,-32,-32768][n%5]
  pads=[(pad+(v-voice)*5)%24 for v in range(12)]
  pool=[]
  for v in range(12):pool.extend([0x0190fac4,0x02010000+(n+v)%8*8,(32+rand()%700)*2048,rand()%3,rand()])
  c=[rand()%32768,pad%6,32+pad%8,(n%3)*8192,((n//3)%2)*16384,rand()%32768,rand()%32768,[0,1,8192,16384,32767][n%5]]
  pitch=[[0,0x04000000,0x08000000,0x10000000,0x7fffffff][(n+i//16)%5] for i in range(128)]
  gain=[int(rand()%65536)-32768 for _ in range(128)]
  data+=struct.pack('<6I12I60I8h128I128h',flags,voice,bank,factory,adj&0xffffffff,other&0xffffffff,*pads,*pool,*c,*pitch,*gain)
 assert len(data)==19968+CASES*STRIDE
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
 setup=''
 for i in range(1025):setup+=f'imm32 P0,{0x018bf010+i*4}\nR0=[P0];\nimm32 P0,0x01aa7014\nCALL(P0);\nimm32 P0,{0xff802154+i*2}\nW[P0]=R0.L;\n'
 setup+='R0.H=coeff_args;\nR0.L=coeff_args;\nP0=5;\nEXCPT 0;\n'
 handles='.section .handles,"aw"\n'+''.join(f'.long {0x02010004+i*8},samples+{i*2112+32}\n' for i in range(8))+'.data\n'
 source=(templates/'drum-core-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@SETUP@',setup).replace('@HANDLES@',handles).replace('@BYTES@',str(CASES*OUT))
 result=execute('drum-core246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=2052+CASES*OUT+16896:raise RuntimeError('Incomplete Core drum output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/drum-core/vectors'/f'drum-core246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_prefix_bytes':19968,'input_record_bytes':STRIDE,'output_prefix_bytes':2052,'output_record_bytes':OUT,'output_suffix_bytes':16896,'core_factory':'0x01967984','factory_helper':'0x01a1b9c0','core_reset':'0x01967ccc','core_release':'0x01967d64','producer_table':'0x01aaad30','selected_sound':'0x01aaac10','voice_pad_table':'0xff801334','control_banks':'0x01aaadb4','sample_references':'0x01cc10cc','type':8,'voices':12,'sounds':8,'pads':24,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Original Core dispatch traced using authored spy endpoints and signed this adjustments; the same routes then execute original sampler reset/release and optional render.','Valid indices only; explicit pads, sample references, controls, PCM padding, pitch/gain and event order. No gain-envelope producer, MIDI allocator, original scheduling or acoustic equivalence.','Research-only, no plugin integration or installer build.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 4608 original Core drum cases, full routing grid, real lifecycle and optional original render')
if __name__=='__main__':main()
