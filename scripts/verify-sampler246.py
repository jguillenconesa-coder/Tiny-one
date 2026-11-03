#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Private melodic sampler oracle for the independently reconstructed complete render."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4096
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 spec=importlib.util.spec_from_file_location('drum',ROOT/'scripts/verify-drum246.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
 data=bytearray(module.inputs()[:33024]);seed=0x53414d50
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 def enc(frame):
  q=round(frame*4096*2147483648/0x40998000);return q>>16,(q&65535)//2
 for n in range(CASES):
  pool=[]
  for j in range(6):pool.extend((rnd(),0x02010000,((rnd()%3500)*4096+rnd()%4096),rnd()))
  start,unused=enc((0,1,100,200)[n%4]);end,unused=enc((300,2000,3500)[n//4%3]);ls,lf=enc((100,200,500)[n//12%3]);le,ef=enc((100,1000,2200)[n//36%3]);
  controls=[start,ls,le,end,(n//108%2)*16384,lf,ef,(0,1,8192,16384,32767)[n//216%5]]
  flags=(1 if n<2048 or n%256==0 else 0)|(2 if n%101==0 else 0)|(4 if n%29==0 else 0)|(8 if n%37==0 else 0)|(32 if n%2 else 0)|((n%4)<<8)
  pitch=[(0,1,0x04000000,0x08000000,0x7fffffff)[(n+i//16)%5] if n<2048 else rnd() for i in range(128)]
  scale=(0,1,0x20000000,0x40000000,0x7fffffff)[n//5%5]
  data+=struct.pack('<24I8h3I128I',*pool,*controls,flags,n%6,scale,*pitch)
 assert len(data)==33024+CASES*636
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
 setup=''
 for i in range(1025):setup+=f'imm32 P0,{0x018bf010+i*4}\nR0 = [P0];\nimm32 P0,0x01aa7014\nCALL (P0);\nimm32 P0,{0xff802154+i*2}\nW[P0] = R0.L;\n'
 setup+='R0.H = coeff_args;\nR0.L = coeff_args;\nP0 = 5;\nEXCPT 0;\n'
 source=(ROOT/'research/blackfin/sampler-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@SETUP@',setup).replace('@BYTES@',str(CASES*968));(out/'probe.s').write_text(source)
 def run(cmd,**kw):subprocess.run(list(map(str,cmd)),check=True,timeout=180,**kw)
 templates=ROOT/'research/blackfin'
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as f:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=f,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Both simulator writeback patches required')
 run([a.assembler,'-I',a.testutils,'-o',out/'fp.o',templates/'parallel-fp-load.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'fp.elf',out/'fp.o'])
 with (out/'fp.bin').open('wb') as f:run([a.simulator,'--model','bf524',out/'fp.elf'],stdout=f,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'fp.bin').read_bytes())!=(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6):raise RuntimeError('FP word-load source-latching patch required')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'sampler-render.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as f,(out/'probe.log').open('wb') as log:run([a.simulator,'--model','bf524','--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000',out/'probe.elf'],stdout=f,stderr=log)
 if (out/'output.bin').stat().st_size!=2052+33024+CASES*968:raise RuntimeError('Incomplete melodic sampler output')
 artifacts={}
 for suffix,source in [('input',out/'inputs.bin'),('output',out/'output.bin')]:
  target=ROOT/'research/sampler-render/vectors'/('sampler246-'+suffix+'.bin')
  if a.record:target.write_bytes(source.read_bytes())
  if target.read_bytes()!=source.read_bytes():raise RuntimeError('Oracle mismatch '+target.name)
  artifacts[target.name]={'bytes':source.stat().st_size,'sha256':sha(source)}
 report={'ldr_sha256':expected,'blocks':CASES,'samples':CASES*128,'render':'0x01a51a7c','factory':'0x01a51e34','reset':'0x01a518f4','release':'0x01a51964','bind':'0x01a519a0','voices':6,'state_bytes':16,'input_record_bytes':636,'output_record_bytes':968,'artifacts':artifacts,'model':'GNU BF524 with three writeback patches','dependencies':'Caller-supplied synthetic PCM with explicit zero padding, pitch stream, scalar pitch factor and lifecycle order; all 1025 legible FIR coefficients converted by original helper. No acoustic equivalence claim.'}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n')
 print('PASS: sampler original observations: 4096 blocks / 524288 samples')
if __name__=='__main__':main()
