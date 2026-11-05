#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded Drum VoiceCtl note allocation, ownership and lifecycle."""
import argparse,gzip,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES=4096
STRIDE=1156
OUT=5156
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 spec=importlib.util.spec_from_file_location('core',ROOT/'scripts/verify-drum-core246.py');core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core)
 data=bytearray(core.inputs()[:19968]);seed=0x44564f49
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 for n in range(CASES):
  step=n%64;scenario=n//64
  flags=(3 if step==0 else 0)|(4 if step<32 or step%3==0 else 8)|(16 if n%4 else 0)|(32 if n%3 else 0)
  sound=scenario%8;factory=(sound+1)%8;voice=n%12;transpose=[-24,-12,0,12,24][n%5];preview=[-1,0,7,23][scenario%4]
  note=53+((step if step<24 else step-25 if step>=32 and flags&4 else step-24) %24)
  if step in (30,31,62,63):note=[52,77,-1,127][step%4]
  velocity=[0,1,64,127,-1,128,32767,-32768][n%8]
  pool=[]
  for v in range(12):pool.extend([0x0190fac4,0x02010000+(scenario+v)%8*8,(32+rand()%700)*2048,0 if v%5==0 else 1,([0,1,0x7fffffff,0x80000000,0xffffffff][(scenario+v)%5] if scenario%3==0 else rand()%100000)])
  busy=bytes([0 if scenario%4==0 and v>=scenario%12 else 1 for v in range(12)])
  owners=[(v+scenario)%24 for v in range(12)];pads=[(v*3+scenario)%24 for v in range(12)]
  pitch=[[0,0x04000000,0x08000000,0x10000000,0x7fffffff][(n+i//16)%5] for i in range(128)]
  gain=[int(rand()%65536)-32768 for _ in range(128)]
  data+=struct.pack('<6I4I60I',flags,sound,factory,voice,transpose&0xffffffff,preview&0xffffffff,note&0xffffffff,velocity&0xffffffff,(rand()&255),rand(),*pool)+busy+struct.pack('<24I128I128h',*owners,*pads,*pitch,*gain)
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
 source=(templates/'drum-voice-stream.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@SETUP@',setup).replace('@HANDLES@',handles).replace('@BYTES@',str(CASES*OUT))
 result=execute('drum-voice246',source,'drum-render.ld',('--memory-size','128M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','3500000000'))
 if result.stat().st_size!=2052+CASES*OUT+16896:raise RuntimeError('Incomplete Core drum output')
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/drum-voice/vectors'/f'drum-voice246-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Numeric fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'cases':CASES,'input_prefix_bytes':19968,'input_record_bytes':STRIDE,'output_prefix_bytes':2052,'output_record_bytes':OUT,'output_suffix_bytes':16896,'core_factory':'0x01967984','factory_helper':'0x01a1b9c0','note_on':'0x01a84ba0','note_off':'0x01a84dac','activity_query':'0x01967308','core_reset':'0x01967ccc','core_release':'0x01967d64','producer_table':'0x01aaad30','selected_sound':'0x01aaac10','voice_pad_table':'0xff801334','control_banks':'0x01aaadb4','sample_references':'0x01cc10cc','type':8,'voices':12,'sounds':8,'pads':24,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Original complete note-on/off and activity query, with null modulation objects/FX, type 1 drum mode, explicit preview selection and event order.','Transposition restricted to -24/-12/0/12/24 semitones; explicit PCM/padding/pitch/gain and optional render cadence. No gain producer or scheduler recovery.','Research only; no plugin integration or installers.']}
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 4096 original drum VoiceCtl cases, real note-on/off, activity and optional render')
if __name__=='__main__':main()
