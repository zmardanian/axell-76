# Axell 76 presets

52 factory presets: the 12 of 2.0 (unchanged, same slots) and the 3.1 pack below. Every 3.1 preset is a stated intent in
numbers, found by search through the plugin's own engine on real material arriving at a standard working level
(vocals -20 LUFS, drums and bass -18, beats and mixes -12), loudness-matched to the dry signal (A/B at equal
loudness), and checked at the default 4x quality on material it was not tuned on. Changes are versus the dry signal
at equal loudness. If your track is hotter or quieter than the working level, move INPUT until the GR meter shows
about the stated gain reduction. Method and scripts: `research/presets/`.

## Signature

Tuned on the artist's own voice (dry vocal + home takes; held out on another stretch of takes). The voice measured ~25 dB of room boom under 160 Hz, 5-7 dB more peak-to-average than produced vocals, 1-4 dB less presence and less harmonic bite. Every Signature preset filters the boom out of the detector and keeps the boom share within ~1.5 dB. For the biggest win put a high-pass at ~100 Hz before Axell 76 (e.g. Axell Penta EQ): a compressor cannot remove boom.

### Signature - Main Lead

The everyday lead: chest punch, grit and presence together, peaks tamed.

INPUT +9.3 dB · OUTPUT -0.8 dB · ATTACK 6.0 · RELEASE 5.4 · RATIO 20:1 · MIX 85 % · SC HPF 100 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| peaks vs body (crest) (dB) | -3.1 | -2.9 |
| low-mid punch at onsets (dB) | +1.0 | +0.3 |
| added harmonics (grit) (dB re output) | -29.2 | -29.3 |
| presence 2.5-6 kHz share (dB) | +0.9 | +2.1 |
| boom 80-160 Hz share (dB) | +1.7 | +0.5 |
| gain reduction, median (dB) | 5.5 | 2.6 |

### Signature - Chest Punch

Lets each syllable's chest through before clamping: low-mid punch.

INPUT +6.2 dB · OUTPUT +1.2 dB · ATTACK 2.0 · RELEASE 5.4 · RATIO 8:1 · MIX 100 % · SC HPF 100 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| low-mid punch at onsets (dB) | +1.2 | +0.4 |
| low-mid share (dB) | +0.5 | -1.5 |
| peaks vs body (crest) (dB) | -2.9 | -2.7 |
| boom 80-160 Hz share (dB) | +1.7 | +0.5 |
| gain reduction, median (dB) | 4.7 | 2.4 |
| added harmonics (grit) (dB re output) | -34.7 | -36.0 |

### Signature - Presence Forward

Lifts consonants and word endings: more presence and upper mids.

INPUT +5.1 dB · OUTPUT +2.7 dB · ATTACK 6.5 · RELEASE 5.0 · RATIO 8:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| presence 2.5-6 kHz share (dB) | +1.0 | +2.7 |
| upper-mid share (dB) | -0.4 | -0.7 |
| peaks vs body (crest) (dB) | -3.2 | -4.2 |
| boom 80-160 Hz share (dB) | +1.5 | +0.4 |
| low-mid punch at onsets (dB) | +0.9 | +0.3 |
| gain reduction, median (dB) | 5.4 | 3.7 |

### Signature - Grit & Bite

The FET's odd harmonics on purpose: grit the dry voice lacks.

INPUT +1.6 dB · OUTPUT +0.6 dB · ATTACK 7.0 · RELEASE 5.0 · RATIO ALL · MIX 55 % · SC HPF 100 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| added harmonics (grit) (dB re output) | -21.7 | -20.9 |
| peaks vs body (crest) (dB) | -2.2 | -1.7 |
| presence 2.5-6 kHz share (dB) | +0.5 | +0.7 |
| boom 80-160 Hz share (dB) | +1.2 | +0.3 |

### Signature - Boom Bap Lead

Locked level for a busy boom bap beat: words never drop under the drums.

