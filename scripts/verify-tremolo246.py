#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Execute isolated rev246 Tremolo (Dual LFO) voice routing with supplied firmware.
Original instructions and ELF remain in a new private output directory.
"""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES,RECORD,RESULT=2048,856,832
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x5452454d
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 edges=(0,1,2047,2048,8192,14744,14745,16000,16383,16384,16385,24576,32766,32767)
 for case in range(CASES):
  state=struct.pack('<7I',*[rnd() for _ in range(7)])
  display=struct.pack('<8I',*[rnd() for _ in range(8)])
  c=[rnd()%32768 for _ in range(8)]
  if case<512:
   for j in range(4):c[j]=edges[(case//(j+1)+j)%len(edges)]
  if 512<=case<1024:c[1]=(-32768,-32767,-1,0,1,32767)[case%6]
  c[7]=(case%5)*6554
  tempo=(66485965,199457888,365672800,498644736)[case%4]
  if 512<=case<1024:tempo=rnd()
  kind=(0,3,0,3,0,1,2,4)[case%8] if case<1024 else (0,3)[case%2]
  action=(2 if case%3==0 else 1) if case<1024 else (2 if case%128==0 else 0)
  pitch=[(0,65536,21474836,0x7fffffff,rnd())[j%5] for j in range(128)]
  audio=[(0,32767,-32768,16384,int(rnd()%65536)-32768)[j%5] for j in range(128)]
  if case>=1024:
   pitch=[21474836]*128;audio=[16384]*128
   c[0]=edges[(case//64)%len(edges)];c[1]=c[2]=24000;c[3]=16000 if case%256<128 else 24000
  data+=state+display+struct.pack('<8h3I128I128h',*c,tempo,kind,action,*pitch,*audio)
 return data

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware LDR')
 spec=importlib.util.spec_from_file_location('ldr_map',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,n,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+n<=base+len(image) and not flags&0x1000:image[address-base:address-base+n]=(struct.pack('<I',arg)*((n+3)//4))[:n] if flags&0x100 else raw[cursor+16:cursor+16+n]
  cursor+=16+(0 if flags&0x100 else n)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs())
 setup=''
 for i in range(24):setup+=f'imm32 P0, {0x18bef50+i*4}\nR0 = [P0];\nimm32 P0, 0x01aa6fd0\nCALL (P0);\nimm32 P0, {0x106a8f4+i*4}\n[P0] = R0;\n'
 templates=ROOT/'research/blackfin'
 source=(templates/'tremolo-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*RESULT)).replace('@SETUP@',setup)
 (out/'probe.s').write_text(source)
 def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=180,**kw)
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as stdout:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=stdout,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Both documented simulator patches are required')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'voice-contract.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as stdout,(out/'probe.log').open('wb') as stderr:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000',out/'probe.elf'],stdout=stdout,stderr=stderr)
 output=(out/'output.bin').read_bytes()
 if len(output)!=96+CASES*RESULT:raise RuntimeError('Incomplete original output: '+str(len(output)))
 artifacts={}
 for name,path in [('tremolo246-input.bin',out/'inputs.bin'),('tremolo246-output.bin',out/'output.bin')]:
  target=ROOT/'Tests/ReferenceVectors'/name
  if a.record:target.write_bytes(path.read_bytes())
  if target.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle mismatch: '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 report={'ldr_sha256':expected,'process':'0x019cbe48','note_reset':'0x019cbdca','synth_factory':'0x019cc664','factory_slot':'0 at 0x018e1358','cases':CASES,'pitch_words':CASES*128,'audio_samples':CASES*128,'full_state_bytes':28,'full_shared_display_bytes':32,'shapes':5,'valid_kinds':[0,3],'independent_cases':1024,'stream_cases':1024,'coefficient_words':24,'coefficient_source':'0x018bef50','coefficient_runtime':'0x0106a8f4','float_to_q31':'0x01aa6fd0','coefficient_setup':'0x01a53c4a..0x01a53c66','model':'GNU 13.1 BF524 with documented D-register and halfword writeback patches; not hardware','artifacts':artifacts,'tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'limitations':['Caller-supplied initial pitch and audio buffers; not original L1 scheduler or mixer.','Display is globally shared in original; call order explicit in this isolated harness.','No physical codec rate or acoustic matching established.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print(f'PASS: {CASES} original Tremolo calls, {CASES*128} pitch words and audio samples, full state/display')
if __name__=='__main__':main()
