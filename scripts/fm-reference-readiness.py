#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Triage public numeric FM preset observations, without reading/publishing media."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    inventory=ROOT/'research/blackfin/factory-preset-inventory.json'
    observations=ROOT/'research/blackfin/preset-video-index.json'
    model=json.loads((ROOT/'research/blackfin/fm246-table-model.json').read_text())
    protected=set(model['authored_indices']);seen=set();rows=[]
    video={p['inventory_id']:p for p in json.loads(observations.read_text())['presets'] if p['category']=='fm'}
    for p in json.loads(inventory.read_text())['presets']:
        if p.get('type')!='fm':continue
        controls=p['knobs'][:4]
        words=[6+i*8+((controls[1]>>shift)&7) for i,shift in enumerate((12,9,6))]
        seen.update(words)
        rows.append(dict(name=p['name'],inventory_id=p['id'],selector=controls[2]>>11,
                         ratio_table_words=words,authored_ratio_words=sorted(protected.intersection(words)),
                         blue_zero=controls[0]==0,motion_zero=controls[3]==0,
                         fx=p.get('fx_type') if p.get('fx_active') else None,
                         lfo=p.get('lfo_type') if p.get('lfo_active') else None,
                         isolated=not p.get('fx_active') and not p.get('lfo_active'),
                         screen_observation=video[p['id']]['screen_observation']))
    result=dict(schema=1,scope='FM reference triage only; screen selections are not MIDI events or proof of identical patch settings.',
                source_sha256={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in (inventory,observations)},
                presets=rows,isolated_count=sum(p['isolated'] for p in rows),
                authored_ratio_words_never_selected_by_stored_presets=sorted(protected.difference({0,1}).difference(seen)),
                limits=['No audio was listened to, measured or calibrated by this script.',
                        'A blue-zero static preset does not expose modulator ratios; active LFO may change this.',
                        'Internal rate constants require nonzero Motion and audible modulation.',
                        'Video matching by name does not prove firmware version, numeric controls, notes or recording chain.'])
    args.output.write_text(json.dumps(result,indent=2)+'\n')
    print(f"{len(rows)} FM presets; {result['isolated_count']} without active FX/LFO; missing ratio words {result['authored_ratio_words_never_selected_by_stored_presets']}")


if __name__=='__main__':main()
