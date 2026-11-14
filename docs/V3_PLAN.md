# Axell 76 → 3.0 plan

3.0 ships only when the headline goals are met on held-out data and nothing that 2.0 does well gets worse.
Baselines are 2.0.0, measured with the same scripts. Every number is reproducible from `research/`.

## The milestone

**Distortion that matches a real 1176LN, not only its gain.** 2.0 matches the hardware's gain movement better than the
UADx does (0.46 vs 0.91 dB), but it is measurably too clean: less added distortion than the hardware (3.1 dB over all 593 notes), and 7-15 dB
short on the UADx's 3rd - 5th harmonics at slow release. 3.0 adds a new hardware test that scores the whole waveform,
distortion included, and closes the gap.

## Target: balanced between the hardware 1176LN and the UADx 1176

Neither reference is the truth on its own: the UADx is measured everywhere (every knob, clean harmonics) but
measurably too dirty against the hardware; the hardware is the original but recorded at one setting only. The target is
defined so the balance is a number, not a taste:

1. **Shape from the UADx.** How the harmonics move with frequency, level, attack, release and ratio (H2 - H5 map).
   Fit A does this.
2. **Amount and gain motion from the hardware**, where hardware data exists. One overall distortion level (and nothing
   else) is set on the hardware *training* notes (the even-note calibration quarter); the 593 odd notes stay held out.
3. **The balance is explicit.** Score every candidate on both distances - harmonics vs UADx (dB RMS) and added
   distortion vs hardware (NLR bias / median) - and sweep the distortion level from "UADx amount" to "hardware amount".
   That gives the trade-off curve (Pareto front). 3.0 ships the point that is closest to both at once (the knee of the
   curve), and the report shows the whole curve, both end points and where 3.0 sits on it.

## Goals

| # | Goal | Measure (script) | 2.0 | 3.0 must reach |
|---|---|---|---:|---:|
| 1 | Hardware waveform | ESR vs hardware 1176LN, 593 held-out notes, median (`v3/hwwave.py`) | -24.65 dB | better, CI excludes 2.0 |
| 2 | Hardware distortion level | added-distortion (NLR) model - hardware, median abs / bias (`v3/hwwave.py`) | 2.26 / -3.11 dB | ≤ 1.0 / within ±1.0 |
| 3 | Harmonics vs UADx, held out | H2-H5 at 103 held-out points (map 100 / 400 / 2000 Hz + the 2.0 100 Hz set; fitting uses 50 / 200 / 1000 / 4000 Hz), RMS (`v3/dist_eval.py`) | 7.18 dB (H3 8.38) | ≤ 3.5 dB |
| 4 | Static curves | 300 points RMS / max (`v3/base_static.py`) | 1.01 / 3.4 dB | ≤ 0.8 / ≤ 2.5 |
| 5 | Fit = ship | the fitting renderer and the plugin give the same output | DC path differs | identical (test) |

## Guards (must not get worse beyond noise)

| Measure | 2.0 |
|---|---:|
| Hardware 1176LN gain trajectory, 593 notes, mean / median / p90 | 0.464 / 0.409 / 0.797 |
| Held-out MUSDB18 (100 clips) median / p90 | 0.63 / 1.09 |
| The artist's takes (21) / Desktop (6) median | 0.77 / 0.48 |
| Dynamic envelopes RMS | 1.01 |
| Aliasing at 4x, CPU at 4x | -68..-90 dB / 2.0 % |

"Beyond noise" is decided with a paired bootstrap over notes / clips (95 % interval of the 3.0 - 2.0 difference).

## Work

1. Measure: hardware waveform baseline (2.0 and UADx, all 593 notes); a UADx distortion map (harmonics vs frequency,
   level, attack, release; `measure/m_dist3.py`) to tell ripple from a static or transformer nonlinearity.
2. Fit = ship: the fit renderer DC-blocks the distortion exactly like the engine.
3. Model: the mechanism the distortion map points to; refit jointly (tones, music, drum hits, distortion).
4. Validate on everything held out; paired bootstrap; accept or reject (rejected ideas are reported, as in 2.0).
5. Release 3.0.0 only if goals 1-5 pass and the guards hold.

## Status
- 2.0 baseline, 593 held-out hardware notes at the shipping 4x: gain trajectory 0.464 / 0.410 / 0.797 dB (unchanged from 2x);
  waveform ESR median **-24.65 dB** (UADx -15.85); added distortion vs hardware: Axell 76 **-3.11 dB** (too clean),
  UADx **+2.29 dB** (too dirty), median abs 2.26 / 2.58 dB. The hardware sits between the two: the UADx distortion map
  shapes the harmonics, the hardware decides how much.
- Fit = ship: `research/fit/libeng.so` is the engine itself (`v3/fit_equals_ship.py`: worst -136 dB, PASS). The 2.0 fit
  renderer differed from the plugin by about -30 dB (undamped distortion DC); the exporter rounded fetDist by 30 %;
  all 2.0 validation renders ran at 2x (the plugin defaults to 4x) - all fixed.
