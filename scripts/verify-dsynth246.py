#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Private D-synth original-render observations; lifecycle port only, not C++ audio parity."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4096
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x4453594e
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  pool=[]
  for slot in range(6):
   w=[rnd() for _ in range(18)];w[5]=slot;pool+=w
  context=[rnd() for _ in range(206)]
  for slot in range(6):context[200+slot]=rnd()%65
  c=[rnd()%32768 for _ in range(8)]
  if n<1024:c=[(0,8191,8192,16383,16384,24575,24576,32767)[(n//(1<<(k%5)))%8] for k in range(8)]
  action=1 if n<2048 or n%256==0 else 0
  if n%101==0:action|=2
  if n%29==0:action|=4
  if n%37==0:action|=8
  if n%113==0:action|=16
  if n%43==0:action|=32
  if n%157==0:action|=64
  pitch=[(0,1,0x146f108,0x7fffffff,0x80000000,0xffffffff)[(n+i//16)%6] if n<1024 else rnd() for i in range(128)]
  gain=[rnd()%32768 for _ in range(128)]
  data+=struct.pack('<314I8h2I128I128h',*pool,*context,*c,action,n%6,*pitch,*gain)
 assert len(data)==CASES*2048
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
 setup='imm32 P0,0x01a1e914\nCALL (P0);\nimm32 P0,0x01a49b1c\nCALL (P0);\n'
 for source,dest,count in [(0x018c13c4,0x0106b3cc,256),(0x018be744,0xff801f50,129)]:
  for i in range(count):setup+=f'imm32 P0,{source+i*4}\nR0 = [P0];\nimm32 P0,0x01aa6fd0\nCALL (P0);\nimm32 P0,{dest+i*4}\n[P0] = R0;\n'
 for label in ('coeff_args','freq_args','range_args'):setup+=f'R0.H = {label};\nR0.L = {label};\nP0 = 5;\nEXCPT 0;\n'
 source=(ROOT/'research/blackfin/dsynth-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@SETUP@',setup).replace('@BYTES@',str(CASES*4320));(out/'probe.s').write_text(source)
 def run(cmd,**kw):subprocess.run(list(map(str,cmd)),check=True,timeout=180,**kw)
 templates=ROOT/'research/blackfin'
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as f:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=f,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Both simulator writeback patches required')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'cwo-foundation.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as f,(out/'probe.log').open('wb') as log:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000',out/'probe.elf'],stdout=f,stderr=log)
 if (out/'output.bin').stat().st_size!=1604+CASES*4320:raise RuntimeError('Incomplete D-synth output')
 artifacts={}
 for suffix,source in [('input',out/'inputs.bin'),('output',out/'output.bin')]:
  target=ROOT/'research/dsynth-render/vectors'/('dsynth246-'+suffix+'.bin')
  if a.record:target.write_bytes(source.read_bytes())
  if target.read_bytes()!=source.read_bytes():raise RuntimeError('Oracle mismatch '+target.name)
  artifacts[target.name]={'bytes':source.stat().st_size,'sha256':sha(source)}
 report={'ldr_sha256':expected,'original_render_blocks':CASES,'original_render_observed_samples':CASES*128,'cpp_audio_renderer':'not implemented; no audio parity claim','routines':{'render':'0x0197e1d8','factory':'0x0197ee60','constructor':'0x0197e058','bind':'0x0197e0b0','reset':'0x0197e0ee','release':'0x0197e168','shared_init':'0x0197eeb0'},'ranges':'0x01de5b90','state_bytes':72,'pool_bytes':432,'shared_context_bytes':824,'input_record_bytes':2048,'output_prefix_bytes':1604,'output_record_bytes':4320,'controls':8,'guards':{'pool':'0xffb001b0','context':'0x01cebbe0','frequency_end':'0xff802154','rng':'0xff802bc8'},'mix_modes_exercised':8,'artifacts':artifacts,'tools':{Path(k).name:sha(k) for k in (a.assembler,a.linker,a.simulator)},'limits':['Audio and post-render state are original-instruction observations, not a reconstructed renderer.','C++ implements lifecycle and history capture only; render state is explicitly handed back by the oracle.','Caller supplies pitch, gain, event order and bounded six-voice context. No original scheduler or acoustic-equivalence claim.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n')
 print('PASS: D-synth original complete render observations, 4096 blocks / 524288 samples')
if __name__=='__main__':main()
