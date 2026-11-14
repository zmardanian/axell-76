# Axell 76 - measurement and validation report

Version 3.2.0 (sections 1 - 6 describe the 1.1 method; section 7 is what 2.0 added, section 8 the rejected 3.0 attempts, section 9 what 3.0 shipped, section 10 the 3.1 presets, section 11 the 3.2 hardware anchoring). Everything here can be rerun from `research/`; the numbers below are copied from the scripts' outputs.

## 1. What was inspected (and what was not)

The UADx 1176 VST3 (Windows, run under Wine through yabridge) was inspected with REA for static facts only:

| Finding | Source |
|---|---|
| A JUCE plugin, PACE/iLok licensing linked into the same binary | PE imports, RTTI strings (`research/rea/`) |
| 5 controls: Input, Output (labelled −∞..0 dB, non-linear tapers), Attack, Release (1-7), Ratio (4/8/12/20/ALL) | manifest, page table, the host's parameter list |
| Latency 86 samples at 44.1 / 48 kHz, 118 at 96 kHz | host report, step onsets |

The DSP machine code and the licensing code were not decompiled. Nothing from the vendor's code or data is in Axell 76.

## 2. Black-box measurements of the UADx (`research/measure/`)

| Set | What | Renders |
|---|---|---:|
| Gain staging | input/output taper, linear gain | 62 |
| Static curves | 1 kHz, −66..0 dBFS in 3 dB steps, 5 ratios × 3 input settings | 315 |
| Dynamics | 25 dB and 40 dB steps; attack 1-7, release 1-7; 4:1, 20:1, ALL | 84 |
| Release memory | bursts of 20 ms / 200 ms / 2 s, release 1 / 4 / 7 | 9 |
| Detector | 30 Hz - 16 kHz at a fixed level, compressing and linear | 22 |
| Harmonics | 100 Hz and 1 kHz, 6 levels, release 1 and 7 | 24 |
| SMPTE IMD | 60 Hz + 7 kHz (4:1), 3 levels, release 1 and 7 | 6 |
| Transients | kick- and snare-like hits, attack 1/4/7, 4:1 and ALL | 12 |
| Sample rate | steps and a level staircase at 44.1 and 96 kHz | 18 |
| Low-frequency response | 2 - 60 Hz, magnitude and phase | 10 |
| Stereo link, polarity | one channel driven; vocal at both polarities | 3 |

Findings that shaped the model: ratios steeper than their names above the knee, with different thresholds per
button; faster attack on bigger overshoots; program-dependent release (a 2 s note builds 2.5 dB more GR than a 20 ms
hit and recovers more slowly); ALL: +1.2 - 1.5 dB gain, ~11 - 13x faster release; GR saturates in the mid-30s dB;
flat detector; fully linked stereo; near-symmetric detector (0.24 dB polarity effect on a vocal); a 3rd-order coupling
low-cut (−1 dB at 10 Hz, −8 dB at 5 Hz); behaviour independent of the sample rate.

## 3. Music set (`research/music/`)

379 clips, random settings (ratio, attack, release, input), UADx outputs recorded raw.
Training: 220 MUSDB18 7-second stems/mixes from the train split + the first 60 % of the artist's session (32 takes).
Held out: 100 MUSDB18 test-split clips, the last 40 % of the session (21 takes), 6 clips of the artist's vocal,
vocal + beat and a finished song. Error metric: per-channel 2.5 ms frame levels after a 40 Hz high-pass on both
signals (sub-20 Hz rumble otherwise turns phase into fake level error).

## 4. Model (`Source/FetModel.h`, `Source/FittedParams.h`)

Peak rectifier (slightly asymmetric) → soft-knee law per ratio button → linear control voltage with level-dependent
attack and program-dependent release (a slow memory) → FET gain 1/(1+v), capped → even-order distortion from the
output and from the pre-FET signal → coupling low-cut at the input. 41 parameters, least-squares fitted on test signals
and music jointly (`research/fit/fit_music.py`), distortion on harmonics + IMD (`fit_dist.py`).
The engine runs the model 2x oversampled, stereo-linked, with a delay-matched dry path for MIX.