- **Paused 2026-11-12 ~09:00 (user break).** Fit A was stopped after ~45 min (no checkpoint, nothing saved from it).
  Resume: `cd research/fit && BOUNDS="s00,0.3,1.25;s01,0.3,1.25;s02,0.3,1.25;s03,0.3,1.25;s04,0.3,1.25;ampDist,0,0.05;fetDist,0,0.01;fetDistIn,0,0.02" STATICW=2.5 DISTW=0.15 nohup ~/axell-tools/venv/bin/python -u fit_v3.py params_v5_final.json params_v3_a.json 30 > fit_v3a.log 2>&1 &`
  (~1 h; add per-iteration progress output first). Then evaluate params_v3_a.json: `v3/music_eval.py`, `v3/dist_eval.py`,
  export to a renderer, `v3/hwgain.py` + `v3/hwwave.py`, `v3/paired.py` against the 2.0 json files in research/v3.
- **Fit A (2026-11-12, rejected).** Joint refit with the four 3.0 options free (UADx distortion map in the fit). Used
  kneeQuad 0.42, ampOdd -0.010, fetOdd -0.0034, relBias 0. Held-out, paired vs 2.0 (`research/v3/eval_candidate.sh`):
  distortion vs UADx 6.35 -> 5.75 dB (better, CI [-0.86, -0.34]); hardware gain 0.464 -> 0.473 (worse, CI excludes 0);
  hardware waveform ESR worse by 0.16 dB (CI excludes 0); added distortion vs hardware unchanged (-3.11 dB); music same.
  Lesson: the UADx's tone harmonics are far too small to explain the hardware's missing ~3 dB of added distortion,
  which dominates the hardware waveform error. Next: probe which mechanism moves the hardware NLR at all
  (`research/v3/probe/`), then build the balance sweep on that mechanism.
- **Where the hardware's extra "dirt" is (2026-11-12, `v3/nlr_split.py`, `v3/nlr_bands.py`, 119 held-out notes, 2.0 model).**
  Not harmonic distortion: harmonics 2-10 are within ~1.3 dB of the hardware, and no static distortion term moves the
  hardware NLR (probe: ampOdd/fetOdd/fetDist any size: 0.0 dB; ampDist x10: +0.6 dB and worse ESR). Not recording noise:
  the hardware's own unexplained signal in the quiet lead-in is -45 dB re the note, ~15 dB under the gap.
  It is **low-frequency content that follows the compression**: below f0/2 the model is 7.0 dB short (hardware -31.2,
  model -37.9 dB re the note), 10.2 dB short in the first 60 ms after the onset; between harmonics 3.5 dB short.
  Reading: the classic 1176 ALL-mode thump - control-voltage feedthrough and/or the envelope-varying DC of the
  even-order distortion passing the coupling capacitors (the model removes that DC). Next: model it as a third
  mechanism (shape from the UADx's step responses, amount from the hardware training notes) and balance it.
- **The thump and the onset hold (2026-11-12).** New UADx measurement with LF-free 5 kHz ramped steps
  (`measure/m_thump.py`, `v3/thump.py`): the UADx thumps only in ALL at attack 1 (LF -33 dB RMS / -8.5 dB peak re the
  tone vs -55..-80 elsewhere), and there it **holds its gain ~10 ms** at the onset (output -3.7 dBFS), 2.0 compresses
  within ~2 ms. The LF follows the squared output level through a ~8 Hz coupling high-pass (class-A output stage);
  2.0 never overshoots, so a level-threshold thump (FetModel thump / thumpHz / thumpMs / thumpThr, off by default,
  2.0 output bit-identical) never fires on its own. Hardware 1176LN, 1 ms frames (`v3/onset_hold.py`, 119 test notes):
  the hardware **holds ~3-4 ms** (+9.4 dB re the settled gain at 1-4 ms; 2.0 +7.3 .. +5.9). So the onset is the
  balance point: 2.0 (no hold) < hardware (~3-4 ms) < UADx (~10 ms). The 2.0 "ALL lag" rejection was a 0.08 ms lag
  fitted to the UADx's drum peaks; the hardware's own onset now says a ms-scale lag is real. Next: lag probe on the
  training notes (`v3/probe/lag.sh`), then thump on top, then the paired held-out evaluation.
- **ALL onset lag (2026-11-12, rejected; training notes, every 5th = 119).** `v3/probe/lag.sh`, allLagMs 0-4 ms:
  added distortion vs hardware gets closer (NLR bias -3.16 -> -2.09 / -1.08 / -0.14 dB at 2 / 3 / 4 ms) but the
  hardware waveform gets clearly worse (ESR median -24.63 -> -23.05 / -22.42 / -21.81 / -21.21 dB at 1 - 4 ms) and the
  1 ms onset profile improves only at 1 - 2 ms (RMS 0.85 -> 0.71 dB) while later frames get worse. A delay of the whole
  control path is the wrong shape for the hardware's hold; goal 1 (waveform) rules it out. The thump depends on the
  hold, so it is not pursued either (code present, off, 2.0 output bit-identical). **2.0 remains the current version.**

**Closed 2026-11-12: no 3.0.** Summary in `docs/REPORT.md` section 8. The plugin's sound is 2.0 (the current build
matches the released 2.0 VST3 to -89 dB or better at five settings; the difference is the full-precision export).

**Released 2026-11-12 as 3.0.0** after the priorities were reset (fuzz toward the UADx at 75 %, a delicate capped thump, 2.0 sessions bit-identical). Results: `docs/REPORT.md` section 9.
