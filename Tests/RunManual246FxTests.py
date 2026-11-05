#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Unpack numerical observations outside the checkout and run the verifier."""
import gzip,subprocess,sys,tempfile
from pathlib import Path
with tempfile.TemporaryDirectory(prefix='pocket-manual-fx246-') as directory:
 for source in ['bend','crank']:
  for suffix in ['input','output']:
   name=f'{source}-fx246-{suffix}.bin'
   (Path(directory)/name).write_bytes(gzip.decompress((Path(sys.argv[2])/(name+'.gz')).read_bytes()))
 sys.exit(subprocess.run([sys.argv[1],directory],check=False).returncode)
