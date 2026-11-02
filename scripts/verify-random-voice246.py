#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded original Random synth Instrument/ADSR routing with supplied firmware.
Original firmware, disassembly and ELF stay in a new private output directory.
"""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES,RECORD,RESULT=4096,88,72
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs(fm=False,string=False,dna=False):
 data=bytearray();seed=0x52414e44
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 edges=(0,1,127,128,14744,14745,16383,16384,24000,32766,32767)
 for case in range(CASES):
  state=struct.pack('<11I',*[rnd() for _ in range(11)])
  rng,meter=rnd(),rnd();controls=[int(rnd()%65536)-32768 for _ in range(8)]
  controls[1]=rnd()%32768;controls[3]=rnd()%32768
  if case<512:
   controls[0]=(-32768,-32767,0,1,16384,32767)[case%6]
   controls[1]=edges[case%len(edges)];controls[3]=edges[case//len(edges)%len(edges)]
  destination=case//4%2;controls[2]=destination*2048+rnd()%2048
  kind=destination
  if case<2048 and case%11==0:kind=(destination+1)%4
  if case<2048 and case%17==0:controls[2]=8192
  values=[int(rnd()%65536)-32768 for _ in range(8)]
  type=11 if dna else 3 if string else 0 if fm else (1,7,2,4)[case%4]
  action=(2 if case%3==0 else 1) if case<2048 else (2 if case%128==0 else 0)
  if case>=2048:
   controls[0]=24000;controls[1]=(0,16000,30000,32767)[case//64%4];controls[3]=16000 if case%256<128 else 24000
  data+=state+struct.pack('<2I8h8hI',rng,meter,*controls,*values,(action|(kind<<8)|(type<<16)))
 return data

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--fm',action='store_true',help='Validate FM type 0')
 p.add_argument('--string',action='store_true',help='Validate String type 3')
 p.add_argument('--dna',action='store_true',help='Validate DNA type 11')
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
 assert len(inputs(a.fm,a.string,a.dna))==CASES*RECORD
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs(a.fm,a.string,a.dna))
 setup='imm32 P0, 0x01a1e914\nCALL (P0);\n'
 for i in range(24):setup+=f'imm32 P0, {0x18bef50+i*4}\nR0 = [P0];\nimm32 P0, 0x01aa6fd0\nCALL (P0);\nimm32 P0, {0x106a8f4+i*4}\n[P0] = R0;\n'
 templates=ROOT/'research/blackfin'
 source=(templates/'random-voice-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*RESULT)).replace('@SETUP@',setup)
 (out/'probe.s').write_text(source)
 def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=180,**kw)
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as stdout:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=stdout,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Both documented simulator patches are required')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'voice-contract.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as stdout,(out/'probe.log').open('wb') as stderr:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000',out/'probe.elf'],stdout=stdout,stderr=stderr)
 output=(out/'output.bin').read_bytes()
 if len(output)!=CASES*RESULT:raise RuntimeError('Incomplete original output: '+str(len(output)))
 artifacts={}
 for name,path in [('random-voice246-input.bin',out/'inputs.bin'),('random-voice246-output.bin',out/'output.bin')]:
  target=ROOT/'Tests/ReferenceVectors'/(name.replace('random-voice246','random-dna246') if a.dna else name.replace('random-voice246','random-string246') if a.string else name.replace('random-voice246','random-fm246') if a.fm else name)
  if a.record:target.write_bytes(path.read_bytes())
  if target.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle mismatch: '+name)
  artifacts[target.name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 report={'ldr_sha256':expected,'process':'0x019cea70','note_reset':'0x019ced74','factory':'0x019cf340','rng_address':'0xff802bc8','lcg_multiplier':1664525,'lcg_increment':1013904223,'range_initializer':'0x01a1e914','instrument_helper':'0x01a1dfec','envelope_helper':'0x019f69b0','cases':CASES,'state_bytes':44,'target_words':8,'instrument_types':[11] if a.dna else [3] if a.string else [0] if a.fm else [1,7,2,4],'destinations':['Instrument eight signals / two passes over four controls','ADSR first four signals'],'independent_cases':2048,'retained_stream_cases':2048,'artifacts':artifacts,'tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'model':'GNU 13.1 BF524 with both documented writeback patches; not hardware validation','limitations':['Selected native families and synth Instrument/ENV callback only; no FX/drum routing.','Caller-supplied target words, event order and RNG; original L1 scheduler not recovered.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print(f'PASS: {CASES} original Random synth calls, full self / RNG / meter / eight targets / return')
if __name__=='__main__':main()
