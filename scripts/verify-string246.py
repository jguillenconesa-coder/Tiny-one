#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Replay String lifecycle and mono render; proprietary instructions remain private."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4096

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x53545247
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  pitch=(0x10000,0x2aaaa,0x1000000,0x146f108,0x20000000,0x7fffffff)[n%6] if n<1024 else 100000+rnd()%0x7ffe0000
  period=max(0x20000,min(0x60000000,(1<<48)//pitch))
  cursor=rnd()%period
  state=[rnd(),rnd(),rnd(),rnd(),cursor,0x010225ec]
  excite=int(rnd()%65536)-32768;clear=(n%17)*1536;flags=rnd()&0xffffff00|(n%2)
  blue=(0,1,8192,16383,16384)[n%5] if n<1024 else rnd()%16385
  green=(0,1023,1024,23040,24575)[n//5%5] if n<1024 else rnd()%24576
  white=(0,1,16384,32767)[n//25%4] if n<1024 else rnd()%32768
  orange=(0,1,8192,16383,16384)[n//100%5] if n<1024 else rnd()%16385
  action=1 if (n<2048 and n%32==0) or n%256==0 else 0
  if action and n%3==0:action|=2
  if action and n%5==0:action|=8
  if n%37==0:action|=4
  data+=struct.pack('<6IhhI4h6I',*state,excite,clear,flags,blue,green,white,orange,pitch,rnd(),action,rnd(),0,0)
 return data

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware LDR')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,n,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+n<=base+len(image) and not flags&0x1000:image[address-base:address-base+n]=(struct.pack('<I',arg)*((n+3)//4))[:n] if flags&0x100 else raw[cursor+16:cursor+16+n]
  cursor+=16+(0 if flags&0x100 else n)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);data=inputs();assert len(data)==CASES*64
 (out/'inputs.bin').write_bytes(data)
 setup='imm32 P0,0x01a1e914\nCALL (P0);\n'
 tables=[(0x018be948,0x0106a5ec,129,0x01aa7014,2),(0x018bebcc,0x0106a730,225,0x01aa7014,2),(0x018befb0,0x0106a954,24,0x01aa6fd0,4)]
 source_tables=[]
 for k,(src,dst,count,fn,width) in enumerate(tables):
  # All three sources must remain ordinary legible SDRAM payload, not model substitutions.
  source_tables.append(dict(source=hex(src),destination=hex(dst),count=count,width=width,source_sha256=hashlib.sha256(image[src-base:src-base+count*4]).hexdigest()))
  setup+=f'imm32 P4,{src}\nimm32 I0,{dst}\nP3 = {count};\nLSETUP(table{k},table{k}_end) LC0 = P3;\ntable{k}: R0 = [P4++];\nimm32 P0,{fn}\nCALL (P0);\ntable{k}_end: '+('W[I0++] = R0.L;\n' if width==2 else '[I0++] = R0;\n')
  setup+=f'R0.H = table{k}_args;\nR0.L = table{k}_args;\nP0 = 5;\nEXCPT 0;\n'
 setup+='R0.H = range_args;\nR0.L = range_args;\nP0 = 5;\nEXCPT 0;\n'
 size=CASES*328+16*49152
 src=(ROOT/'research/blackfin/string-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@SETUP@',setup).replace('@BYTES@',str(size))
 (out/'probe.s').write_text(src)
 def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=180,**kw)
 templates=ROOT/'research/blackfin'
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as f:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=f,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Both documented simulator patches required')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'cwo-foundation.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as f,(out/'probe.log').open('wb') as log:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','1500000000',out/'probe.elf'],stdout=f,stderr=log)
 if (out/'output.bin').stat().st_size!=868+size:raise RuntimeError('Incomplete String output '+str((out/'output.bin').stat().st_size))
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',out/'output.bin')]:
  name='string246-'+suffix+'.bin';target=ROOT/'research/string-render/vectors'/name
  if a.record:target.write_bytes(path.read_bytes())
  if target.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle mismatch '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 report={'ldr_sha256':expected,'cases':CASES,'samples':CASES*128,'state_bytes':32,'workspace_bytes':49152,'complete_workspace_snapshots':16,'routines':{'render':'0x01a4f098','construct':'0x01a4ef38','bind':'0x01a4ef64','release':'0x01a4efae','reset':'0x01a4efc8','period_division':'0x01aa494a'},'slot':0,'workspace_address':'0x010225ec','source_tables':source_tables,'artifacts':artifacts,'tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Valid cursors and explicit slot 0 workspace; synthetic states, not complete original startup/allocator.','RNG and callback ordering supplied by harness; not acoustic or hardware validation.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: String original lifecycle/render, 4096 blocks and 16 complete workspaces')
if __name__=='__main__':main()
