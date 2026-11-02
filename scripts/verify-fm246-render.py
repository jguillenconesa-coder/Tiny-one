#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Private firmware oracle for four-operator FM; no firmware is recorded."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
CASES = 4096


def inputs():
    result = bytearray()
    seed = 0x464d3456
    def rnd():
        nonlocal seed
        seed = (1664525 * seed + 1013904223) & 0xffffffff
        return seed
    for n in range(CASES):
        state = [rnd() for _ in range(15)]
        blue, green, orange = rnd() & 32767, rnd() & 32767, rnd() & 32767
        selector = n % 9
        white = selector * 2048 + (rnd() & 2047)
        if n < 256:
            blue = (0,1,8191,8192,8193,16383,16384,32767)[n % 8]
            orange = (0,1,8192,32767)[(n // 8) % 4]
        if n % 64 == 63:
            white = (-32768,-1,18432,32767)[(n // 64) % 4]
        action = 1 if n < 2048 or n % 128 == 0 else 0
        pitch = rnd()
        if n < 256: pitch = (0,1,65535,65536,0x7fffffff,0x80000000,0xffffffff,0x0146f98e)[n % 8]
        scratch = [rnd() for _ in range(128)]
        audio = [rnd() for _ in range(64)]
        result += struct.pack('<15I4hII128I64I', *state,blue,green,white,orange,pitch,action,*scratch,*audio)
    return result


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for key in ('ldr','assembler','linker','simulator','testutils','output'):
        parser.add_argument('--'+key,type=Path,required=True)
    parser.add_argument('--record',action='store_true')
    args=parser.parse_args(); out=args.output.resolve()
    if out.exists() or out.is_relative_to(ROOT): raise ValueError('Use a NEW private directory outside checkout')
    out.mkdir(parents=True)
    common=[]
    for key in ('ldr','assembler','linker','simulator','testutils'):common += ['--'+key,str(getattr(args,key))]
    # Existing provenance locks and simulator writeback microtests run first.
    subprocess.run([sys.executable,str(ROOT/'scripts/verify-fm246-foundation.py'),*common,'--output',str(out/'foundation')],check=True)
    (out/'table.bin').write_bytes((out/'foundation/output.bin').read_bytes()[:60])
    data=inputs();assert len(data)==CASES*844
    (out/'inputs.bin').write_bytes(data)
    source=(ROOT/'research/blackfin/fm-render.s.in').read_text().replace('@WORK@',out.as_posix()).replace('@BYTES@',str(CASES*896))
    (out/'probe.s').write_text(source)
    def run(cmd,**kw):subprocess.run(list(map(str,cmd)),check=True,timeout=180,**kw)
    run([args.assembler,'-I',args.testutils,'-o',out/'probe.o',out/'probe.s'])
    run([args.linker,'-T',ROOT/'research/blackfin/voice-contract.ld','-o',out/'probe.elf',out/'probe.o'])
    with (out/'output.bin').open('wb') as output,(out/'probe.log').open('wb') as log:
        run([args.simulator,'--model','bf524','--memory-size','64M','--memory-region','0xff800000,0x8000','--memory-region','0xff900000,0x8000','--watch-cycles-int','900000000',out/'probe.elf'],stdout=output,stderr=log)
    if (out/'output.bin').stat().st_size!=CASES*896:raise RuntimeError('Incomplete original FM render')
    artifacts={}
    for name,source in [('fm246-render-input.bin',out/'inputs.bin'),('fm246-render-output.bin',out/'output.bin')]:
        target=ROOT/'research/fm-render/vectors'/name
        if args.record:target.write_bytes(source.read_bytes())
        if target.read_bytes()!=source.read_bytes():raise RuntimeError('Oracle fixture mismatch: '+name)
        artifacts[name]={'bytes':source.stat().st_size,'sha256':hashlib.sha256(source.read_bytes()).hexdigest()}
    report={'cases':CASES,'samples':CASES*128,'callback':'0x019a23d4','source_block_samples':128,
            'state_bytes':60,'scratch_bytes':512,'table_words':30,'artifacts':artifacts,
            'model':'GNU BF524 with both verified writeback patches; ten authored coefficients',
            'limits':['Not hardware/acoustic validation.','Pitch, scheduler and output routing supplied explicitly.'],
            'foundation_ldr_sha256':json.loads((out/'foundation/evidence.json').read_text())['ldr_sha256']}
    (out/'evidence.json').write_text(json.dumps(report,indent=2)+'\n')
    print('PASS: original four-operator FM render: 4096 blocks / 524288 samples')


if __name__=='__main__':main()
