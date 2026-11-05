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
  off=case*444;source=case//8%4;colour=[2,3,0,4][case//64%4];kind=0 if case%19==0 else 4 if case%23==0 else [2,5][case//32%2];dest=-1 if case%17==0 else 3
  struct.pack_into('<4I',data,off,2,case%8,kind,[0x80008000,0x7fff7fff,0xffff0001,0x00008000,0x7fff8000,0x00010000,0x0000ffff,0x4000c000][case%8])
  struct.pack_into('<4h',data,off+16,source*2048,[-32768,-32767,-16384,-1,0,1,16384,32767][case//256%8],dest*2048,colour*4096)
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
 size=8192*1204;source=(templates/'element-fx-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(size))
 result=execute('element-fx246',source,linker='element-lfo.ld',extra=('--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000'))
 if result.stat().st_size!=size:raise RuntimeError('Incomplete output')
 raw_input=(out/'inputs.bin').read_bytes();raw_output=result.read_bytes();coverage={};returns={}
 for n in range(8192):
  type,kind=struct.unpack_from('<2I',raw_input,n*444+4);c=struct.unpack_from('<8h',raw_input,n*444+16);source=c[0]//2048;dest=c[2]//2048;colour=c[3]//4096
  key=f'{source}:{type}:{kind}:{colour}'
  if dest==3 and kind in [2,5] and colour in [2,3]:coverage[key]=coverage.get(key,0)+1
  ret=struct.unpack_from('<I',raw_output,n*1204+36)[0];returns[str(ret)]=returns.get(str(ret),0)+1
 if len(coverage)!=128:raise RuntimeError('Missing accepted source/type/kind/curve matrix case')
 # Gzip contains numerical observations only, never executable firmware bytes.
 import gzip
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'Tests/ReferenceVectors'/f'element-fx246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'factory_index_zero_based':3,'callback':'0x019cdf22','model':'BF524 simulator, three verified writeback patches; not hardware','calls':8192,'processed_matrix_counts':coverage,'return_value_counts':returns,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Explicit sensor, external audio, envelope and level words, kinds 2/5 destination 3 colours 2/3; host producers and routing not recovered.','No voice gain/pitch or auxiliary kind-4 Envelope integration, no acoustic equivalence.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 8192 original Element FX audio calls and complete buffers')
if __name__=='__main__':main()
