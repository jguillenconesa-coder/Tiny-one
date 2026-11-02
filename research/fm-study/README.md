# Authored FM study (instrument 0.26.0)

This is a diagnostic of Pocket One's **authored two-oscillator FM**, not a
reconstruction or acoustic reference for the original four-operator engine.
Own code: AGPL-3.0-only. No firmware, disassembly, presets or manual pages are
included. See [findings and limits](../../docs/fm-authored-study.md).

Run from the repository root with C++17, Python, NumPy and SciPy:

```sh
mkdir -p /tmp/pocket-fm-study
c++ -std=c++17 -O2 -I Source research/fm-study/render.cpp -o /tmp/pocket-fm-study/render
python research/fm-study/study.py --renderer /tmp/pocket-fm-study/render --output /tmp/pocket-fm-study/results
```

The script measures 45 stationary cases at 44.1/48/96 kHz. A signed Bessel
expansion retaining only frequencies below Nyquist is the bandlimited reference
for the **authored oscillator formula**. Negative-frequency sidebands are
included, so overlapping harmonics cancel correctly. The residual is the RMS
of sampled raw oscillator minus that reference, relative to raw oscillator RMS.
It is neither an OP-1 error measurement nor an alias-energy percentage; correlated
components and cancellations prevent that interpretation. The stationary model
excludes envelope motion, the implicit filter, saturation, delay and host routing.

The optional C++ renderer separately calls the actual `pocket::Engine`, warms
controls in silence for one second, holds one voice for 125 ms, and writes 250 ms
of samples. It fixes velocity 1, sustain 1, attack 0, drive/effect/LFO off and
internal volume .7. All hybrid flags remain off. Its twelve captures establish
finite output and report RMS/peak, **not agreement with the Bessel reference**.
They include the actual common filter and nonlinearities, but not Processor's
master bus. No audio files need to be committed. Results can vary slightly with
compiler/libm; the report is not a bit-exact golden test.
