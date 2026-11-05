#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Unpack numeric observations outside the checkout and run native verifier."""
import gzip,subprocess,sys,tempfile
from pathlib import Path
with tempfile.TemporaryDirectory(prefix='pocket-element-pitch246-') as directory:
 for suffix in ['input','output']:
  name=f'element-pitch246-{suffix}.bin'
  (Path(directory)/name).write_bytes(gzip.decompress((Path(sys.argv[2])/(name+'.gz')).read_bytes()))
 sys.exit(subprocess.run([sys.argv[1],directory],check=False).returncode)
