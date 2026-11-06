#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce complete drum note-on words over all raw velocity halfwords."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=65536
STRIDE=24
OUT=1076
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 data=bytearray();seed=0x44524556
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for bank in range(8):
  for pad in range(24):data+=struct.pack('<8h',(pad-12)*1024+256,2+pad,32+pad,((bank+pad)%3)*8192,((bank+pad)%2)*16384,(bank*113+pad*37)%32768,(bank*353+pad*71)%32768,16384)
 for n in range(CASES):
  data+=struct.pack('<6I',n//256%8,(((n&255)*73+(n>>8)*19+37)%256-128)&0xffffffff,53+(n//8)%24,(rand()&0xffff0000)|n,rand(),rand())
 assert len(data)==3072+CASES*STRIDE
 return data
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,count,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+count<=base+len(image) and not flags&0x1000:image[address-base:address-base+count]=(struct.pack('<I',arg)*((count+3)//4))[:count] if flags&0x100 else raw[cursor+16:cursor+16+count]
  cursor+=16+(0 if flags&0x100 else count)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(inputs());templates=ROOT/'research/blackfin'
 def execute(stem,source,linker='flat.ld',extra=()):
  def run(cmd,**kw):subprocess.run(list(map(str,cmd)),check=True,timeout=300,**kw)
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as f,(out/(stem+'.log')).open('wb') as log:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=f,stderr=log)
  return out/(stem+'.bin')
 for name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  if struct.unpack('<5I',execute(Path(name).stem,(templates/name).read_text()).read_bytes())!=probe:raise RuntimeError('All three verified writeback patches required')
 source=(templates/'drum-event-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*OUT))
 result=execute('drum-event246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=CASES*OUT:raise RuntimeError('Incomplete original event output')
 raw_result=result.read_bytes();raw_input=(out/'inputs.bin').read_bytes();words=[None]*256
 for n in range(CASES):
  index=struct.unpack_from('<i',raw_input,3072+n*STRIDE+4)[0]+128
  word=struct.unpack_from('<I',raw_result,n*OUT+956)[0]
  if words[index] is not None and word!=words[index]:raise RuntimeError('Pitch word depends on another supplied input')
  words[index]=word
 if any(word is None for word in words):raise RuntimeError('Incomplete transposition domain')
 table='// SPDX-License-Identifier: AGPL-3.0-only\n// Derived discrete outputs of original rev246 note-on; not an original ROM table.\n#pragma once\n#include <array>\n#include <cstdint>\nnamespace pocket::reference {\ninline constexpr std::array<uint32_t,256> drum246EventPitchWords{{\n'
 for i in range(0,256,8):table+='    '+','.join(f'0x{x:08x}u' for x in words[i:i+8])+',\n'
 table+='}};\n}\n'
 table_path=ROOT/'Source/Drum246EventTables.h'
 if a.record:table_path.write_text(table)
 if table_path.read_text()!=table:raise RuntimeError('Derived pitch table differs')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/drum-event/vectors'/f'drum-event246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_prefix_bytes':3072,'input_record_bytes':STRIDE,'output_record_bytes':OUT,'note_on':'0x01a84ba0','velocity_word':'0xff8012ec','pitch_word':'0xff801304','transpose_domain':[-128,127],'velocity_domain':'all 65536 low-halfword patterns with varied upper bits','derived_table':{'path':str(table_path.relative_to(ROOT)),'sha256':sha(table_path),'unique_pitch_words':len(set(words))},'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Complete unmodified note-on with type-1 drum mode, type-8 sampler, empty held registry and busy flags, preview -1 and null LFO/FX; factory sound 0 references supplied.','Explicit control banks and event order; state, 812 global bytes, first retained row/count recorded. No render or produced pitch/gain buffers.','Lookup table is derived output for a bounded integer domain, not an original firmware table; no claim outside -128..127 semitones.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 65536 complete original drum note-on cases, all velocity halfwords and 256 transpositions')
if __name__=='__main__':main()
