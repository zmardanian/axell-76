# Axell 76 → 2.0.0 plan (overnight, 2026-11-11/12)

Every claim measurable; every step committed. Baselines are 1.2.0.

| # | Work | Measure | 1.2.0 | 2.0 target |
|---|---|---|---|---|
| 1 | Robustness suite: automation zipper, NaN/Inf/denormal input, SR switch, mono, state round trip (skin), block sizes | DSP + plugin tests | 30 checks | all new checks green |
| 2 | Aliasing: measure 1x/2x/4x vs UADx; QUALITY switch if it pays | alias level at 8-16 kHz drive | unmeasured | ≤ UADx |
| 3 | Fidelity: ALL transients, static extremes, harmonics - model upgrades + joint refit | hardware / held-out music / static / transients | 0.47 / 0.65 / 1.92 / −3.3 dB | ≤0.45 / ≤0.60 / ≤1.3 / within ±1.5 dB |
| 4 | Features: side-chain high-pass, external side-chain, meter selector (GR / +4 / +8 / OFF), factory presets | tests + manual | - | in, off by default (fidelity unchanged) |
| 5 | Formats: LV2 (JUCE native) beside VST3 + standalone | validator / lv2 load | VST3 + SA | VST3 + LV2 + SA |
| 6 | CPU: measure vs UADx, optimise the per-sample path | % of one core, 48 kHz, 128 blocks | unmeasured | ≤ UADx |
| 7 | GUI: meter selector, preset menu, quality, SC HPF on the panel; compact; both skins | snapshots | - | - |
| 8 | Docs, report, portable build, release 2.0.0 | - | - | - |

Status is tracked in git log.

## Status (23:40)
- [x] 1 robustness: NaN/Inf sanitising, +10 DSP checks, plugin suite (state, reload identity, bypass null, junk state, quality)
- [x] 2 aliasing measured: A76 2x is 10-20 dB cleaner than UADx, 4x 25-35 dB; QUALITY 2x/4x(default)/8x, RT-safe switch
- [~] 3 fidelity: stage A committed (detector blend, soft FET cap): static 1.01, MUSDB 0.63, hardware 0.464. Stage B running:
      ALL-mode control lag + per-hit peak residuals (fit_v5b.log -> params_v5_b.json). Accept only if hardware and held-out
      music do not get worse.
- [x] 4 side-chain HPF, external side-chain bus, meter modes, 12 factory presets (OUTPUT to be calibrated: research/presets/calibrate.py after the final fit)
- [x] 5 LV2 (discovered by lilv; lv2apply cannot drive JUCE's atom port)
- [~] 6 CPU: below-threshold early-out, meter dB per block; re-benchmark on a quiet machine
- [x] 7 GUI: strip, preset display, meter modes
- [ ] 8 calibrate presets, README/REPORT, screenshots + audio refresh, portable build, release 2.0.0
