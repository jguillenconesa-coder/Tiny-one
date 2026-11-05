#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded Drum VoiceCtl bank transitions, held-note reset and registry clear."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=8192
STRIDE=1156
OUT=5292
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 spec=importlib.util.spec_from_file_location('voice',ROOT/'scripts/verify-drum-voice246.py');voice=importlib.util.module_from_spec(spec);spec.loader.exec_module(voice);base=voice.inputs()
 data=bytearray(base[:19968])
 for n in range(CASES):
  row=bytearray(base[19968+(n%4096)*STRIDE:19968+(n%4096+1)*STRIDE]);step=n%64;scenario=n//64
  if n<4096:
   flags=3|1024|(32 if n%3 else 0)|(16 if n%4 else 0)
   for v in range(12):row[280+v]=[1,127,128,255][(n+v)%4] if n&(1<<v) else 0
  else:
   flags=struct.unpack_from('<I',row)[0]|1024
   if step in (24,35):flags|=256
   if step in (25,36):flags|=512
   if step in (26,37,48,62):flags|=64
   if step in (28,44):flags=(flags|128)&~(4|8|16|32)
   if step in (27,43):
    flags=(flags&~4)|8;struct.pack_into('<I',row,24,60 if step==27 else 70)
   if step==29:struct.pack_into('<I',row,24,60)
   sound=(scenario+(1 if step>=25 else 0)+(1 if step>=36 else 0))%8
   struct.pack_into('<I',row,4,sound)
  struct.pack_into('<I',row,0,flags);data+=row
 assert len(data)==19968+CASES*STRIDE
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
 setup=''
 for i in range(1025):setup+=f'imm32 P0,{0x018bf010+i*4}\nR0=[P0];\nimm32 P0,0x01aa7014\nCALL(P0);\nimm32 P0,{0xff802154+i*2}\nW[P0]=R0.L;\n'
 setup+='R0.H=coeff_args;\nR0.L=coeff_args;\nP0=5;\nEXCPT 0;\n'
 handles='.section .handles,"aw"\n'+''.join(f'.long {0x02010004+i*8},samples+{i*2112+32}\n' for i in range(8))+'.data\n'
 source=(templates/'drum-bank-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@SETUP@',setup).replace('@HANDLES@',handles).replace('@BYTES@',str(CASES*OUT))
 result=execute('drum-bank246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=2052+CASES*OUT+16896:raise RuntimeError('Incomplete Core drum output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/drum-bank/vectors'/f'drum-bank246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_prefix_bytes':19968,'input_record_bytes':STRIDE,'output_prefix_bytes':2052,'output_record_bytes':OUT,'output_suffix_bytes':16896,'core_factory':'0x01967984','factory_helper':'0x01a1b9c0','note_on':'0x01a84ba0','note_off':'0x01a84dac','activity_query':'0x01967308','held_reset':'0x01a856bc','registry_clear':'0x01a8624c','begin_bank':'0x01a85df8','finish_factory':'0x01a85e40','busy_query':'0x01a27e2c','external_note_flags':'0x01aafd60','midi_output_word':'0x01dfe03c','core_reset':'0x01967ccc','core_release':'0x01967d64','producer_table':'0x01aaad30','selected_sound':'0x01aaac10','voice_pad_table':'0xff801334','control_banks':'0x01aaadb4','sample_references':'0x01cc10cc','type':8,'voices':12,'sounds':8,'pads':24,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Complete original registry clear, held reset, bank begin/factory finish and busy query; originals note-on/off, activity and optional sampler render composed at supplied order.','MIDI output word zero: original external-note helper clears supplied flags, without transmitter/IPC. Null modulation objects and FX; type-8 sampler and type-1 drum mode.','Only five octave transpositions; PCM/padding/pitch/gain/cadence and bank sequence explicit. Not normal render retirement, gain/pitch buffer producer or original scheduler.','Research only; instrument unchanged, no installer build.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 8192 original drum bank/registry cases, all 4096 busy masks and sequenced lifecycle/render')
if __name__=='__main__':main()