INPUT +1.3 dB · OUTPUT +3.5 dB · ATTACK 7.0 · RELEASE 7.0 · RATIO 4:1 · MIX 100 % · SC HPF 100 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| peaks vs body (crest) (dB) | -3.0 | -2.2 |
| loudness range (dB) | -4.0 | -1.0 |
| added harmonics (grit) (dB re output) | -30.8 | -34.0 |
| low-mid punch at onsets (dB) | +0.4 | +0.0 |
| boom 80-160 Hz share (dB) | +1.7 | +0.4 |
| pumping (dB / 10 ms) | 0.6 | 0.5 |
| gain reduction, median (dB) | 1.7 | 0.6 |

### Signature - Smooth Control

Gentle and clean for melodic parts: peaks caught, colour untouched.

INPUT -1.4 dB · OUTPUT +6.0 dB · ATTACK 3.5 · RELEASE 4.5 · RATIO 4:1 · MIX 100 % · SC HPF 100 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| peaks vs body (crest) (dB) | -2.0 | -1.8 |
| gain reduction, median (dB) | 3.1 | 1.9 |
| added harmonics (grit) (dB re output) | -39.8 | -42.0 |
| low-mid punch at onsets (dB) | +0.3 | +0.0 |
| boom 80-160 Hz share (dB) | +1.0 | +0.3 |
| pumping (dB / 10 ms) | 0.2 | 0.2 |

### Signature - Ad-lib Smash

Ad-libs and hype doubles: flattened, gritty, in your face.

INPUT +22.1 dB · OUTPUT -4.4 dB · ATTACK 6.9 · RELEASE 7.0 · RATIO 20:1 · MIX 100 % · SC HPF 100 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 11.9 | 5.5 |
| added harmonics (grit) (dB re output) | -23.8 | -24.9 |
| peaks vs body (crest) (dB) | -4.0 | -2.2 |

### Signature - Parallel Thickener

Heavy compression blended under the dry voice: body and density, transients kept.

INPUT +25.4 dB · OUTPUT +0.8 dB · ATTACK 5.4 · RELEASE 5.0 · RATIO 12:1 · MIX 50 % · SC HPF 100 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 23.0 | 18.0 |
| low-mid punch at onsets (dB) | +0.6 | +0.0 |
| low-mid share (dB) | +0.3 | -1.0 |
| peaks vs body (crest) (dB) | -2.0 | -2.5 |
| added harmonics (grit) (dB re output) | -21.5 | -20.8 |
| boom 80-160 Hz share (dB) | +1.2 | +0.3 |

### Signature - Radio Forward

Maximum forwardness: dense, present, gritty - for a loud final mix.

INPUT +7.8 dB · OUTPUT -1.0 dB · ATTACK 7.0 · RELEASE 5.9 · RATIO 20:1 · MIX 85 % · SC HPF 100 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| peaks vs body (crest) (dB) | -3.2 | -2.8 |
| loudness range (dB) | -4.6 | -1.3 |
| presence 2.5-6 kHz share (dB) | +0.9 | +1.9 |
| added harmonics (grit) (dB re output) | -28.8 | -29.3 |
| boom 80-160 Hz share (dB) | +1.8 | +0.5 |
| gain reduction, median (dB) | 3.4 | 1.0 |

### Signature - Low-Energy Lift

Quiet or laid-back takes: levels the phrase, brings detail up, stays clean.

INPUT +0.5 dB · OUTPUT +5.0 dB · ATTACK 6.0 · RELEASE 5.0 · RATIO 4:1 · MIX 100 % · SC HPF 100 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| loudness range (dB) | -4.4 | -1.0 |
| presence 2.5-6 kHz share (dB) | +0.6 | +1.2 |
| peaks vs body (crest) (dB) | -2.4 | -2.1 |
| added harmonics (grit) (dB re output) | -37.0 | -39.2 |
| boom 80-160 Hz share (dB) | +1.2 | +0.4 |
| gain reduction, median (dB) | 3.6 | 2.1 |

## Vocal

Tuned on 8 singers (MUSDB18 vocal stems), held out on 8 others.

