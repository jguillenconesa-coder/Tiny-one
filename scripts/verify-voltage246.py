#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Private Voltage lifecycle/render oracle. Only numerical fixtures are recorded."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4096
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x564f4c54
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 edges=(0,1,65535,65536,0x146f108,0x7fffffff,0x80000000,0xffffffff)
 for n in range(CASES):
  pool=[]
  for slot in range(6):
   words=[rnd() for _ in range(13)];words[12]=(words[12]&0xffffff00)|((n+slot)%3 if n%5 else 127);pool+=words
  controls=[rnd()%32768 for _ in range(4)]
  if n<1024:controls=[(0,1,32766,32767)[n%4],(0,1,8192,32767)[n//4%4],(0,1,15766,15767,17000,32766,32767)[n//16%7],(0,1,16384,32767)[n//112%4]]
  action=1 if n<2048 or n%256==0 else 0
  if n%29==0:action|=2
  if n%37==0:action|=4
  if n%101==0:action|=8
  pitch=[edges[(n+i//16)%8] if n<1024 else rnd() for i in range(128)]
  data+=struct.pack('<78I4h2I128I',*pool,*controls,action,n%6,*pitch)
 assert len(data)==CASES*840
 return data
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ('ldr','assembler','linker','simulator','testutils','output'):p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware LDR')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,n,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+n<=base+len(image) and not flags&0x1000:image[address-base:address-base+n]=(struct.pack('<I',arg)*((n+3)//4))[:n] if flags&0x100 else raw[cursor+16:cursor+16+n]
  cursor+=16+(0 if flags&0x100 else n)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs())
 setup='imm32 P0,0x01a1e914\nCALL (P0);\nimm32 P0,0x01a49b1c\nCALL (P0);\nimm32 P4,0x018be948\nimm32 I0,0x0106a5ec\nP3 = 129;\nLSETUP(table,table_end) LC0 = P3;\ntable: R0 = [P4++];\nimm32 P0,0x01aa7014\nCALL (P0);\ntable_end: W[I0++] = R0.L;\n'
 for label in ('table_args','range_args','padding_args'):setup+=f'R0.H = {label};\nR0.L = {label};\nP0 = 5;\nEXCPT 0;\n'
 source=(ROOT/'research/blackfin/voltage-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@SETUP@',setup).replace('@BYTES@',str(CASES*1472));(out/'probe.s').write_text(source)
 def run(cmd,**kw):subprocess.run(list(map(str,cmd)),check=True,timeout=180,**kw)
 templates=ROOT/'research/blackfin'
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as f:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=f,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Both simulator writeback patches required')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'cwo-foundation.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as f,(out/'probe.log').open('wb') as log:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','1500000000',out/'probe.elf'],stdout=f,stderr=log)
 if (out/'output.bin').stat().st_size!=324+CASES*1472:raise RuntimeError('Incomplete Voltage output')
 artifacts={}
 for suffix,source in [('input',out/'inputs.bin'),('output',out/'output.bin')]:
  target=ROOT/'research/voltage-render/vectors'/('voltage246-'+suffix+'.bin')
  if a.record:target.write_bytes(source.read_bytes())
  if target.read_bytes()!=source.read_bytes():raise RuntimeError('Oracle mismatch '+target.name)
  artifacts[target.name]={'bytes':source.stat().st_size,'sha256':sha(source)}
 report={'ldr_sha256':expected,'cases':CASES,'samples':CASES*128,'pool_bytes':312,'state_bytes':52,'pitch_words':128,'display_words':32,'routines':{'render':'0x01a49bf4','factory':'0x01a49ef8','reset':'0x01a49b94','release':'0x01a49b4e','sine_loader':'0x01a49b1c'},'range_address':'0x01de5bd0','source_table':{'address':'0x018be948','count':129,'sha256':hashlib.sha256(image[0x18be948-base:0x18be948-base+516]).hexdigest()},'artifacts':artifacts,'tools':{str(x):sha(x) for x in (a.assembler,a.linker,a.simulator)},'limits':['Caller supplies controls, six-state pool, per-sample pitch and event ordering.','No hardware/OTP/other-RAM model, original scheduler or acoustic equivalence is claimed.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: Voltage original lifecycle/render, 4096 blocks / 524288 samples')
if __name__=='__main__':main()
