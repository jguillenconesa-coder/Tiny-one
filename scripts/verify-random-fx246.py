#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce the Random FX parameter branch with operator-supplied rev246 firmware.
Only synthetic inputs and numeric observations may be recorded in the checkout.
"""
import argparse, hashlib, importlib.util, json, struct, subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(name,path):
 s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def inputs():
 # Retain the proven state/edge/stream generator, changing only FX dispatch.
 data=bytearray(load('random_voice_generator',ROOT/'scripts/verify-random-voice246.py').inputs())
 for n in range(4096):
  off=n*88;action=struct.unpack_from('<I',data,off+84)[0]&255
  kind=(2,5)[n%2];dest=4096+n%2048
  if n<2048 and n%11==0:kind=(0,1,3,6)[n//11%4]
  if n<2048 and n%17==0:dest=(0,2048,6144,-2048)[n//17%4]
  struct.pack_into('<h',data,off+56,dest)
  struct.pack_into('<I',data,off+84,action|(kind<<8)|((n//2%8)<<16))
 return data

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for k in ('ldr','assembler','linker','simulator','testutils','output'):p.add_argument('--'+k,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a NEW private directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported LDR')
 mapper=load('ldr_mapper',ROOT/'scripts/map-op1-ldr.py');raw=a.ldr.read_bytes();meta,_=mapper.inspect(raw)
 base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,n,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+n<=base+len(image) and not flags&0x1000:image[address-base:address-base+n]=(struct.pack('<I',arg)*((n+3)//4))[:n] if flags&0x100 else raw[cursor+16:cursor+16+n]
  cursor+=16+(0 if flags&0x100 else n)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs())
 setup='imm32 P0, 0x01a1e914\nCALL (P0);\n'
 for i in range(24):setup+=f'imm32 P0, {0x18bef50+i*4}\nR0 = [P0];\nimm32 P0, 0x01aa6fd0\nCALL (P0);\nimm32 P0, {0x106a8f4+i*4}\n[P0] = R0;\n'
 templates=ROOT/'research/blackfin'
 # The authored harness uses the same 44-byte state and 8-word target contract.
 # FX instance indices remain zero; the supplied type goes into each FX slot.
 source=(templates/'random-voice-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(4096*72)).replace('@SETUP@',setup)
 for before,after in [('0x019cea70','0x019cefb0'),('0x019ced74','0x019cf2f4'),('0xff8016e4','0xff801734'),('0xff801704','0xff801754')]:source=source.replace(before,after)
 (out/'probe.s').write_text(source)
 def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=180,**kw)
 for name,expected_words in [('parallel-halfword',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd))]:
  run([a.assembler,'-I',a.testutils,'-o',out/(name+'.o'),templates/(name+'.s')]);run([a.linker,'-T',templates/'flat.ld','-o',out/(name+'.elf'),out/(name+'.o')])
  with (out/(name+'.bin')).open('wb') as f:run([a.simulator,'--model','bf524',out/(name+'.elf')],stdout=f,stderr=subprocess.DEVNULL)
  if struct.unpack('<5I',(out/(name+'.bin')).read_bytes())!=expected_words:raise RuntimeError('Required simulator writeback patches missing')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'voice-contract.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as f,(out/'probe.log').open('wb') as log:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000',out/'probe.elf'],stdout=f,stderr=log)
 if (out/'output.bin').stat().st_size!=4096*72:raise RuntimeError('Incomplete original output')
 artifacts={}
 for name,local in [('random-fx246-input.bin','inputs.bin'),('random-fx246-output.bin','output.bin')]:
  fixture=ROOT/'Tests/ReferenceVectors'/name
  if a.record:fixture.write_bytes((out/local).read_bytes())
  if fixture.read_bytes()!=(out/local).read_bytes():raise RuntimeError('Oracle mismatch: '+name)
  artifacts[name]={'bytes':fixture.stat().st_size,'sha256':sha(fixture),'matches_committed_fixture':True}
 evidence={'ldr_sha256':expected,'process':'0x019cefb0','note_reset':'0x019cf2f4','range_initializer':'0x01a1e914','range_helper':'0x01a1de68','cases':4096,'independent_cases':2048,'retained_cases':2048,'fx_types':list(range(8)),'kinds':[2,5],'state_bytes':44,'target_words':8,'artifacts':artifacts,'tools':{str(x):sha(x) for x in (a.assembler,a.linker,a.simulator)},'scope':'Complete self, RNG, 32-bit meter, eight targets and return. FX kinds 2/5 parameter dispatch only; kind 4 excluded.','limits':['Original caller, clock, initialization/ownership and acoustic equivalence not established.','GNU BF524 with documented D-register and halfword writeback patches; not hardware.']}
 (out/'evidence.json').write_text(json.dumps(evidence,indent=2)+'\n');print('PASS: 4096 original Random FX calls recorded/reproduced')
if __name__=='__main__':main()
