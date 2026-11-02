#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-only
"""Stationary authored-FM alias diagnostic; no OP-1 acoustic reference.

Requires numpy/scipy. Optionally invokes the actual Engine renderer. Generated
CSVs/audio samples belong in a scratch directory, outside the repository.
"""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
import subprocess

import numpy as np
from scipy.special import jv


def stationary(rate, note, tone, ratio):
    frequency = 440 * 2 ** ((note - 69) / 12)
    # Integer-Hz frequencies in this study permit exactly periodic one-second
    # measurements. No FFT window or oversampling-filter convention is needed.
    assert abs(frequency - round(frequency)) < 1e-9
    t = np.arange(rate, dtype=np.float64) / rate
    beta = 7 * tone  # level = 1; LFO off
    direct = np.sin(2 * np.pi * frequency * t +
                    beta * np.sin(2 * np.pi * frequency * ratio * t))
    reference = np.zeros_like(direct)
    reconstructed = np.zeros_like(direct)
    # Jacobi-Anger expansion. Signed frequencies must be retained: positive
    # and negative sidebands can overlap and cancel. Truncation is far below
    # float64 precision for beta <= 7; this is not an original-firmware model.
    for k in range(-80, 81):
        f = frequency * (1 + k * ratio)
        component = jv(k, beta) * np.sin(2 * np.pi * f * t)
        reconstructed += component
        if abs(f) < rate / 2:
            reference += component
    reconstruction_error = float(np.max(np.abs(direct - reconstructed)))
    assert reconstruction_error < 5e-10
    residual = direct - reference
    rms = lambda x: float(np.sqrt(np.mean(x * x)))
    error = rms(residual)
    return dict(rate=rate, note=note, hz=frequency, tone=tone, ratio=ratio,
                beta=beta, raw_rms=rms(direct), bandlimited_rms=rms(reference),
                full_expansion_max_error=reconstruction_error,
                folded_residual_rms=error,
                folded_residual_db_relative_raw=(20 * math.log10(error / rms(direct))
                                                 if error else -999))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--renderer", type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    cases = [(0., 1), (.45, 3), (1., 8)]
    rows = [stationary(rate, note, tone, ratio)
            for rate in (44100, 48000, 96000)
            for note in (57, 69, 81, 93, 105)
            for tone, ratio in cases]
    # Negative control: unmodulated sine has no folded PM sidebands.
    assert all(r["folded_residual_rms"] < 1e-10
               for r in rows if r["tone"] == 0)
    assert all(math.isfinite(v) for r in rows for v in r.values())
    engine = []
    if args.renderer:
        for rate in (44100, 48000, 96000):
            for note, tone, color in ((69, 0, 0), (69, .45, .35),
                                      (93, 1, 1), (105, 1, 1)):
                path = args.output / f"engine-{rate}-{note}-{tone}.csv"
                subprocess.run([str(args.renderer.resolve()), str(rate), str(note),
                                str(tone), str(color), str(path)], check=True)
                x = np.loadtxt(path, delimiter=",", skiprows=1)[:, 1]
                assert len(x) == rate // 4 and np.all(np.isfinite(x))
                engine.append(dict(rate=rate, note=note, tone=tone, color=color,
                                   samples=len(x), rms=float(np.sqrt(np.mean(x*x))),
                                   peak=float(np.max(np.abs(x)))))
    result = dict(scope="Authored FM diagnostic, NOT OP-1 acoustic validation",
                  source_engine_sha256=hashlib.sha256(
                      (Path(__file__).resolve().parents[2] / "Source/Engine.h").read_bytes()
                  ).hexdigest(),
                  raw_model="sin(wc*t + 7*tone*sin(ratio*wc*t)), level=1",
                  reference="Signed Bessel sidebands |f| < Nyquist; k=-80..80",
                  exclusions=["envelope motion", "filter", "nonlinearities",
                              "delay", "voice summation", "plugin output bus"],
                  stationary=rows, engine=engine)
    (args.output / "results.json").write_text(json.dumps(result, indent=2) + "\n")
    with (args.output / "stationary.csv").open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader(); writer.writerows(rows)
    print(f"{len(rows)} stationary cases; {len(engine)} actual Engine renders")


if __name__ == "__main__":
    main()
