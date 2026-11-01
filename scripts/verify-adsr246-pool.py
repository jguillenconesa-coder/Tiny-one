#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Original complete ADSR lazy accessor, valid indices only; private firmware.
Fixtures contain our numeric state images and returned addresses, not code.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess

ROOT=Path(__file__).resolve().parents[1]
CASES,RECORD,RESULT=1024,188,176
WATCHES=['0x01919aa8','0xffa00000,0xffafffff']

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x504f4f4c
 def rnd():
  nonlocal seed
  seed=(seed*1664525+1013904223)&0xffffffff
  return seed
 for c in range(CASES):
  reset=c<512 or (c-512)%64==0
  index=c%6
  ready=[0,1,0x100,0x10000,0x80000000,0xffffffff,0,0][c%8]
  # 3 ready words with neighbors, 2 guard words + 36 pool words + 2 guards.
  image=[rnd(),ready,rnd()]+[rnd() for _ in range(40)]
  mutate=0xffffffff if c%3==0 else c%36
  data+=struct.pack('<47I',reset,index,mutate,rnd(),*image)
 return data

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for k in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+k,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a NEW private output outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Only inventoried rev246 supported')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,addr,n,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=addr and addr+n<=base+len(image) and not flags&0x1000:image[addr-base:addr-base+n]=(struct.pack('<I',arg)*((n+3)//4))[:n] if flags&0x100 else raw[cursor+16:cursor+16+n]
  cursor+=16+(0 if flags&0x100 else n)
 out.mkdir(mode=0o700,parents=True);(out/'sdram.bin').write_bytes(image);data=inputs();(out/'inputs.bin').write_bytes(data)
 def copy(label,source,dest,n):
  return f'{source}\n{dest}\nP3 = {n//4};\nLSETUP({label},{label}_end) LC0 = P3;\n{label}: R0 = [P1++];\n{label}_end: [P2++] = R0;\n'
 load=copy('ready_in','P1 = P4;\nP1 += 16;','imm32 P2, 0x01b60538',12)+copy('pool_in','P1 = P4;\nP1 += 28;','imm32 P2, 0xff8000cc',160)
 save=copy('ready_out','imm32 P1, 0x01b60538','P2 = P5;',12)+'P5 = P2;\n'+copy('pool_out','imm32 P1, 0xff8000cc','P2 = P5;',160)+'P5 = P2;\n'
 source=f'''.include "testutils.inc"
start
imm32 SP, 0x03000000
P4.H = inputs;
P4.L = inputs;
P5.H = results;
P5.L = results;
record_loop:
R0 = [P4];
CC = R0 == 0;
IF CC JUMP retain;
{load}
retain:
R0 = [P4+8];
imm32 R1, 0xffffffff
CC = R0 == R1;
IF CC JUMP access;
R0 <<= 2;
imm32 R1, 0xff8000d4
R0 = R0 + R1;
P0 = R0;
R0 = [P4+12];
[P0] = R0;
access:
R0 = [P4+4];
imm32 P0, 0x0191264c
CALL (P0);
P0.H = returned;
P0.L = returned;
[P0] = R0;
{save}
P0.H = returned;
P0.L = returned;
R0 = [P0];
[P5++] = R0;
P0 = {RECORD};
P4 = P4 + P0;
R0 = P4;
R1.H = inputs_end;
R1.L = inputs_end;
CC = R0 == R1;
IF CC JUMP done;
JUMP.L record_loop;
done:
R0.H = args;
R0.L = args;
P0 = 5;
EXCPT 0;
exit 0
.data
.align 4
args: .long 1,results,{CASES*RESULT}
returned: .long 0
inputs: .incbin "{out}/inputs.bin"
inputs_end:
results: .space {CASES*RESULT}
.section .sdram,"awx"
.incbin "{out}/sdram.bin"
'''
 (out/'probe.s').write_text(source);templates=ROOT/'research/blackfin'
 def run(cmd,**kw):return subprocess.run([str(x) for x in cmd],check=True,timeout=180,**kw)
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as stdout:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=stdout)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Both GNU writeback patches required')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'voice-control-input.ld','-o',out/'probe.elf',out/'probe.o'])
 # Verify watch semantics on a program of our own before trusting exclusions.
 (out/'watch.s').write_text('.text\n.global __start\n__start:\nR0.H = exit_args;\nR0.L = exit_args;\nP0 = 1;\nEXCPT 0;\n.data\n.align 4\nexit_args: .long 0\n')
 run([a.assembler,'-o',out/'watch.o',out/'watch.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'watch.elf',out/'watch.o'])
 codes=[]
 for opts in [[],['--watch-pc-int','0x1000'],['--watch-pc-int','0x1010','--watch-pc-int','0x1000'],['--watch-pc-int','0x1000,0x1008']]:
  codes.append(subprocess.run([str(a.simulator),'--model','bf524',*opts,str(out/'watch.elf')],capture_output=True,timeout=180).returncode)
 if codes!=[0,5,5,5]:raise RuntimeError('PC watch probes failed')
 guards=[arg for address in WATCHES for arg in ['--watch-pc-int',address]]
 with (out/'output.bin').open('wb') as stdout,(out/'probe.log').open('wb') as stderr:run([a.simulator,'--model','bf524','--memory-size','128M','--memory-region','0xff800000,0x8000','--watch-cycles-int','900000000',*guards,out/'probe.elf'],stdout=stdout,stderr=stderr)
 output=(out/'output.bin').read_bytes()
 if len(output)!=CASES*RESULT:raise RuntimeError('Incomplete original output')
 cold=warm=0;ready=0
 for c in range(CASES):
  x=data[c*RECORD:(c+1)*RECORD];e=output[c*RESULT:(c+1)*RESULT]
  if struct.unpack_from('<I',x)[0]:ready=struct.unpack_from('<I',x,20)[0]
  if ready==0:cold+=1
  else:warm+=1
  ready=struct.unpack_from('<I',e,4)[0]
 artifacts={}
 for name,path in [('adsr246-pool-input.bin',out/'inputs.bin'),('adsr246-pool-output.bin',out/'output.bin')]:
  fixture=ROOT/'Tests/ReferenceVectors'/name
  if a.record:fixture.write_bytes(path.read_bytes())
  if fixture.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle differs: '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path)}
 paths=[Path(__file__).resolve(),ROOT/'Source/Adsr246Pool.h',ROOT/'Source/Envelope246Transition.h',ROOT/'Tests/Adsr246PoolTests.cpp']
 report={'ldr_sha256':expected,'entry':'0x0191264c','end_exclusive':'0x019126d8','routine_sha256':hashlib.sha256(image[0x0191264c-base:0x019126d8-base]).hexdigest(),'initializer':'0x019120ba','array_helpers':['0x01aa3862','0x01aa37c4'],'cases':CASES,'independent':512,'streams':8,'calls_per_stream':64,'cold':cold,'warm':warm,'input_record_bytes':RECORD,'output_record_bytes':RESULT,'pool_base':'0xff8000d4','ready_word':'0x01b6053c','monitored_regions':[{'address':'0x01b60538','bytes':12},{'address':'0xff8000cc','bytes':160}],'artifacts':artifacts,'tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'source_sha256':{str(x.relative_to(ROOT)):sha(x) for x in paths},'pc_watches':WATCHES,'pc_watch_probe_exit_codes':codes,'pc_watch_original_run_exit_code':0,'model':'GNU BF524 with both documented writeback patches','limits':['Valid indices 0..5 only. Original invalid-index assert excluded; adapter rejects before initialization.','Explicit ready word, poison pool and host-owned event order. No concurrency, allocator, scheduler, L1 or physical audio claim.','Selected pool/ready regions with guards and returned pointer compared; not full RAM or CPU state.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS original ADSR lazy pool:',CASES,'calls;',cold,'cold /',warm,'warm')
if __name__=='__main__':main()