## 5. Results

### 5.1 Against a hardware 1176LN (independent)

Martinez Ramirez et al. 2020 (Zenodo 3562442): 1186 dry/processed note pairs at ALL, 800 µs, 1100 ms. One global input
gain fitted on a quarter of the even notes, 593 odd notes held out; gain trajectory in 5 ms frames, per-note constant
removed (`research/validation/hw1176.py`, `hw1176_uad.py`).

| | mean | median | p90 |
|---|---:|---:|---:|
| Axell 76 1.1 | **0.471 dB** | **0.414 dB** | **0.797 dB** |
| Axell 76 1.0 (test signals only) | 0.654 | 0.599 | 0.932 |
| UADx 1176 | 0.905 | 0.880 | 1.122 |

### 5.2 Against the UADx, held-out music

| | median clip | p90 clip | bias |
|---|---:|---:|---:|
| MUSDB18 test clips (100) | 0.65 dB | 1.05 dB | +0.16 |
| The artist's takes (21) | 0.77 dB | 2.36 dB | +0.14 |
| Vocal / vocal + beat / song (6) | 0.49 dB | 1.16 dB | +0.01 |

1.0 on the same held-out sets (same metric): 1.46 / 1.68 / 1.17 dB median.

### 5.3 Against the UADx, the artist's vocal at matched settings (`ab_music.py`, 5 ms frames)

| Setting | vocal | vocal + beat |
|---|---:|---:|
| 4:1, attack 4, release 4 | 0.41 | 0.31 |
| 4:1, attack 7, release 7 | 1.29 | 0.96 |
| 8:1 | 0.57 | 0.40 |
| 20:1, fast attack | 0.47 | 0.55 |
| ALL, slow | 0.66 | 0.52 |
| ALL, fast | 0.68 | 0.55 |

### 5.4 Test signals, distortion, rates

| | Axell 76 vs UADx |
|---|---|
| Static curves (300 points), RMS | 1.92 dB (largest at extreme drive) |
| Dynamic envelopes (onsets weighted), RMS | 1.02 dB |
| Harmonics H2/H3, 20 points, RMS | 6.7 dB (levels span −30..−90 dB) |
| SMPTE IMD | within 2 dB at 5 of 6 points |
| 44.1 / 96 kHz | same behaviour as at 48 kHz |
| Drum hits, peak through, 4:1 | within ±1.6 dB |
| Drum hits, peak through, ALL slow attack | −3.3 dB (open) |

### 5.5 What did not work

- **Feedback topology** (detector after the FET, as in the 1176 circuit): fitted twice from two starting points on the
  same data - 1.59 dB median / 5.9 dB p90 on the training music vs 0.66 / 1.30 for the shipped law. Kept in the code
  (`feedback`, off).
- **Test signals alone**: 1.0 left 2 - 10 dB errors on dense music at heavy GR; music had to be part of the fit.

## 6. Plugin checks

DSP suite 30/30 (linearity, ratio slopes, finiteness at 44.1 - 192 kHz, exact nulls at MIX 0 and IN/OUT, block-size
independence, release ordering); pluginval strictness 10 including Steinberg's VST3 validator: pass.

## 7. Version 2.0 (overnight, 2026-11-11)

### 7.1 Model
- **Detector**: a blend of the instantaneous peak and an averaged (RC) level, scaled so a steady sine reads the same
  in both - static curves keep their calibration while spiky material reads lower. Fitted blend 0.078, tau 43 ms.
- **FET range**: a soft (3 dB) limit instead of a hard clamp.
- Refit on test signals (static weighted 2.5x), 252 music clips and the drum-hit recordings.