### Vocal - Transparent Levelling

Invisible: peaks and level evened, tone and texture unchanged.

INPUT -2.1 dB · OUTPUT +5.9 dB · ATTACK 3.6 · RELEASE 4.1 · RATIO 4:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 2.9 | 3.1 |
| peaks vs body (crest) (dB) | -1.5 | -1.3 |
| added harmonics (grit) (dB re output) | -42.4 | -40.5 |
| low-mid share (dB) | +1.0 | +0.5 |
| presence 2.5-6 kHz share (dB) | -0.2 | -0.0 |

### Vocal - Pop Polish

Smooth, even and a little forward.

INPUT +6.5 dB · OUTPUT +0.1 dB · ATTACK 3.1 · RELEASE 6.1 · RATIO 8:1 · MIX 100 % · SC HPF 150 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 4.7 | 5.3 |
| peaks vs body (crest) (dB) | -2.9 | -2.4 |
| presence 2.5-6 kHz share (dB) | -0.3 | +0.1 |
| added harmonics (grit) (dB re output) | -34.5 | -32.4 |

### Vocal - Rap Double

Doubles that sit tight behind the lead: flat and gritty.

INPUT +12.1 dB · OUTPUT -4.0 dB · ATTACK 6.7 · RELEASE 7.0 · RATIO ALL · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 8.8 | 9.1 |
| peaks vs body (crest) (dB) | -4.5 | -4.1 |
| added harmonics (grit) (dB re output) | -18.1 | -16.7 |

### Vocal - Backing Glue

Backing vocals pushed behind: softened onsets, steady level.

INPUT +2.7 dB · OUTPUT +2.6 dB · ATTACK 6.7 · RELEASE 2.5 · RATIO 8:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 4.3 | 4.8 |
| peaks vs body (crest) (dB) | -2.3 | -1.7 |
| attack of hits/syllables (dB) | -0.2 | +0.2 |
| added harmonics (grit) (dB re output) | -40.8 | -38.2 |

### Vocal - Spoken Word

Speech, podcast, intros: steady and intelligible, clean.

INPUT +6.7 dB · OUTPUT +1.5 dB · ATTACK 3.6 · RELEASE 5.7 · RATIO 4:1 · MIX 100 % · SC HPF 60 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| loudness range (dB) | -5.1 | -3.3 |
| peaks vs body (crest) (dB) | -2.6 | -2.3 |
| added harmonics (grit) (dB re output) | -35.4 | -33.9 |
| gain reduction, median (dB) | 6.5 | 7.0 |

### Vocal - Hook Lift (Parallel)

A hook that jumps out: parallel density, punch and presence.

INPUT +24.4 dB · OUTPUT -2.0 dB · ATTACK 2.1 · RELEASE 4.7 · RATIO ALL · MIX 65 % · SC HPF 60 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| low-mid punch at onsets (dB) | +0.7 | +1.3 |
| presence 2.5-6 kHz share (dB) | +0.1 | +0.4 |
| peaks vs body (crest) (dB) | -2.3 | -1.9 |

## Beat

For a 2-track beat on its own insert, tuned on a boom bap beat and held out on another (both at a mastered -12 LUFS).

### Beat - Boom Bap Glue

Holds the loop together without losing the knock.

INPUT -13.5 dB · OUTPUT +16.3 dB · ATTACK 4.9 · RELEASE 5.1 · RATIO 4:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 1.8 | 1.9 |
| peaks vs body (crest) (dB) | -0.4 | -0.7 |
| attack of hits/syllables (dB) | +0.0 | +0.2 |
| added harmonics (grit) (dB re output) | -37.6 | -38.8 |

### Beat - Knock

More kick and snare attack: hits come through before the clamp.

INPUT -6.9 dB · OUTPUT +11.5 dB · ATTACK 1.0 · RELEASE 3.8 · RATIO 8:1 · MIX 100 % · SC HPF 60 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| attack of hits/syllables (dB) | +0.7 | +0.4 |
| gain reduction, median (dB) | 3.5 | 3.3 |
| boom 80-160 Hz share (dB) | -0.1 | +0.2 |
| added harmonics (grit) (dB re output) | -36.3 | -36.4 |

