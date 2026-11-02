#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce original FM table/constructor/reset/six-envelope dependencies.
Ten protected source words use an explicit model; twenty clear words are locked.
Private firmware, SDRAM, original instructions and ELF never enter the checkout.
"""
import argparse,hashlib,importlib.util,json,math,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=8192

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 out=bytearray();seed=0x464d3234
 def rnd():
  nonlocal seed
  seed=(seed*1664525+1013904223)&0xffffffff;return seed
 for i in range(CASES):
  state=[rnd() for _ in range(15)]
  amount=int(rnd()%65536)-32768
  if i<1024:amount=(-32768,-32767,-16384,-1,0,1,16384,32767)[i%8]
  action=(1,2,3)[i%3] if i<4096 else (1 if i%256==0 else 2 if i%256==127 else 0)
  table=[int(rnd()%65536)-32768 for _ in range(30)]
  if i%16==0:table[:6]=[-32768,-1,0,1,32766,32767]
  out+=struct.pack('<15IhHI30h',*state,amount,0,action,*table)
 return out

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--table-model',type=Path,default=ROOT/'research/blackfin/fm246-table-model.json');p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a NEW private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware LDR')
 model=json.loads(a.table_model.read_text());values=model['float32_values']
 if len(values)!=30 or any(not math.isfinite(x) or not 0<x<=1 for x in values):raise ValueError('Expected 30 bounded positive model values')
 floats=struct.pack('<30f',*values)
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0;source=[];pointer=None
 while cursor<len(raw):
  flags,address,n,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+n<=base+len(image) and not flags&0x1000:image[address-base:address-base+n]=(struct.pack('<I',arg)*((n+3)//4))[:n] if flags&0x100 else raw[cursor+16:cursor+16+n]
  if not flags&0x100:
   if address<=0xff906f6c and 0xff906f70<=address+n:
    off=0xff906f6c-address
    if off%24<8:raise ValueError('Shared pointer is no longer in a clear interval')
    pointer=struct.unpack_from('<I',raw,cursor+16+off)[0]
   for i in range(30):
    target=0xff906f70+i*4
    if address<=target and target+4<=address+n:
     off=target-address;protected=off%24<8
     if not protected and floats[i*4:i*4+4]!=raw[cursor+16+off:cursor+20+off]:raise ValueError('Model altered legible FM coefficient '+str(i))
     source.append({'index':i,'address':hex(target),'block_address':hex(address),'ldr_offset':hex(cursor+16+off),'offset_mod24':off%24,'protected_under_partial_model':protected})
  cursor+=16+(0 if flags&0x100 else n)
 protected=[x['index'] for x in source if x['protected_under_partial_model']]
 if len(source)!=30 or protected!=model['authored_indices'] or pointer!=0xffb00168:raise ValueError('FM table/pointer provenance mismatch')
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);data=inputs();assert len(data)==CASES*128
 (out/'inputs.bin').write_bytes(data);(out/'table-floats.bin').write_bytes(floats)
 factory='imm32 P0, 0xffb00000\nimm32 R0, 0xa5a5a5a5\nP1 = 90;\nLSETUP(fill_pool,fill_pool_end) LC0 = P1;\nfill_pool:\nfill_pool_end: [P0++] = R0;\n'
 for slot in range(6):factory+=f'imm32 R0, {slot}\nimm32 P0, 0x019a3d38\nCALL (P0);\nP0.H = factory_return;\nP0.L = factory_return;\n[P0] = R0;\nR0.H = pool_args;\nR0.L = pool_args;\nP0 = 5;\nEXCPT 0;\nR0.H = return_args;\nR0.L = return_args;\nP0 = 5;\nEXCPT 0;\n'
 source=(ROOT/'research/blackfin/fm-foundation.s.in').read_text().replace('@FACTORY@',factory).replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*128));(out/'probe.s').write_text(source)
 def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=120,**kw)
 templates=ROOT/'research/blackfin'
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as f:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=f,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Both documented simulator patches required')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'voice-contract.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as f,(out/'probe.log').open('wb') as log:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000',out/'probe.elf'],stdout=f,stderr=log)
 if (out/'output.bin').stat().st_size!=2244+CASES*128:raise RuntimeError('Incomplete FM foundation output')
 artifacts={}
 for name,path in [('fm246-foundation-input.bin',out/'inputs.bin'),('fm246-foundation-output.bin',out/'output.bin')]:
  target=ROOT/'research/fm-foundation/vectors'/name
  if a.record:target.write_bytes(path.read_bytes())
  if target.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle mismatch: '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 report={'ldr_sha256':expected,'routines':{'constructor':'0x019a21d8','reset':'0x019a2248','six_envelopes':'0x019a22a4','table_copy':'0x019a3d84','table_conversion':'0x019a3de4'},'factory':'0x019a3d38','factory_slots':6,'table_prefix_bytes':60,'factory_prefix_bytes':2184,'cases':CASES,'state_bytes':60,'table_words':30,'table_pointer':'0xffb00168','clear_source_words':20,'authored_source_words':10,'source_words':source,'model_sha256':sha(a.table_model),'artifacts':artifacts,'tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'model':'GNU BF524 with both writeback patches; not hardware validation','limits':['Table boot conversion conditioned on ten authored binary32 values, twenty legible words locked.','Per-record six-envelope tests use explicit synthetic table words.','Not the FM audio callback, host timing/allocator/ADSR or acoustic validation.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: FM original boot conversion / table copy / constructor / reset / six envelopes: 8192 records')
if __name__=='__main__':main()
