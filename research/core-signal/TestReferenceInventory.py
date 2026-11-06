#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Authored synthetic encodings; no firmware. Guard every decoder read."""
import importlib.util,pathlib,struct,unittest
from unittest.mock import patch
ROOT=pathlib.Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('audit',ROOT/'scripts/audit-drum246-render-references.py');audit=importlib.util.module_from_spec(spec);spec.loader.exec_module(audit)
ORIGINAL_UNPACK=struct.unpack_from

def inventory(data,visible=None,base=0x1000000,calls=True):
 if visible is None:visible=lambda offset,n:offset>=0 and offset+n<=len(data)
 def guarded(fmt,buffer,offset=0):
  if not visible(offset,struct.calcsize(fmt)):raise AssertionError('Decoded protected/unmapped byte')
  return ORIGINAL_UNPACK(fmt,buffer,offset)
 with patch.object(audit.struct,'unpack_from',guarded):return audit.scan(data,base,visible,calls)

def pair(target,register=9):return struct.pack('<4H',0xe100+register,target&65535,0xe140+register,target>>16)

class InventoryTests(unittest.TestCase):
 def test_exact_adjacent_pair_and_call(self):
  target=audit.TARGETS['core_signal_callback'];r=inventory(pair(target)+struct.pack('<H',0x61))['core_signal_callback']['low_half_candidates'];self.assertEqual(len(r),1);self.assertTrue(r[0]['matches_target']);self.assertEqual(r[0]['adjacent_indirect_call'],'0x1000008')
 def test_different_high_half_is_not_target(self):
  r=inventory(pair(0x01cc1304))['drum_event_pitch']['low_half_candidates'][0];self.assertFalse(r['matches_target']);self.assertEqual(r['resolved_address'],'0x1cc1304')
 def test_different_register_does_not_resolve(self):
  data=struct.pack('<4H',0xe109,0x1304,0xe14a,0xff80);r=inventory(data)['drum_event_pitch']['low_half_candidates'][0];self.assertFalse(r['adjacent_same_register_high']);self.assertIsNone(r['resolved_address'])
 def test_high_first_pair(self):
  data=struct.pack('<4H',0xe149,0xff80,0xe109,0x1304);self.assertEqual(inventory(data)['drum_event_pitch']['high_first_pairs'],['0x1000000'])
 def test_known_hole_never_decoded(self):
  data=pair(audit.TARGETS['drum_event_pitch']);visible=lambda i,n:i>=0 and i+n<=4;r=inventory(data,visible)['drum_event_pitch']['low_half_candidates'][0];self.assertIsNone(r['matches_target']);self.assertFalse(r['adjacent_high_visible'])
 def test_protected_pairs_never_decoded(self):
  data=bytearray(48);data[:8]=pair(audit.TARGETS['drum_event_pitch']);data[12:20]=pair(audit.TARGETS['core_signal_callback']);data[20:22]=struct.pack('<H',0x61)
  visible=lambda i,n:i>=0 and i+n<=len(data) and all(k%24>=8 for k in range(i,i+n));r=inventory(data,visible,0xffa00000,False);self.assertEqual(r['drum_event_pitch']['low_half_candidates'],[]);self.assertEqual(r['core_signal_callback']['low_half_candidates'][0]['adjacent_indirect_call'],'0xffa00014')
 def test_protected_high_is_unknown_not_reference(self):
  data=bytearray(48);data[20:28]=pair(audit.TARGETS['drum_event_pitch']);visible=lambda i,n:i>=0 and i+n<=len(data) and all(k%24>=8 for k in range(i,i+n));r=inventory(data,visible)['drum_event_pitch']['low_half_candidates'][0];self.assertIsNone(r['matches_target']);self.assertIsNone(r['resolved_address'])
 def test_relative_calls_signed_forward_and_backward(self):
  target=audit.TARGETS['core_signal_callback']
  for base in [target-8,target+8]:
   delta=((target-base)//2)&0xffffff;data=struct.pack('<HH',0xe300|(delta>>16),delta&65535);self.assertEqual(inventory(data,base=base)['core_signal_callback']['relative_calls'],[hex(base)]);self.assertEqual(inventory(data,base=base,calls=False)['core_signal_callback']['relative_calls'],[])
 def test_literal_alignment_is_explicit(self):
  needle=struct.pack('<I',audit.TARGETS['drum_render']);self.assertEqual(inventory(needle)['drum_render']['literal_words'],['0x1000000']);self.assertEqual(inventory(b'\0\0'+needle)['drum_render']['literal_words'],[])

 def test_loaded_buffer_followed_by_call_is_not_callee(self):
  target=audit.TARGETS['drum_pitch_buffer'];base=0x1000000;destination=audit.TARGETS['vector_constructor'];delta=((destination-base-8)//2)&0xffffff
  data=pair(target,0)+struct.pack('<HH',0xe300|(delta>>16),delta&65535)
  item=inventory(data)['drum_pitch_buffer']['low_half_candidates'][0]
  self.assertIsNone(item['adjacent_indirect_call']);self.assertEqual(item['following_relative_call'],{'address':hex(base+8),'destination':hex(destination)})
 def test_following_relative_call_signed(self):
  data=pair(audit.TARGETS['drum_gain_buffer'])+struct.pack('<HH',0xe3ff,0xfffc)
  self.assertEqual(inventory(data)['drum_gain_buffer']['low_half_candidates'][0]['following_relative_call']['destination'],'0x1000000')
 def test_following_call_cannot_cross_protected_span(self):
  data=pair(audit.TARGETS['drum_gain_buffer'])+struct.pack('<HH',0xe300,4)
  visible=lambda i,n:i>=0 and i+n<=len(data) and not(i<12 and i+n>10)
  self.assertIsNone(inventory(data,visible)['drum_gain_buffer']['low_half_candidates'][0]['following_relative_call'])
 def test_different_high_half_has_no_following_call(self):
  data=pair(0x01cc1950)+struct.pack('<HH',0xe300,4)
  self.assertIsNone(inventory(data)['drum_pitch_buffer']['low_half_candidates'][0]['following_relative_call'])
 def test_noncall_after_pair(self):
  data=pair(audit.TARGETS['drum_gain_buffer'])+bytes(4)
  self.assertIsNone(inventory(data)['drum_gain_buffer']['low_half_candidates'][0]['following_relative_call'])
 def test_high_first_following_relative_call(self):
  data=struct.pack('<6H',0xe140,0xff80,0xe100,0x1950,0xe300,4)
  entry=inventory(data)['drum_pitch_buffer']['high_first_following_calls'][0]
  self.assertEqual(entry,{'pair_address':'0x1000000','call':{'address':'0x1000008','destination':'0x1000010'}})
 def test_new_targets_are_distinct(self):
  self.assertEqual(len(audit.TARGETS),22);self.assertEqual(len(set(audit.TARGETS.values())),22)

if __name__=='__main__':unittest.main()
