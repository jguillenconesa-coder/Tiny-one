#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Replay DNA render with explicit identity bytes and synthetic live L1 RAM.
The OTP reader is not emulated: the original post-reader initializer is entered
with a caller-supplied frame. Proprietary code/ELFs remain in private outputs.
"""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4096

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x444e4132
 def rnd():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  segment=n//64 if n<2048 else 32+(n-2048)//256
  pitch=(0x40000,0x70000,0x146f108,0x2000000,0x20000000,0x7fffffff)[segment%6]
  period=max(0x20000,min(0x30000000,(1<<46)//pitch))
  blue=(-29491,-24576,-1,0,1,8191,8192,32767)[n%8]
  green=(4608,5120,8192,10240,12800)[n//8%5]
  white=(0,1,2047,2048,16384,30720,32767)[n//40%7]
  orange=(0,1,8192,24000,32767)[n//280%5]
  pool=bytearray()
  for slot in range(6):
   a,b=rnd()%period,rnd()%period
   limit=max(1,min(period>>12,0x3ffff000>>(white>>11)))
   counter=rnd()%max(1,min(limit<<(white>>11),131072))
   pool+=struct.pack('<5Ihh2I',rnd(),a,b,counter,rnd(),int(rnd()%65536)-32768,int(rnd()%65536)-32768,rnd(),(rnd()&0xffffff00)|(n%2))
  action=1 if n%64==0 and n<2048 or n%256==0 else 0
  if n%11==0:action|=2
  if n%37==0:action|=4
  if n%101==0:action|=8
  identity=('00000000000000000000000000000000','ffffffffffffffffffffffffffffffff','0123456789abcdef0123456789abcdef','706f636b65746f6e652d646e612d7630')[segment%4].encode()
  data+=pool+struct.pack('<4h6I',blue,green,white,orange,pitch,rnd(),action,rnd(),n%6,segment%2)+identity
 assert len(data)==CASES*256
 return data

def ram_probe_inputs():
 data=bytearray()
 for profile in (0,1):
  pool=b''.join(struct.pack('<5Ihh2I',0x019102a4,0,0,256 if slot==0 else 0,0,0,0,0,0) for slot in range(6))
  data+=pool+struct.pack('<4h6I',0,8192,0,32767,0x146f108,123,1,0x1234,0,profile)+b'0'*32
 return data

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--ram-probe',action='store_true',help='Paired original callbacks differing only in RAM outside the live pool')
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
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);data=ram_probe_inputs() if a.ram_probe else inputs();cases=len(data)//256;(out/'inputs.bin').write_bytes(data)
 loads=sum(bool(struct.unpack_from('<I',data,n*256+208)[0]&1) for n in range(cases))
 setup='imm32 P0,0x01a1e914\nCALL (P0);\n'
 for k,(src,dst,count) in enumerate([(0x018be948,0x0106a5ec,129),(0x018bebcc,0x0106a730,225)]):
  setup+=f'imm32 P4,{src}\nimm32 I0,{dst}\nP3 = {count};\nLSETUP(table{k},table{k}_end) LC0 = P3;\ntable{k}: R0 = [P4++];\nimm32 P0,0x01aa7014\nCALL (P0);\ntable{k}_end: W[I0++] = R0.L;\nR0.H = table{k}_args;\nR0.L = table{k}_args;\nP0 = 5;\nEXCPT 0;\n'
 setup+='R0.H = range_args;\nR0.L = range_args;\nP0 = 5;\nEXCPT 0;\n'
 size=cases*492+loads*512+(cases//256)*(196608+24576)
 source=(ROOT/'research/blackfin/dna-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@SETUP@',setup).replace('@BYTES@',str(size));(out/'probe.s').write_text(source)
 def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=180,**kw)
 templates=ROOT/'research/blackfin'
 run([a.assembler,'-I',a.testutils,'-o',out/'halfword.o',templates/'parallel-halfword.s']);run([a.linker,'-T',templates/'flat.ld','-o',out/'halfword.elf',out/'halfword.o'])
 with (out/'halfword.bin').open('wb') as f:run([a.simulator,'--model','bf524',out/'halfword.elf'],stdout=f,stderr=subprocess.DEVNULL)
 if struct.unpack('<5I',(out/'halfword.bin').read_bytes())!=(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd):raise RuntimeError('Both documented simulator patches required')
 run([a.assembler,'-I',a.testutils,'-o',out/'probe.o',out/'probe.s']);run([a.linker,'-T',templates/'cwo-foundation.ld','-o',out/'probe.elf',out/'probe.o'])
 with (out/'output.bin').open('wb') as f,(out/'probe.log').open('wb') as log:run([a.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--memory-region','0xffb01000,0x2f000','--watch-cycles-int','1500000000',out/'probe.elf'],stdout=f,stderr=log)
 if (out/'output.bin').stat().st_size!=772+size:raise RuntimeError('Incomplete DNA output '+str((out/'output.bin').stat().st_size))
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',out/'output.bin')]:
  name=('dna246-ram-probe-' if a.ram_probe else 'dna246-')+suffix+'.bin';target=ROOT/'research/dna-render/vectors'/name
  if a.record:target.write_bytes(path.read_bytes())
  if target.read_bytes()!=path.read_bytes():raise RuntimeError('Oracle mismatch '+name)
  artifacts[name]={'bytes':path.stat().st_size,'sha256':sha(path),'matches_committed_fixture':True}
 report={'ldr_sha256':expected,'cases':cases,'samples':cases*128,'state_bytes':32,'pool_bytes':192,'wave_bytes':24576,'source_ram_bytes':196608,'complete_memory_snapshots':cases//256,'identity_initializations':loads,'routines':{'render':'0x019b48d8','factory':'0x019b4c64','reset':'0x019b4888','release':'0x019b4844','wave_clear':'0x019b4d4e','identity_fragment':'0x019b4cd4..0x019b4d40','hardware_identity_reader_not_executed':'0x019f16a2'},'artifacts':artifacts,'tools':{str(x):sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Four synthetic identities and explicit RAM images, not original physical OTP or a hardware RAM capture.','Original live six-slot pool aliases the first 192 RAM bytes; other RAM is controlled zero/LCG data.','Positive pitch >=0x40000, valid phases/counters and original four-control descriptor bounds; caller timing and device boot not established.']}
 if a.ram_probe:
  raw=(out/'output.bin').read_bytes();energies=[]
  for index in range(2):
   offset=772+index*(512+492)+512+232
   samples=struct.unpack_from('<128h',raw,offset);energies.append(sum(v*v for v in samples))
  if energies[0]!=0 or energies[1]==0:raise RuntimeError('RAM dependence probe did not isolate a response')
  report['paired_ram_probe']={'difference':'Only non-pool RAM bytes change from zero to seeded LCG; same pool/identity/controls/pitch/RNG','audio_sum_squares':energies}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print(f'PASS: DNA original identity fragment/lifecycle/render, {cases} blocks and {cases//256} full memory snapshots')
if __name__=='__main__':main()