| | 1.2 | **2.0** |
|---|---:|---:|
| Hardware 1176LN, 593 held-out notes, mean / median / p90 | 0.471 / 0.414 / 0.797 | **0.464 / 0.409 / 0.797** |
| Static curves (300 points, RMS) | 1.92 dB (max 10.0) | **1.01 dB (max 3.4)** |
| Held-out MUSDB18 (100 clips) median / p90 | 0.65 / 1.05 | **0.63 / 1.09** |
| The artist's takes (21) median | 0.77 | 0.77 |
| Vocal / vocal + beat / song (6) median | 0.49 | 0.48 |

### 7.2 Tried and rejected (the hardware decides)
In ALL mode with a slow attack the UADx lets ~3 dB more of a drum hit's peak through. Two mechanisms were fitted:
an ALL-mode control lag (fitted 0.08 ms) and a separate ALL attack at the slow end of the knob (scanned 1.07 - 5x).
Both match the UADx's drum peaks better (peak error 1.70 -> 1.29 dB) and both move the model away from the hardware
1176LN at exactly that setting: 0.464 -> 0.473 dB (lag) and -> 0.536 dB (slow end 2.5x). Neither ships (code present, off).
A polynomial fast log/exp for the control path was also rejected: slower than the library and up to 0.008 dB off.

### 7.3 Aliasing (`research/validation/aliasing.py`)
Non-harmonic energy re the tone (20 Hz - 20 kHz), 0.5 FS sine, heavy fast settings:

| Setting, tone | UADx | A76 1x | A76 2x | **A76 4x (default)** | A76 8x |
|---|---:|---:|---:|---:|---:|
| ALL fast, 5 kHz | -51.9 | -56.2 | -63.5 | **-83.7** | -91.2 |
| ALL fast, 9 kHz | -44.6 | -51.3 | -61.4 | **-80.1** | -86.2 |
| ALL fast, 13 kHz | -41.1 | -45.6 | -64.0 | **-68.2** | -80.0 |
| ALL fast, 17 kHz | -40.3 | -41.6 | -54.1 | **-68.3** | -83.1 |
| 4:1 fast, 5 kHz | -55.9 | -60.1 | -72.5 | **-89.5** | -95.1 |
| 4:1 fast, 9 kHz | -53.9 | -57.8 | -70.2 | **-78.8** | -94.2 |
| 4:1 fast, 13 kHz | -52.6 | -57.2 | -66.5 | **-84.3** | -90.3 |
| 4:1 fast, 17 kHz | -54.8 | -44.1 | -68.6 | **-76.6** | -91.1 |

The oversampling factor changes the compression itself by < 0.02 dB (vocal, 1x vs 8x).

### 7.4 CPU (one core, 48 kHz, stereo, always compressing; offline renderer incl. file I/O)
1x 0.56 %, 2x 1.05 %, **4x 2.03 %**, 8x 3.63 %. The UADx 1176 measured 2.34 % through the Wine bridge (not like for like).

### 7.5 Features and robustness
QUALITY (2x / 4x / 8x, real-time-safe switching with latency update; pre-2.0 states load every setting and QUALITY 2x - the refined 2.0 model makes them sound closer to the hardware, not bit-identical to 1.x),
side-chain high-pass (OFF - 500 Hz) and external side-chain input, meter modes (GR / VU +4 / VU +8), 12 presets
calibrated by intended gain reduction on real material, LV2, NaN/Inf input sanitising. Tests: DSP 42, plugin 14
(state round trip incl. skin and quality, reload identity, bypass null, junk state), ASan/UBSan clean, pluginval
strictness 10 + Steinberg VST3 validator.

## 8. After 2.0: the first 3.0 attempts (rejected)

Goal: get closer to the hardware 1176LN's *waveform* (distortion included), not only its gain, while balancing the
hardware against the UADx. The two candidates below were rejected. Full log: `docs/V3_PLAN.md`; scripts in
`research/v3/`. All candidates were judged on held-out data with a paired bootstrap against 2.0.

