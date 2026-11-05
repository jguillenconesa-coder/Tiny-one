#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded Bend/Crank FX audio observations using privately supplied rev246."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(name):
 spec=importlib.util.spec_from_file_location(name,ROOT/'scripts'/('verify-'+name+'-voice246.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def inputs(name):
 stride=96 if name=='bend' else 128;cpos=64 if name=='bend' else 96
 records=bytearray((module(name).inputs()*2)[:8192*stride])
 for case in range(8192):
  off=case*stride
  struct.pack_into('<4I',records,off,2,case%8,(0 if case%19==0 else 4 if case%23==0 else [2,5][case//128%2]),1|(2 if case%5==0 else 0)|(4 if case%193==0 else 0))
  if name=='bend':
   struct.pack_into('<I',records,off+20,case%128)
   # Retain polled source on most calls; selected calls advance it.
   struct.pack_into('<I',records,off+60,0x5a010000|(case&65535))
  else:
   struct.pack_into('<I',records,off+20,0xa5b60101)
   struct.pack_into('<I',records,off+28,case%256)
  colour=[2,3,1,4][case//2048%4];destination=-1 if case%17==0 else 3
  amount=[-32767,-16384,-1,0,1,16384,32767,12345][case//256%8]
  controls=list(struct.unpack_from('<8h',records,off+cpos))
  if name=='bend':controls[:3]=[colour*4096,destination*2048,amount]
  else:controls[1:4]=[amount,destination*2048,colour*4096]
  struct.pack_into('<8h',records,off+cpos,*controls)
 return records

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--record',action='store_true');p.add_argument('--source',choices=['bend','crank'],required=True);a=p.parse_args();name=a.source;stride=96 if name=='bend' else 128;output_stride=1108 if name=='bend' else 1140;return_offset=80 if name=='bend' else 112;control_offset=64 if name=='bend' else 96;out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,count,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+count<=base+len(image) and not flags&0x1000:image[address-base:address-base+count]=(struct.pack('<I',arg)*((count+3)//4))[:count] if flags&0x100 else raw[cursor+16:cursor+16+count]
  cursor+=16+(0 if flags&0x100 else count)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs(name));templates=ROOT/'research/blackfin'
 def execute(stem,source,linker='voice-contract.ld',extra=()):
  def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=300,**kw)
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as stdout,(out/(stem+'.log')).open('wb') as stderr:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=stdout,stderr=stderr)
  return out/(stem+'.bin')
 for probe_name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  path=execute(Path(probe_name).stem,(templates/probe_name).read_text(),'flat.ld')
  if struct.unpack('<5I',path.read_bytes())!=probe:raise RuntimeError('All three documented writeback patches required')
 size=8192*output_stride;source=(templates/(name+'-fx-stream.s.in')).read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(size))
 result=execute(name+'-fx246',source,linker='crank-lfo.ld',extra=('--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000'))
 if result.stat().st_size!=size:raise RuntimeError('Incomplete output')
 raw_input=(out/'inputs.bin').read_bytes();raw_output=result.read_bytes();coverage={}
 for n in range(8192):
  family,type,kind=struct.unpack_from('<3I',raw_input,n*stride);c=struct.unpack_from('<8h',raw_input,n*stride+control_offset);colour=c[0 if name=='bend' else 3]//4096
  if struct.unpack_from('<I',raw_output,n*output_stride+return_offset)[0]:
   key=f'{type}:{kind}:{colour}';coverage[key]=coverage.get(key,0)+1
 # Gzip contains numerical observations only, never executable firmware bytes.
 import gzip
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'Tests/ReferenceVectors'/f'{name}-fx246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'source':name,'callback':'0x019ca328' if name=='bend' else '0x019cb3d4','model':'BF524 simulator, three verified writeback patches; not hardware','calls':8192,'accepted_matrix_counts':coverage,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['FX callback kinds 2/5 destination 3 colours 2/3 with supplied source state and complete buffers. Other kinds/selectors rejected.','Host scheduling and input routing not recovered; no hardware acoustic equivalence.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS:',name,'8192 original FX calls, source state and complete buffers')
if __name__=='__main__':main()
