#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded Element modulation observations using privately supplied rev246."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 spec=importlib.util.spec_from_file_location('element_base',ROOT/'scripts/verify-element246.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 data=bytearray(m.inputs()[:8192*444])
 for case in range(8192):
  off=case*444;family=1 if case%7==0 else 0;type=([8,9][case%2] if family else [0,1,2,3,4,6,7,11,12,13][case%10]);source=case//10%4;colour=[0,2,3,4][case//40%4];kind=1 if case%19==0 else 2 if case%23==0 else 0;dest=-1 if case%17==0 else 3
  struct.pack_into('<4I',data,off,family,type,kind,[0x80008000,0x7fff7fff,0xffff0001,0x00008000,0x7fff8000,0x00010000,0x0000ffff,0x4000c000][case%8])
  struct.pack_into('<4h',data,off+16,source*2048,[-32768,-32767,-16384,-1,0,1,16384,32767][case//160%8],dest*2048,colour*4096)
 return data

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,count,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+count<=base+len(image) and not flags&0x1000:image[address-base:address-base+count]=(struct.pack('<I',arg)*((count+3)//4))[:count] if flags&0x100 else raw[cursor+16:cursor+16+count]
  cursor+=16+(0 if flags&0x100 else count)
 # Element is factory index 3; its original source file is LfoReRoute.
 if struct.unpack_from('<I',image,0x018e1358-base+3*4)[0]!=0x019c9a88:raise RuntimeError('Unexpected Element factory selector')
 for address,callback in [(0x019108b0,0x019cda14),(0x019108d0,0x019cde28),(0x019108f0,0x019cdf22)]:
  if struct.unpack_from('<I',image,address-base)[0]!=callback:raise RuntimeError('Unexpected Element vtable')
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs());templates=ROOT/'research/blackfin'
 def execute(stem,source,linker='voice-contract.ld',extra=()):
  def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=300,**kw)
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as stdout,(out/(stem+'.log')).open('wb') as stderr:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=stdout,stderr=stderr)
  return out/(stem+'.bin')
 for name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  path=execute(Path(name).stem,(templates/name).read_text(),'flat.ld')
  if struct.unpack('<5I',path.read_bytes())!=probe:raise RuntimeError('All three documented writeback patches required')
 size=8192*1204;source=(templates/'element-gain-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(size))
 result=execute('element-gain246',source,linker='element-lfo.ld',extra=('--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000'))
 if result.stat().st_size!=size:raise RuntimeError('Incomplete output')
 raw_input=(out/'inputs.bin').read_bytes();raw_output=result.read_bytes();coverage={};returns={}
 for n in range(8192):
  family,type,kind=struct.unpack_from('<3I',raw_input,n*444);c=struct.unpack_from('<8h',raw_input,n*444+16);source=c[0]//2048;dest=c[2]//2048;colour=c[3]//4096
  key=f'{source}:{type}:{kind}:{colour}'
  if family==0 and dest==3 and kind==0 and colour==0:coverage[key]=coverage.get(key,0)+1
  ret=struct.unpack_from('<I',raw_output,n*1204+36)[0];returns[str(ret)]=returns.get(str(ret),0)+1
 if len(coverage)!=40:raise RuntimeError('Missing accepted source/type/kind/curve matrix case')
 # Gzip contains numerical observations only, never executable firmware bytes.
 import gzip
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'Tests/ReferenceVectors'/f'element-gain246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'factory_index_zero_based':3,'callbacks':{'synth':'0x019cda14','drum_rejection':'0x019cde28'},'model':'BF524 simulator, three verified writeback patches; not hardware','calls':8192,'processed_matrix_counts':coverage,'return_value_counts':returns,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Synth gain kind 0 destination 3 colour 0; explicit sensor/external/gain/level inputs and complete buffers; drum and wrong route rejection.','Pitch colour 1 excluded; host producers, allocator, scheduler and external voice stream not recovered.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 8192 original Element gain audio calls and complete buffers')
if __name__=='__main__':main()
