#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Bounded address inventory, not a control-flow or producer recovery.
Never decodes protected L1 bytes. Publishes addresses/metadata, not instructions.
"""
import argparse,hashlib,importlib.util,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
TARGETS={
 'core_mode':0xff801038,'drum_occupancy':0xff801070,
 'drum_event_velocity':0xff8012ec,'drum_event_pitch':0xff801304,
 'drum_voice_pad':0xff801334,'drum_audio_objects':0x01aaad30,
 'drum_vtable':0x0190fac4,'drum_render':0x019789ec,
 'core_signal_callback':0x019674f8,'core_signal_coefficient':0x01bab53c,
 'core_signal_history':0x01aaac14,'core_signal_input':0xff801e50,
 'core_signal_output':0xff800bb4}
FUNCTIONS={'drum_render','core_signal_callback'}
def scan(image,base,visible,relative_calls):
 result={name:{'target':hex(target),'low_half_candidates':[],'high_first_pairs':[],'literal_words':[],'relative_calls':[]} for name,target in TARGETS.items()}
 bylow={}
 for name,target in TARGETS.items():bylow.setdefault(target&65535,[]).append(name)
 for offset in range(0,len(image)-3,2):
  if not visible(offset,4):continue
  opcode,low=struct.unpack_from('<HH',image,offset)
  if opcode&0xffe0==0xe100 and low in bylow:
   for name in bylow[low]:
    item={'address':hex(base+offset),'adjacent_high_visible':visible(offset+4,4)}
    if item['adjacent_high_visible']:
     highop,high=struct.unpack_from('<HH',image,offset+4)
     same=highop==opcode+0x40
     item.update(adjacent_same_register_high=same,resolved_address=hex((high<<16)|low) if same else None,matches_target=((high<<16)|low)==TARGETS[name] if same else False)
    else:item.update(adjacent_same_register_high=None,resolved_address=None,matches_target=None)
    item['adjacent_indirect_call']=None
    register=opcode&31
    if item['matches_target'] and 8<=register<=13 and visible(offset+8,2):
     call,=struct.unpack_from('<H',image,offset+8)
     if call==0x60+register-8:item['adjacent_indirect_call']=hex(base+offset+8)
    result[name]['low_half_candidates'].append(item)
  if opcode&0xffe0==0xe140 and visible(offset+4,4):
   lowop,value=struct.unpack_from('<HH',image,offset+4)
   if lowop==opcode-0x40:
    target=(low<<16)|value
    for name,wanted in TARGETS.items():
     if target==wanted:result[name]['high_first_pairs'].append(hex(base+offset))
  if offset%4==0:
   literal=struct.unpack_from('<I',image,offset)[0]
   for name,target in TARGETS.items():
    if literal==target:result[name]['literal_words'].append(hex(base+offset))
  if relative_calls and opcode&0xff00==0xe300:
   displacement=((opcode&255)<<16)|low
   if displacement&0x800000:displacement-=0x1000000
   destination=(base+offset+2*displacement)&0xffffffff
   for name in FUNCTIONS:
    if destination==TARGETS[name]:result[name]['relative_calls'].append(hex(base+offset))
 return result

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--ldr',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();out=a.output.resolve()
 if out.exists() or out.is_relative_to(ROOT):raise ValueError('Use a new private output directory outside checkout')
 raw=a.ldr.read_bytes();digest=hashlib.sha256(raw).hexdigest();expected=json.loads((ROOT/'docs/firmware/op1-246-ldr-map.json').read_text())['sha256']
 if digest!=expected:raise ValueError('Unsupported firmware')
 spec=importlib.util.spec_from_file_location('mapper',ROOT/'scripts/map-op1-ldr.py');mapper=importlib.util.module_from_spec(spec);spec.loader.exec_module(mapper);meta,_=mapper.inspect(raw);base=int(meta['sdram_start'],16);image=bytearray(int(meta['sdram_end'],16)-base);known=bytearray(len(image));cursor=0;l1=[]
 while cursor<len(raw):
  flags,address,n,arg=struct.unpack_from('<4I',raw,cursor);fill=bool(flags&0x100)
  if base<=address and address+n<=base+len(image) and not flags&0x1000:
   image[address-base:address-base+n]=(struct.pack('<I',arg)*((n+3)//4))[:n] if fill else raw[cursor+16:cursor+16+n]
   known[address-base:address-base+n]=b'\x01'*n
  if address==0xffa00000 and n==49076 and not fill:
   payload=raw[cursor+16:cursor+16+n]
   # The previously inventoried community profile is an assumption, not a key.
   def visible(i,size):return i>=0 and i+size<=len(payload) and all(k%24>=8 for k in range(i,i+size))
   l1.append({'header_offset':hex(cursor),'destination':hex(address),'bytes':n,'flags':hex(flags),'relative_calls_scanned':False,'targets':scan(payload,address,visible,False)})
  cursor+=16+(0 if fill else n)
 if len(l1)!=1:raise RuntimeError('Expected exactly the inventoried L1 block')
 def visible_sdram(i,size):return i>=0 and i+size<=len(image) and all(known[i:i+size])
 for block in l1:
  for target in block['targets'].values():
   for item in target['low_half_candidates']:
    group=(int(item['address'],16)-0xffa00000)//24*24+0xffa00000
    item['neighboring_protected_spans']=[[hex(group),hex(group+8)],[hex(group+24),hex(group+32)]]
 report={'ldr_sha256':digest,'sdram':{'base':hex(base),'bytes':len(image),'known_bytes':sum(known),'relative_calls_scanned':True,'targets':scan(image,base,visible_sdram,True)},'l1':l1,'protection_assumption':'For the inventoried 49076-byte L1 block only: first 8 bytes in each 24-byte group protected. Protected bytes are never decoded; visibility does not establish executable instructions.','limits':['Only adjacent low/high or high/low immediate pairs, aligned literal words and SDRAM relative CALL encodings are inventoried. Gapped pairs, arithmetic, aliases, indirect calls and data flow are not reconstructed.','A low half alone or a resolved different high half is not a reference to the requested target. Full pairs and pointers remain candidates, not verified reads/writes or callers.','Relative CALL encodings may occur in data; absence of direct calls does not prove absence of a caller or an L1 dependency. No L1 instructions executed or decrypted.','Input/output buffer roles, sample rate, routing, per-sample drum gain/pitch producers and scheduling remain unproven.']}
 out.mkdir(parents=True,mode=0o700);(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: bounded SDRAM/L1 inventory for 13 drum/Core targets; no protected-byte decoding')
if __name__=='__main__':main()
