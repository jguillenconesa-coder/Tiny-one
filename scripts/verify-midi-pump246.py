#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce original MIDI enqueue/pump/completion/reset with a supplied busy endpoint."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4096
STRIDE=5108
OUT=5092
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
WINDOWS=[(27990840, 4120), (31449144, 12), (25960080, 12), (25911396, 16), (33689600, 16), (33689856, 192), (33690112, 48), (33690176, 48), (33690368, 32), (33690624, 160), (33690880, 160), (33691136, 272)]
WATCHES=['0x01a6bf26,0x01a6bf53','0x01919aa8','0xffa00000,0xffafffff']
def inputs():
 data=bytearray();seed=0x4d505532
 def rand():
  nonlocal seed
  seed=(seed*1664525+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  image=bytearray(struct.pack('<1272I',*[rand() for _ in range(1272)]))
  if n%4<3:image[:4120]=bytes([[0,255,165][n%4]])*4120
  def put(o,v):struct.pack_into('<I',image,o,v&0xffffffff)
  action=n%5;read=[0,990,1020,1023,0xfffffff0,0x7ffffff0][n//5%6];used=[0,1,2,31,32,33,63,64,65,1022,1023][n//30%11]
  put(4,read);put(4108,read+used);put(8,1 if action==3 else 1+n%2 if action==2 else n//330%3);image[4112]=[0,1,165][n//17%3]
  put(4120,0x02021100);put(4124,[0,1,2,0xffffffff][n//13%4]);put(4136,0x02021000);put(4168,[0,6,7,6][n//11%4])
  selected=n//7%2;present=n//19%4
  put(4352,0 if present==1 else 0x02021200);put(4356,0 if present==2 else 0x02021240);put(4360,selected)
  for v in range(2):
   p=4368+v*48;addr=0x02021200+v*64
   put(p,addr);put(p+4,addr);put(p+8,0 if (n//23+v)%3==0 else 0x02021300);put(p+24,0x02021408+v*256)
  put(4468,0x02021304);put(4472,0x02021304);put(4476,[1,165,0xffffffff][n%3])
  count=n%129 if action==1 else n//5%65;complete=min(used,n%33)*4+n%4
  if n<32:
   action=0;count=n+1;complete=0;put(4,0);put(4108,0);put(8,0);image[4112]=1;put(4124,1);put(4168,6);put(4352,0x02021200);put(4356,0x02021240);put(4360,n%2);put(4376,0x02021300);put(4424,0x02021300)
  data+=struct.pack('<5I',action,count,complete,rand(),rand())+image
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
 probe_source='.text\n.global __start\n__start:\nR0.H=exit_args;\nR0.L=exit_args;\nP0=1;\nEXCPT 0;\n.data\n.align 4\nexit_args: .long 0\n'
 execute('watch',probe_source)
 for opts in [['--watch-pc-int','0x1000'],['--watch-pc-int','0x1010','--watch-pc-int','0x1000'],['--watch-pc-int','0x1000,0x1008']]:
  result=subprocess.run([str(a.simulator),'--model','bf524',*opts,str(out/'watch.elf')],capture_output=True,timeout=60)
  if result.returncode!=5:raise RuntimeError('PC watch calibration failed')
 source=(templates/'midi-pump.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('midi-pump246',source,'midi-pump.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000',*[x for pc in WATCHES for x in ['--watch-pc-int',pc]]))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original MIDI flow output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/midi-pump/vectors'/f'midi-pump246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 routines={}
 for name,entry,end in [('enqueue',0x019e0d18,0x019e0e5c),('callback',0x019e10fc,0x019e193c),('pump',0x01a6a5fc,0x01a6a698),('submit',0x01a6c134,0x01a6c19c)]:routines[name]={'entry':hex(entry),'end_exclusive':hex(end),'sha256':hashlib.sha256(image[entry-base:end-base]).hexdigest()}
 report={'ldr_sha256':expected,'cases':CASES,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'routines':routines,'windows':WINDOWS,'pc_watches':WATCHES,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Original enqueue, complete event callback in event9/event16 profiles, pump, copy and submission with supplied graph. No code replacements or L1 trampolines.','Busy endpoint suppresses driver activation, or null endpoint rejects submission. All descriptors detached on entry; no descriptor reuse or full USB lifecycle.','Completion events/metadata and initial counters supplied independently, not physical/timing observations. No consumer concurrency, physical driver, DSP render or plugin integration.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 4096 original MIDI enqueue/pump/completion/reset profiles')
if __name__=='__main__':main()
