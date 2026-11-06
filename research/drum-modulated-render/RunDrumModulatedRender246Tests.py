#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
import gzip, pathlib, subprocess, sys, tempfile
with tempfile.TemporaryDirectory(prefix='drum-modulated-render246-') as directory:
    for packed in pathlib.Path(sys.argv[2]).glob('*.bin.gz'):
        (pathlib.Path(directory) / packed.stem).write_bytes(gzip.decompress(packed.read_bytes()))
    (pathlib.Path(directory) / 'verified-sine.bin').write_bytes((pathlib.Path(__file__).resolve().parents[2] / 'Tests/ReferenceVectors/verified-sine.bin').read_bytes())
    subprocess.run([sys.argv[1], directory], check=True)