### 8.1 New measurements and tools
- **Hardware waveform test** (`v3/hwwave.py`): per held-out note, the error after a per-note linear filter (ESR) and the
  added distortion a 5 ms gain + linear filter cannot explain (NLR). 2.0: ESR -24.65 dB median; UADx -15.85 dB.
  Added distortion vs hardware: 2.0 -3.11 dB (too clean), UADx +2.29 dB (too dirty).
- **UADx distortion map** (`measure/m_dist3.py`): H2 - H5 over 7 frequencies, 3 levels, attack and release (213 renders).
- **UADx thump** (`measure/m_thump.py`): LF-free 5 kHz ramped steps, so any output below 500 Hz is thump.
- **Fit = ship**: the fitting renderer is now the plugin's engine itself (`fit/libeng.so`, identical to -136 dB); the
  parameter exporter writes full precision (the 2.0 file was rounded to 6 decimals: below -89 dB in the output).
- **One-command candidate scoring** (`v3/eval_candidate.sh`).

### 8.2 What was learned
- **The hardware's extra "dirt" is not harmonic distortion.** Harmonics 2 - 10 of 2.0 are within ~1.3 dB of the
  hardware; no static distortion setting moves the hardware's added distortion (even 10x: +0.6 dB, worse waveform).
  It is not recording noise either (-45 dB re the note, ~15 dB under the gap). It is low-frequency content that
  follows the compression, strongest in the first 60 ms (2.0 is 7 dB short below f0/2, 10 dB short at the onset).
- **The onset is where the three differ.** At ALL, attack 1 the hardware holds its gain ~3 - 4 ms at a note's onset
  (1 ms frames), the UADx ~10 ms, 2.0 ~1 - 2 ms. The UADx's thump comes from its long hold: the output overshoots
  toward full scale and the class-A output stage shifts with it (LF through a ~8 Hz coupling high-pass).

### 8.3 Tried and rejected
| Candidate | Better | Worse (beyond noise) |
|---|---|---|
| Joint refit with a quadratic knee and odd-order distortion (fit A) | distortion vs UADx 6.35 -> 5.75 dB | hardware gain 0.464 -> 0.473 dB; hardware waveform -0.16 dB |
| ALL onset lag 1 - 4 ms (training notes) | added distortion vs hardware -3.16 -> -0.14 dB | hardware waveform -24.6 -> -21.2 dB |
| Thump (class-A shift above a level) | - | needs the overshoot of a hold; not pursued |

The options stay in the code, off; with them off the plugin's output is identical to 2.0. What the evidence points
to next: the hardware's hold is not a delay of the whole control path but a change in the attack's start -
a different attack mechanism, a larger piece of work with an uncertain payoff.

## 9. Version 3.0 (2026-11-12)

The priorities were reset by the artist: more of the UADx's FET character, but not all of it, and a thump that is
delicate yet audible, without giving up the match with the hardware. Both were built on what section 8 measured.

### 9.1 What changed
- **Fuzz**: only the distortion constants were refitted (fetDist, ampDist, fetDistIn, ampOdd, fetOdd) on the training
  half of the UADx distortion map; the gain behaviour is 2.0's, untouched. The shipped point is 75 % of the way from
  2.0 to that fit (`research/v3/fit_fuzz.py`).
- **Thump**: control feedthrough - a low bump proportional to the gain reduction in dB, softly capped at 12 dB,
  smoothed (1 ms) and passed through an 8 Hz coupling high-pass (`thumpCv`, FetModel). The first version followed
  the raw control voltage and reached -6 dB re peak under extreme drive; the capped one stays 27 - 65 dB under the
  processed signal on every factory preset (`v3/thump_delicacy.py`). Strength chosen on the hardware training notes.
- **Sessions saved with 2.0** carry no model mark and load the 2.0 model (FittedParamsV2.h, frozen as shipped):
  rendered by the 3.0 processor they are bit-identical to the released 2.0 VST3 at 8 settings
  (`v3/audit_v2_sessions.py`). New sessions are marked 3.