### Beat - Lo-Fi Grit

Dusty, saturated, a little squashed - tape-and-sampler character.

INPUT +1.8 dB · OUTPUT +6.4 dB · ATTACK 6.4 · RELEASE 5.8 · RATIO ALL · MIX 70 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| added harmonics (grit) (dB re output) | -22.6 | -23.1 |
| gain reduction, median (dB) | 6.2 | 6.6 |
| peaks vs body (crest) (dB) | -2.3 | -2.4 |

### Beat - Parallel Smack

Drums get heavier and louder without losing their crack (parallel).

INPUT +9.5 dB · OUTPUT +7.0 dB · ATTACK 5.0 · RELEASE 4.6 · RATIO ALL · MIX 50 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 14.3 | 15.0 |
| attack of hits/syllables (dB) | -0.3 | +0.1 |
| boom 80-160 Hz share (dB) | -0.2 | +0.2 |
| peaks vs body (crest) (dB) | -1.3 | -1.5 |

### Beat - Tame the Master

Catches only the loudest peaks of a hot beat; transparent otherwise.

INPUT -10.8 dB · OUTPUT +12.6 dB · ATTACK 6.1 · RELEASE 4.7 · RATIO 12:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, loud moments (dB) | 2.3 | 2.3 |
| gain reduction, median (dB) | 1.0 | 1.1 |
| peaks vs body (crest) (dB) | -0.3 | -0.6 |
| added harmonics (grit) (dB re output) | -40.6 | -40.1 |

### Beat - Breathing Pump

Audible, musical pumping with the groove.

INPUT +1.5 dB · OUTPUT +6.1 dB · ATTACK 7.0 · RELEASE 5.9 · RATIO ALL · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| pumping (dB / 10 ms) | 2.1 | 2.2 |
| gain reduction, median (dB) | 5.9 | 6.2 |
| peaks vs body (crest) (dB) | -3.2 | -3.4 |

### Beat - Dense Loop

Sample chops and loops get dense and even.

INPUT -7.3 dB · OUTPUT +12.8 dB · ATTACK 7.0 · RELEASE 5.5 · RATIO 4:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| peaks vs body (crest) (dB) | -1.0 | -1.3 |
| loudness range (dB) | -2.4 | -2.9 |
| gain reduction, median (dB) | 3.6 | 3.9 |
| added harmonics (grit) (dB re output) | -32.3 | -34.2 |

### Beat - Warm Saturation

Mostly colour: light compression, warm FET harmonics.

INPUT -4.5 dB · OUTPUT +7.8 dB · ATTACK 3.9 · RELEASE 4.3 · RATIO ALL · MIX 100 % · SC HPF 60 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| added harmonics (grit) (dB re output) | -27.4 | -29.4 |
| gain reduction, median (dB) | 1.4 | 1.2 |
| peaks vs body (crest) (dB) | -0.9 | -1.0 |

### Beat - Low End Steady

The bass and kick sit at a steady level under the vocal.

INPUT -7.7 dB · OUTPUT +12.0 dB · ATTACK 5.7 · RELEASE 4.6 · RATIO 8:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| peaks vs body (crest) (dB) | -0.7 | -0.9 |
| loudness range (dB) | -2.1 | -2.7 |
| gain reduction, median (dB) | 3.0 | 3.0 |
| added harmonics (grit) (dB re output) | -34.3 | -34.8 |

### Beat - Ready for Vocals

Evens the beat's level so the vocal has a steady bed to sit on.

INPUT -9.0 dB · OUTPUT +12.5 dB · ATTACK 3.5 · RELEASE 4.2 · RATIO 8:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| loudness range (dB) | -1.8 | -2.6 |
| gain reduction, median (dB) | 2.5 | 2.6 |
| attack of hits/syllables (dB) | +0.0 | +0.1 |
| added harmonics (grit) (dB re output) | -36.8 | -36.6 |

