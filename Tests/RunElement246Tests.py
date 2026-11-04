#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Unpack synthetic numerical observations outside the checkout and run native test."""
import gzip,subprocess,sys,tempfile
from pathlib import Path
with tempfile.TemporaryDirectory(prefix='pocket-element246-') as directory:
 for suffix in ['input','output']:
  name=f'element246-{suffix}.bin'
  (Path(directory)/name).write_bytes(gzip.decompress((Path(sys.argv[2])/(name+'.gz')).read_bytes()))
 sys.exit(subprocess.run([sys.argv[1],directory]).returncode)