- **GUI**: four walnut boards; mitred joints with a hairline seam; outer round-over and inner easing shaded from one
  light on their own normals; irregular figure, fibres and tapered pores; textures rebuilt at the drawn pixel density
  (up to 4x) for HiDPI and enlarged windows.

### 9.2 Held-out results (paired bootstrap vs 2.0, `v3/eval_candidate.sh`)
| | 2.0 | **3.0** | 3.0 - 2.0, 95 % CI |
|---|---:|---:|---|
| Harmonics H2 - H5 vs UADx, 103 held-out points (mean) | 6.35 dB | **5.88 dB** | -0.47 [-0.67, -0.29] |
| ... RMS / 3rd harmonic RMS | 7.18 / 8.38 | **6.76 / 6.90** | |
| Added distortion vs hardware, median abs / bias | 2.26 / -3.11 | **2.03 / -2.65** | |
| Hardware gain trajectory, 593 notes (mean) | 0.4643 | 0.4660 | +0.002 [-0.000, +0.004], within noise |
| Hardware waveform ESR (mean) | -25.05 | -25.07 | -0.01 [-0.04, +0.01], within noise |
| MUSDB18 held-out clips (mean) | 0.6854 | 0.6849 | -0.0005 [-0.0009, -0.0000] |
| The artist's takes / desktop clips (mean) | 1.0594 / 0.6867 | 1.0595 / 0.6869 | within noise |
| Static curves / dynamic envelopes RMS | 1.01 / 1.01 | 1.01 / 1.01 | |

Tests: DSP 42, plugin 19 (5 new: model marks, reload, the 2.0 model kept, the two models differ), pluginval strictness
10 with Steinberg's VST3 validator.

## 10. Version 3.1: the preset pack (2026-11-12)

40 new factory presets (52 with the 12 of 2.0, which keep their slots), in seven menu groups. Each is a stated intent in
numbers (`research/presets/specs.py`): targets on changes versus the dry signal at equal loudness - crest, band shares
(boom 80-160 Hz, low-mid, presence), low-mid onset punch, transient, loudness range, added harmonics against the
engine's clean path, gain reduction, pumping; for pocket presets the duck while the vocal sounds, while it is silent and
within a phrase. Settings were found by random search plus local refinement through the shipping engine
(`research/presets/tune.py`, cached renders; MIX and OUTPUT computed exactly from one render), OUTPUT set for equal
BS.1770 loudness, and every preset verified at 4x on its tuning material and on held-out material. Per-preset results:
docs/PRESETS.md.

- **The artist's voice** (`research/presets/voice_profile.py`, vs 94 MUSDB18 vocal stems at equal loudness): ~25 dB more
  energy at 80 - 160 Hz (room and proximity boom), 1.3 - 4.4 dB less presence (2.5 - 6 kHz), 5 - 7 dB more crest,
  less harmonic bite. A first exploration (240 settings) showed what a 1176 can do to it: crest down ~6 dB at most,
  presence share up to +2.4 dB, onset low-mid punch up to +4 dB - and the boom share rising up to +6 dB under heavy
  compression. The Signature presets therefore keep the boom out of the detector (side-chain HPF 100 Hz) and within
  ~1.5 dB of the dry voice. They are tuned on the dry vocal plus a stretch of the home takes and held out on another.
- **Second pass**: 21 presets whose first-pass targets proved infeasible (crest on a peaky voice or a mastered beat,
  'more snap' with an 800 us slowest attack, pocket depth with breaths in the key) were re-targeted to the measured
  achievable range and retuned; two pocket presets were renamed to the depth they measurably deliver (2.5 and 4 dB).
- **Level**: presets are calibrated for material at a working level (vocals -20 LUFS, drums/bass -18, beats/mixes -12);
  quieter material compresses less (the held-out takes about half as much). Move INPUT to reach the stated GR.