## Pocket

**Routing**: put Axell 76 on the beat, send the lead vocal to Axell 76's side-chain input (the preset switches SIDE-CHAIN to EXT). The beat then dips only while you rap, by the stated amount, and returns between phrases. The key is the vocal at about -20 LUFS; a hotter or quieter send changes the depth - adjust INPUT. Without a side-chain signal the plugin listens to the beat itself, which is not what these presets are for.

### Pocket - Whisper 1 dB

The beat moves out of the way just enough to notice.

INPUT -3.7 dB · OUTPUT +0.0 dB · ATTACK 4.5 · RELEASE 4.1 · RATIO 12:1 · MIX 100 % · SC HPF 200 Hz · SIDE-CHAIN EXT

| what it does | tuned on | held out |
|---|---:|---:|
| duck while the vocal sounds (dB) | 1.0 | 0.2 |
| duck while the vocal is silent (dB) | 0.6 | 0.0 |
| duck movement within a phrase (dB) | 0.8 | 0.9 |

### Pocket - Gentle 2 dB

A clear but unnoticeable pocket for the vocal.

INPUT -4.8 dB · OUTPUT +0.0 dB · ATTACK 6.0 · RELEASE 5.0 · RATIO 4:1 · MIX 100 % · SC HPF 200 Hz · SIDE-CHAIN EXT

| what it does | tuned on | held out |
|---|---:|---:|
| duck while the vocal sounds (dB) | 1.9 | 0.8 |
| duck while the vocal is silent (dB) | 0.6 | 0.0 |
| duck movement within a phrase (dB) | 1.0 | 1.2 |

### Pocket - Clear 2.5 dB

The vocal clearly on top, the beat steps back while you rap.

INPUT -2.7 dB · OUTPUT +0.0 dB · ATTACK 5.6 · RELEASE 5.0 · RATIO 4:1 · MIX 100 % · SC HPF 200 Hz · SIDE-CHAIN EXT

| what it does | tuned on | held out |
|---|---:|---:|
| duck while the vocal sounds (dB) | 2.6 | 1.1 |
| duck while the vocal is silent (dB) | 0.8 | 0.0 |
| duck movement within a phrase (dB) | 1.3 | 1.6 |

### Pocket - Deep 4 dB

A dense beat makes room for a quiet or soft vocal.

INPUT +3.1 dB · OUTPUT +0.0 dB · ATTACK 2.9 · RELEASE 5.0 · RATIO 8:1 · MIX 100 % · SC HPF 200 Hz · SIDE-CHAIN EXT

| what it does | tuned on | held out |
|---|---:|---:|
| duck while the vocal sounds (dB) | 3.8 | 1.1 |
| duck while the vocal is silent (dB) | 1.3 | 0.1 |
| duck movement within a phrase (dB) | 2.1 | 2.5 |

### Pocket - Word by Word

The beat breathes with every word - fast, rhythmic ducking.

INPUT +5.4 dB · OUTPUT +0.0 dB · ATTACK 6.4 · RELEASE 5.8 · RATIO 20:1 · MIX 100 % · SC HPF 200 Hz · SIDE-CHAIN EXT

| what it does | tuned on | held out |
|---|---:|---:|
| duck while the vocal sounds (dB) | 2.4 | 0.2 |
| duck movement within a phrase (dB) | 2.3 | 2.3 |
| duck while the vocal is silent (dB) | 0.3 | 0.0 |

### Pocket - Phrase Hold

One smooth dip per phrase - the beat stays down while you're on the mic.

INPUT -11.5 dB · OUTPUT +0.0 dB · ATTACK 4.4 · RELEASE 1.0 · RATIO 4:1 · MIX 100 % · SC HPF 200 Hz · SIDE-CHAIN EXT

| what it does | tuned on | held out |
|---|---:|---:|
| duck while the vocal sounds (dB) | 1.1 | 0.5 |
| duck movement within a phrase (dB) | 0.4 | 0.6 |
| duck while the vocal is silent (dB) | 0.9 | 0.1 |

