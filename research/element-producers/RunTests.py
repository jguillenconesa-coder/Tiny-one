#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
import gzip,subprocess,sys,tempfile
from pathlib import Path
with tempfile.TemporaryDirectory() as directory:
    for name in ['input','output']:
        filename='envelope-publisher246-'+name+'.bin'
        (Path(directory)/filename).write_bytes(gzip.decompress((Path(sys.argv[2])/(filename+'.gz')).read_bytes()))
    subprocess.run([sys.argv[1],directory],check=True)