- **Fix**: a host can leave a switch at an in-between value; JUCE's replaceState does not move a parameter whose saved
  value equals the tree's, so it kept 0.37. Every parameter now gets its saved value on load (pluginval, seed 0x6699bf6;
  present since 2.0).
- Sessions: 2.0 sessions bit-identical to the 2.0.0 release, 3.0 sessions bit-identical to 3.0.0. Tests: DSP 42, plugin 22
  (every preset loads exactly; the 2.0 presets keep their numbers; pocket presets switch EXT on), pluginval strictness
  10 x 3 + Steinberg's VST3 validator.

## 11. Version 3.2: anchored to the hardware (2026-11-12)

### 11.1 A circuit model first (research/circuit)
The UREI 1176LN schematic (rev F, S/N suffix K, with static DC voltages) was transcribed into an ngspice netlist of the
whole compression path: input pot, the FET gain cell (gate audio feedback through R7 / R9, source inside the preamp loop),
the preamp, output pot and ratio ladder B, the GR control amp, the full-wave FD333 detector, attack / release / hold
capacitor, ratio ladder A (threshold bias) and the Q-bias trim. The netlist reproduces every printed DC voltage within
~3 %, the Q-bias trim is set by the adjustment procedure (-1 dB at rest), and the static curves show the 1176's ordering
(ratio thresholds rising -24 / -20 / -20 / -16 dBV). Two findings: the 1176 is a **feedback** compressor (the
side-chain is taken after the FET); and ALL closes both ladders at once (less side-chain gain than 20:1, the lowest
threshold). On the hardware notes, though, the generic-parts circuit scored 1.57 dB (attack path too slow at ATTACK 1,
~2 dB too little gain reduction); a tolerance calibration moved it only to 1.61 -> stopped in favour of 11.2.

### 11.2 The joint fit (research/hwfit)
Axell 76's own model fitted to the UADx terms of 3.0 (static, dynamics, 300 music clips, distortion) and the hardware
1176LN calibration notes (149 of the even notes; the 593 odd notes never enter a fit), the hardware term weighted equal
to all UADx terms at the start. 20 parameters (ALL law, timing, release memory, ALL attack, detector) + the input level.
Calibration notes 0.480 -> 0.450 dB. A variant with the feedback blend free ended at feedback 0.0005 and the same score:
the data does not ask for output-referred detection.

| held out, paired vs 3.1 | 3.1 | **3.2** | 3.2 - 3.1, 95 % CI |
|---|---:|---:|---|
| Hardware 1176LN gain trajectory, 593 notes (mean / median / p90) | 0.466 / 0.410 / 0.807 | **0.435 / 0.370 / 0.796** | -0.031 [-0.034, -0.028] |
| Hardware waveform ESR (mean) | -25.07 dB | **-25.73 dB** | -0.66 [-0.71, -0.61] |
| Harmonics vs UADx, 103 points (mean) | 5.88 dB | **5.58 dB** | -0.30 [-0.70, -0.03] |
| MUSDB18 vs UADx, 100 clips (mean) | 0.685 | 0.700 | **+0.015 [+0.005, +0.026]** (the trade-off) |
| The artist's takes / desktop clips | 1.060 / 0.687 | 1.063 / 0.707 | within noise |
| Static curves / dynamic envelopes | 1.01 / 1.01 | 1.01 / 0.99 | |

The one cost - music a little less UADx-like - is the balance the project asked for: where the hardware has data it
decides. Released with the user's approval. Sessions: model marks 2 (2.0), 3 (3.0/3.1), 4 (3.2); 2.0 sessions render
bit-identically to the 2.0.0 bundle and 3.x sessions to the 3.1.0 bundle (8 settings each). Presets re-checked on the
3.2 model: 2 of 40 OUTPUT values re-matched (> 0.25 dB). Tests: DSP 42, plugin 25.