## Drums

Tuned on MUSDB18 drum stems, held out on others.

### Drums - Room Glue

A kit that sounds like one instrument in one room.

INPUT -8.4 dB · OUTPUT +12.1 dB · ATTACK 4.2 · RELEASE 3.0 · RATIO 4:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 3.0 | 3.2 |
| peaks vs body (crest) (dB) | -0.9 | -1.6 |
| attack of hits/syllables (dB) | -0.5 | +0.1 |
| added harmonics (grit) (dB re output) | -36.0 | -40.0 |

### Drums - Punch Snap

More attack on every hit.

INPUT -6.0 dB · OUTPUT +9.1 dB · ATTACK 1.9 · RELEASE 4.1 · RATIO 4:1 · MIX 100 % · SC HPF 200 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| attack of hits/syllables (dB) | -0.4 | +0.1 |
| gain reduction, median (dB) | 2.0 | 2.2 |
| added harmonics (grit) (dB re output) | -36.1 | -43.3 |

### Drums - Parallel NY

New York parallel compression: weight and loudness, crack intact.

INPUT +20.2 dB · OUTPUT +3.4 dB · ATTACK 3.9 · RELEASE 5.2 · RATIO 20:1 · MIX 50 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 20.5 | 21.0 |
| attack of hits/syllables (dB) | -0.5 | -0.0 |
| peaks vs body (crest) (dB) | -1.8 | -2.1 |

### Drums - Break Crush

A crushed, distorted break - all buttons.

INPUT +15.0 dB · OUTPUT -1.1 dB · ATTACK 6.7 · RELEASE 6.2 · RATIO ALL · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 11.6 | 10.5 |
| added harmonics (grit) (dB re output) | -20.0 | -20.9 |
| peaks vs body (crest) (dB) | -6.2 | -8.0 |

## Bass

Tuned on MUSDB18 bass stems, held out on others.

### Bass - Steady Low End

Every note the same weight.

INPUT +0.9 dB · OUTPUT +4.8 dB · ATTACK 5.2 · RELEASE 3.7 · RATIO 8:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| loudness range (dB) | -4.1 | -2.5 |
| peaks vs body (crest) (dB) | -2.2 | -3.0 |
| added harmonics (grit) (dB re output) | -35.8 | -37.4 |

### Bass - Growl

An even, growling bass with FET harmonics that read on small speakers.

INPUT +5.1 dB · OUTPUT +0.8 dB · ATTACK 6.9 · RELEASE 5.4 · RATIO ALL · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| added harmonics (grit) (dB re output) | -22.9 | -21.1 |
| peaks vs body (crest) (dB) | -4.6 | -5.2 |

## Mix Bus

Tuned on MUSDB18 full mixes, held out on others.

### Mix Bus - Gentle Glue 1 dB

One dB of glue on the whole mix; you feel it more than hear it.

INPUT -16.8 dB · OUTPUT +18.0 dB · ATTACK 2.2 · RELEASE 6.0 · RATIO 4:1 · MIX 100 % · SC HPF 60 Hz

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 1.0 | 1.0 |
| peaks vs body (crest) (dB) | -0.2 | -0.6 |
| added harmonics (grit) (dB re output) | -44.8 | -45.0 |
| attack of hits/syllables (dB) | -0.4 | -0.0 |

### Mix Bus - Loud & Dense

A dense, loud mix bus before the limiter.

INPUT -6.0 dB · OUTPUT +10.2 dB · ATTACK 5.9 · RELEASE 6.0 · RATIO 8:1 · MIX 100 % · SC HPF off

| what it does | tuned on | held out |
|---|---:|---:|
| gain reduction, median (dB) | 3.9 | 3.6 |
| peaks vs body (crest) (dB) | -1.6 | -2.3 |
| added harmonics (grit) (dB re output) | -32.4 | -33.1 |
| attack of hits/syllables (dB) | -0.2 | -0.3 |
