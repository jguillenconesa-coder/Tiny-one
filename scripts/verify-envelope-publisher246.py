#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Reproduce bounded Envelope publisher modulation observations using privately supplied rev246."""
import argparse,hashlib,importlib.util,json,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 seed=0x5055424c;data=bytearray()
 def rand():
  nonlocal seed
  seed=(1664525*seed+1013904223)&0xffffffff;return seed
 def signed():return int(rand()%65536)-32768
 waves=[]
 for k in range(16):
  waves.append([0 if k==0 else 32767 if k==1 else -32768 if k==2 else (32767 if i%2 else -32768) if k==3 else (16000 if i==k*7%128 else 0) if k<8 else signed() for i in range(128)])
 for case in range(4096):
  action=((case%4)<<2)|(1 if case<1024 or case==1024 else 0)|(2 if case%37==0 else 0)
  state=[rand(),rand(),rand()]
  if case%8==0:state=[0,0,0]
  if case<1024 and case%16==4:
   edge=[0,1,0x7fffffff,0x80000000,0xffffffff,0x7ffffffe,0x80000001];j=case//16;state=[edge[j%7],edge[(j+2)%7],edge[(j+4)%7]]
  c=[signed() for _ in range(4)] if case<1024 else [8192,16384,4096,[0,8192,16384,24576,32767][case//128%5]]
  if case<1024 and case%16==0:c=[[-32768,-1,0,1,16384,32767][case//16%6]]*4
  if case>=3840:c=[0,16384,0,0]
  audio=waves[case%16 if case<1024 else (0 if case%256>=128 else 8+case//256%8)]
  if case>=3840:audio=waves[0]
  data+=struct.pack('<4I4h128h',action,*state,*c,*audio)
 return data

def composition_inputs():
 """Artificial before/after scheduling; no hardware caller is inferred."""
 raw=inputs();data=bytearray();seed=0x434f4d50
 for case in range(4096):
  values=[]
  for _ in range(16):
   seed=(1664525*seed+1013904223)&0xffffffff;values.append((seed>>16)-32768)
  family=1+case%2;type=8+case//2%2 if family==1 else case//2%8
  kind=case//4%4 if family==1 else 5
  dest=([0,1,2,-1] if family==1 else [2,0,1,-1])[case//16%4]
  colour=case//64%8;amount=[-32768,-32767,-16384,-1,0,1,16384,32767][case//512%8]
  controls=[4096,amount,dest*2048,colour*(2048 if family==1 and dest==0 else 4096),*values[:4]]
  data+=raw[case*280:(case+1)*280]+struct.pack('<4I8h8h',family,type,kind,0x12340000+case,*controls,*values[8:])
 return data

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for key in ['ldr','assembler','linker','simulator','testutils','output']:p.add_argument('--'+key,type=Path,required=True)
 p.add_argument('--composition',action='store_true');p.add_argument('--record',action='store_true');a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if sha(a.ldr)!=expected:raise ValueError('Unsupported firmware')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 raw=a.ldr.read_bytes();meta,_=m.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);cursor=0
 while cursor<len(raw):
  flags,address,count,arg=struct.unpack_from('<4I',raw,cursor)
  if base<=address and address+count<=base+len(image) and not flags&0x1000:image[address-base:address-base+count]=(struct.pack('<I',arg)*((count+3)//4))[:count] if flags&0x100 else raw[cursor+16:cursor+16+count]
  cursor+=16+(0 if flags&0x100 else count)
 out.mkdir(parents=True);(out/'sdram.bin').write_bytes(image);(out/'inputs.bin').write_bytes(composition_inputs() if a.composition else inputs());templates=ROOT/'research/blackfin'
 def execute(stem,source,linker='voice-contract.ld',extra=()):
  def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,timeout=300,**kw)
  (out/(stem+'.s')).write_text(source);run([a.assembler,'-I',a.testutils,'-o',out/(stem+'.o'),out/(stem+'.s')]);run([a.linker,'-T',templates/linker,'-o',out/(stem+'.elf'),out/(stem+'.o')])
  with (out/(stem+'.bin')).open('wb') as stdout,(out/(stem+'.log')).open('wb') as stderr:run([a.simulator,'--model','bf524',*extra,out/(stem+'.elf')],stdout=stdout,stderr=stderr)
  return out/(stem+'.bin')
 for name,probe in [('parallel-halfword.s',(0x1234,0x0200abcd,0x1234,0xabcd0300,0x2468abcd)),('parallel-fp-load.s',(0x1234,0xaabbccdd,0x1357,0xaabbccdd,6))]:
  path=execute(Path(name).stem,(templates/name).read_text(),'flat.ld')
  if struct.unpack('<5I',path.read_bytes())!=probe:raise RuntimeError('All three documented writeback patches required')
 stride=704 if a.composition else 312;input_stride=328 if a.composition else 280;prefix='envelope-publisher246-element' if a.composition else 'envelope-publisher246';publisher_offset=40 if a.composition else 0
 size=4096*stride;source=(templates/('envelope-publisher-element-stream.s.in' if a.composition else 'envelope-publisher-stream.s.in')).read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(size))
 result=execute(prefix,source,linker='element-lfo.ld' if a.composition else 'voice-contract.ld',extra=('--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000'))
 if result.stat().st_size!=size:raise RuntimeError('Incomplete output')
 raw_input=(out/'inputs.bin').read_bytes();raw_output=result.read_bytes()
 silent_positive=sum(not any(raw_input[n*input_stride+24:n*input_stride+280]) and struct.unpack_from('<i',raw_output,n*stride+publisher_offset+32)[0]>0 for n in range(4096))
 silent_getter=sum(not any(raw_input[n*input_stride+24:n*input_stride+280]) and struct.unpack_from('<h',raw_output,n*stride+publisher_offset+36)[0]>0 for n in range(4096))
 if silent_positive==0 or silent_getter==0:raise RuntimeError('Missing observed nonzero silent publication')
 # Gzip contains numerical observations only, never executable firmware bytes.
 import gzip
 artifacts={}
 for suffix,path in [('input',out/'inputs.bin'),('output',result)]:
  target=ROOT/'research/element-producers/vectors'/f'{prefix}-{suffix}.bin.gz';packed=gzip.compress(path.read_bytes(),mtime=0)
  if a.record:target.write_bytes(packed)
  if gzip.decompress(target.read_bytes())!=path.read_bytes():raise RuntimeError('Fixture differs: '+target.name)
  artifacts[target.name]={'bytes':len(packed),'raw_bytes':path.stat().st_size,'sha256':sha(target)}
 report={'ldr_sha256':expected,'callback':'0x01a66922','published_word':'0x01dfc80c','reset_published':'0x01a66bd8','getter':'0x01a66912','model':'BF524 simulator with three verified writeback patches; not hardware','blocks':4096,'samples':524288,'caller_states':4,'getter_calls':4096,'published_reset_calls':111,'silent_positive_blocks':silent_positive,'silent_positive_getter_blocks':silent_getter,'artifacts':artifacts,'tools':{Path(x).name:sha(x) for x in [a.assembler,a.linker,a.simulator]},'limits':['Explicit four two-word caller states sharing one publisher word, four signed controls and 128 audio halfwords. Full original routine, reset and getter executed without patching original code.','No recovered active caller, input routing, clock, control semantics or hardware acoustic equivalence. No production integration.']}
 if a.composition:
  accepted=sum(struct.unpack_from('<I',raw_output,n*704+offset+36)[0]!=0 for n in range(4096) for offset in [0,352])
  changed=sum(raw_output[n*704+20:n*704+36]!=raw_output[n*704+372:n*704+388] for n in range(4096))
  if accepted!=2048 or changed!=250:raise RuntimeError('Unexpected composed dispatch/scheduling coverage')
  if any(raw_output[n*704+40:n*704+352]!=raw_output[n*704+392:n*704+704] for n in range(4096)):raise RuntimeError('Consumer mutated publisher state or audio')
  report.pop('getter_calls');report.update(explicit_getter_calls=4096,consumer_getter_reads=accepted,total_getter_calls=4096+accepted,accepted_consumer_dispatches=accepted,blocks_with_different_before_after_targets=changed)
  report.update(consumer_calls=8192,consumer_callbacks={'drum':'0x019cde28','fx':'0x019cdf22'},scheduling='synthetic before and after each publisher block, identical initial consumer targets; not recovered hardware order')
  report['limits'].append('Only source 2 drum and FX kind 5 parameter dispatch; no original producer caller, scheduling, control ownership or acoustic equivalence.')
 (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 4096 original publisher blocks / 524288 samples, state/shared word/reset/getter and audio'+('; 8192 composed original Element calls under artificial scheduling' if a.composition else ''))
if __name__=='__main__':main()
