#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Private Drum sampler oracle for the independently reconstructed complete render."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4096
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x4452554d
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 # Four authored sample buffers: alternating extremes, ramp, sparse impulses, noise.
 for bank in range(4):
  pcm=[(-32768 if i%2 else 32767) if bank==0 else ((i*37)%65536)-32768 if bank==1 else (32767 if i%257==0 else -32768 if i%257==128 else 0) if bank==2 else (rnd()%65536)-32768 for i in range(4096)]
  data+=struct.pack('<16h4096h16h',*([0]*16),*pcm,*([0]*16))
 def position(frame):
  packed=round(frame*2048*2147483648/0x40998000);return packed>>16,(packed&65535)//2
 for n in range(CASES):
  pool=[]
  for slot in range(12):pool.extend((rnd(),0x02010000,((64+rnd()%2900)*2048+(rnd()%2048)),rnd()%3,rnd()))
  first=(0,1,64,512,1000)[n%5];last=(first,first+1,1000,2000,3000)[n//5%5];last=max(first,last)
  start,fine=position(first);end,endfine=position(last)
  c=[rnd()%32768,start,end,((n//32)%4)*8192+rnd()%8192,((n//16)%2)*16384+rnd()%16384,fine,endfine,(0,1,8192,16384,32767)[n//4%5]]
  flags=1 if n<2048 or n%256==0 else 0
  for bit,period in [(1,101),(2,29),(3,37),(4,13)]:
   if n%period==0:flags|=1<<bit
  if n%2:flags|=32
  flags|=(n%4)<<8
  pitch=[(0,1,0x04000000,0x08000000,0x7fffffff)[(n+i//16)%5] if n<2048 else rnd()&0x7fffffff for i in range(128)]
  gain=[int(rnd()%65536)-32768 for _ in range(128)]
  data+=struct.pack('<60I8h2I128I128h',*pool,*c,flags,n%12,*pitch,*gain)
 assert len(data)==33024+CASES*1032
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
 source=(ROOT/'research/blackfin/drum-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@SETUP@',setup).replace('@BYTES@',str(CASES*2032));(out/'probe.s').write_text(source)
 def run(cmd,**kw):subprocess.run(list(map(str,cmd)),check=True,timeout=180,**kw)
 templates=ROOT/'research/blackfin'
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as f:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=f,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Both simulator writeback patches required')
 run([a.assembler,'-I',a.testutils,'-o',out/'fp.o',templates/'parallel-fp-load.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'fp.elf',out/'fp.o'])
 with (out/'fp.bin').open('wb') as f:run([a.simulator,'--model','bf524',out/'fp.elf'],stdout=f,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'fp.bin').read_bytes())!=(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6):raise RuntimeError('FP word-load source-latching patch required')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'drum-render.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as f,(out/'probe.log').open('wb') as log:run([a.simulator,'--model','bf524','--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000',out/'probe.elf'],stdout=f,stderr=log)
 if (out/'output.bin').stat().st_size!=2052+33024+CASES*2032:raise RuntimeError('Incomplete Drum sampler output')
 artifacts={}
 for suffix,source in [('input',out/'inputs.bin'),('output',out/'output.bin')]:
  target=ROOT/'research/drum-render/vectors'/('drum246-'+suffix+'.bin')
  if a.record:target.write_bytes(source.read_bytes())
  if target.read_bytes()!=source.read_bytes():raise RuntimeError('Oracle mismatch '+target.name)
  artifacts[target.name]={'bytes':source.stat().st_size,'sha256':sha(source)}
 report={'ldr_sha256':expected,'original_render_blocks':CASES,'original_render_samples':CASES*128,'cpp_renderer':'Source/Drum246.h; independent comparison required','model':'GNU BF524 with Dreg, halfword and FP-load writeback patches','routines':{'render':'0x019789ec','factory':'0x01978df4','bind':'0x019788ec','reset':'0x01978926','release':'0x01978898','active_query':'0x019788b0'},'input_prefix_bytes':33024,'input_record_bytes':1032,'output_prefix_bytes':2052,'output_suffix_bytes':33024,'output_record_bytes':2032,'voices':12,'state_bytes':20,'fir_coefficients':1025,'coefficient_source':'0x018bf010 via original float-to-fract16 helper','artifacts':artifacts,'tools':{Path(k).name:sha(k) for k in (a.assembler,a.linker,a.simulator)},'limits':['Caller-supplied PCM, nonnegative pitch streams, gain and event cadence.','Corrected simulator is not physical acoustic validation. Dynamic envelope producer not included.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n')
 print('PASS: original drum sampler observations, 4096 blocks / 524288 samples')
if __name__=='__main__':main()
