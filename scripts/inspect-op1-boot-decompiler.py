#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce private rev246 boot decompilation and synthetic-key XTEA checks.
Requires caller-installed Ghidra 12.2/Blackfin 1.2.0, JDK25 and GNU Blackfin.
Never boots firmware, calls OTP/MMIO, extracts real keys or modifies the plugin.
"""
import argparse,ctypes,hashlib,io,json,lzma,os,struct,subprocess,tarfile,zlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
FIRMWARE='c5315218f825f143b415ca554516541898abee843d3a236df0b54c04e1fb13a9'
BOOT='f6233b974bdb6c92f39e73401f206721924ef3ce52abd66b642a5350cebcad93'
LIMIT=128*1024*1024

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def blocks(raw):
 cursor=0;rows=[]
 while cursor<len(raw):
  if cursor+16>len(raw):raise ValueError('Truncated LDR header')
  flags,address,n,value=struct.unpack_from('<4I',raw,cursor)
  checksum=0
  for byte in raw[cursor:cursor+16]:checksum^=byte
  if flags>>24!=0xad or checksum:raise ValueError('Invalid LDR header')
  end=cursor+16+(0 if flags&0x100 else n)
  if end>len(raw):raise ValueError('Truncated LDR payload')
  rows.append((cursor,flags,address,n,value,raw[cursor+16:end]));cursor=end
 return rows

def decipher(a,b,key,rounds):
 mask=0xffffffff;total=(rounds*0x9e3779b9)&mask
 for _ in range(rounds):
  b=(b-((((a<<4)^(a>>5))+a)^(total+key[(total>>11)&3])))&mask
  total=(total-0x9e3779b9)&mask
  a=(a-((((b<<4)^(b>>5))+b)^(total+key[total&3])))&mask
 return a,b

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ['firmware','output','ghidra','jdk','assembler','linker','simulator','testutils']:p.add_argument('--'+name,type=Path,required=True)
 args=p.parse_args();out=args.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a NEW private output directory outside repository')
 if args.firmware.stat().st_size>LIMIT or sha(args.firmware)!=FIRMWARE:raise ValueError('Only inventoried rev246 container supported')
 raw=args.firmware.read_bytes()
 if struct.unpack_from('<I',raw)[0]!=zlib.crc32(raw[4:]):raise ValueError('Container CRC mismatch')
 decoder=lzma.LZMADecompressor(format=lzma.FORMAT_ALONE,memlimit=LIMIT);archive=decoder.decompress(raw[4:],max_length=LIMIT+1)
 if not decoder.eof or decoder.unused_data or len(archive)>LIMIT:raise ValueError('Invalid LZMA profile')
 with tarfile.open(fileobj=io.BytesIO(archive)) as tf:
  matches=[m for m in tf.getmembers() if m.name=='te-boot.ldr']
  if len(matches)!=1 or not matches[0].isfile() or not 0<matches[0].size<=LIMIT:raise ValueError('Invalid boot member')
  boot=tf.extractfile(matches[0]).read(LIMIT+1)
 if hashlib.sha256(boot).hexdigest()!=BOOT:raise ValueError('Boot SHA256 mismatch')
 os.umask(0o077);out.mkdir(parents=True,mode=0o700);(out/'te-boot.ldr').write_bytes(boot);rows=blocks(boot)
 # Keep initial loader and final runtime overlays separate: addresses overlap.
 selected={0x10:('init-data.bin',0xff804000,188),0xdc:('init-stage.bin',0xffa00000,2508),0xab8:('init-callback.bin',0xffa10000,296),0xc10:('runtime-low.bin',4,168592),0x33d10:('runtime-l1.bin',0xffa00000,49076)}
 for offset,flags,address,n,value,data in rows:
  if offset in selected:
   name,want_address,want_n=selected[offset]
   if flags&0x100 or (address,n)!=(want_address,want_n):raise ValueError('Boot layout mismatch')
   (out/name).write_bytes(data)
 for name,_,_ in selected.values():
  if not (out/name).exists():raise ValueError('Missing observed boot region')
 def run(command,log,**kw):
  with (out/log).open('w') as f:subprocess.run([str(x) for x in command],check=True,stdout=f,stderr=subprocess.STDOUT,timeout=300,**kw)
 def elf(name,sections,entry):
  source='';layout='ENTRY('+entry+')\nSECTIONS { '
  for section,address,filename in sections:
   source+=f'.section .{section},"awx"\n.incbin "{out/filename}"\n'
   layout+=f'.{section} {hex(address)} : {{ *(.{section}) }} '
  source+=f'.global {entry}\n.set {entry}, {hex(sections[0][1])}\n'
  (out/(name+'.s')).write_text(source);(out/(name+'.ld')).write_text(layout+'}\n')
  run([args.assembler,'-o',out/(name+'.o'),out/(name+'.s')],name+'-as.log')
  run([args.linker,'-T',out/(name+'.ld'),'-o',out/(name+'.elf'),out/(name+'.o')],name+'-ld.log')
 elf('init',[('init_stage',0xffa00000,'init-stage.bin'),('callback',0xffa10000,'init-callback.bin'),('init_data',0xff804000,'init-data.bin')],'init_entry')
 elf('runtime',[('runtime_low',4,'runtime-low.bin'),('runtime_l1',0xffa00000,'runtime-l1.bin')],'runtime_entry')
 (out/'projects').mkdir();(out/'home').mkdir()
 env=dict(os.environ);env['JAVA_HOME']=str(args.jdk.resolve());env['JAVA_TOOL_OPTIONS']=env.get('JAVA_TOOL_OPTIONS','')+f' -Duser.home={out/"home"} -XX:ActiveProcessorCount=2';env['GHIDRA_HEADLESS_MAXMEM']='1536M'
 targets={'init':['ffa00000','ffa00514','ffa10000','ffa1007e'],'runtime':['ffa0bee6','1da']}
 for name,addresses in targets.items():
  before=sha(out/(name+'.elf'))
  run([args.ghidra/'support/analyzeHeadless',out/'projects',name,'-import',out/(name+'.elf'),'-processor','Blackfin:LE:32:default','-cspec','default','-noanalysis','-scriptPath',ROOT/'research/blackfin','-postScript','DecompileBoot246.java',out,*addresses],name+'-ghidra.log',env=env)
  log=(out/(name+'-ghidra.log')).read_text()
  for address in addresses:
   if 'DECOMPILE_COMPLETED '+address+' ' not in log or not (out/(address+'.c')).is_file():raise RuntimeError('Incomplete Ghidra result: '+address)
  if before!=sha(out/(name+'.elf')):raise RuntimeError('Original ELF bytes changed')
 # Synthetic inputs only; no values read from OTP or application-key state.
 data=bytearray();seed=0x58544541
 def rnd():
  nonlocal seed
  seed=(seed*1664525+1013904223)&0xffffffff;return seed
 for c in range(1024):
  rounds=[0,1,2,16,32,64][c%6];block=[rnd(),rnd()];key=[rnd() for _ in range(4)]
  if c<6:block=[0,0];key=[0]*4
  data+=struct.pack('<10I',rounds,0x13579bdf,*block,*key,0x2468ace0,0xdeadbeef)
 (out/'inputs.bin').write_bytes(data)
 (out/'probe.s').write_text((ROOT/'research/blackfin/boot-xtea.s.in').read_text().replace('@ROOT@',str(out)))
 (out/'probe.ld').write_text('ENTRY(__start)\nSECTIONS { .text 0x04000000 : { *(.text) } .data 0x04100000 : { *(.data) } .plain 0xffa0bee6 : { *(.plain) } .optimized 0xffa10000 : { *(.optimized) } }\n')
 run([args.assembler,'-I',args.testutils,'-o',out/'probe.o',out/'probe.s'],'probe-as.log')
 run([args.linker,'-T',out/'probe.ld','-o',out/'probe.elf',out/'probe.o'],'probe-ld.log')
 with (out/'output.bin').open('wb') as stdout,(out/'sim.log').open('wb') as stderr:
  subprocess.run([str(x) for x in [args.simulator,'--model','bf524','--memory-size','128M','--memory-region','0xffa00000,0x20000','--watch-cycles-int','900000000',out/'probe.elf']],check=True,stdout=stdout,stderr=stderr,timeout=180)
 output=(out/'output.bin').read_bytes()
 if len(output)!=81920:raise RuntimeError('Incomplete original numeric output')
 for c in range(1024):
  x=data[c*40:(c+1)*40];v=struct.unpack('<10I',x);expected=bytearray(x);struct.pack_into('<2I',expected,8,*decipher(v[2],v[3],v[4:8],v[0]))
  for variant in range(2):
   if output[c*80+variant*40:c*80+(variant+1)*40]!=expected:raise RuntimeError(f'Original cipher mismatch: {c}/{variant}')
 # Compile only the two inspected self-contained arithmetic functions privately.
 # Host pointer width and defined wrapping signed arithmetic are explicit adapters.
 for address in ['ffa0bee6','ffa10000']:
  code=(out/(address+'.c')).read_text()
  if 'UNRECOVERED_JUMPTABLE' in code or 'int param_3' not in code:raise RuntimeError('Review changed decompiler output before host adaptation')
  code=code.replace('int param_3','uintptr_t param_3')
  (out/'host.c').write_text('#include <stdint.h>\ntypedef uint32_t uint;\n'+code)
  run(['cc','-O2','-fwrapv','-shared','-fPIC',out/'host.c','-o',out/(address+'.so')],address+'-cc.log')
  library=ctypes.CDLL(str(out/(address+'.so')));function=getattr(library,'target_'+address);function.argtypes=[ctypes.c_uint32,ctypes.POINTER(ctypes.c_uint32),ctypes.c_size_t];function.restype=None
  for c in range(1024):
   v=struct.unpack_from('<10I',data,c*40);block=(ctypes.c_uint32*2)(*v[2:4]);key=(ctypes.c_uint32*4)(*v[4:8]);function(v[0],block,ctypes.addressof(key))
   if bytes(block)!=output[c*80+8:c*80+16]:raise RuntimeError(f'Ghidra arithmetic mismatch: {address}/{c}')
 report={'firmware_sha256':FIRMWARE,'boot_sha256':BOOT,'decompiled_entries':targets,'original_xtea_calls':2048,'synthetic_inputs':1024,'ghidra_host_comparisons':2048,'rounds':[0,1,2,16,32,64],'input_sha256':sha(out/'inputs.bin'),'output_sha256':sha(out/'output.bin'),'source_sha256':{str(p.relative_to(ROOT)):sha(p) for p in [Path(__file__).resolve(),ROOT/'research/blackfin/DecompileBoot246.java',ROOT/'research/blackfin/boot-xtea.s.in']},'tools':{str(p):sha(p) for p in [args.assembler,args.linker,args.simulator]},'limits':['Synthetic keys only; no OTP or MMIO was executed and no application key or decrypted L1 was recovered.','Annotated observed indirect-return epilogues in analysis metadata only. ELF executable bytes unchanged.','Decompiler arithmetic host adaptation widens its key pointer and defines signed wrap; not copied into product code.','GNU BF524 and Blackfin p-code are analysis models, not hardware/boot validation. Original pseudocode, ELF and outputs stay private.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n')
 print('PASS: six private Ghidra decompilations; 2048 original XTEA calls; 2048 decompiled-host comparisons; synthetic keys only')
if __name__=='__main__':main()
