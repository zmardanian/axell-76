// Axell 76 engine: input -> 2x oversampled FET model -> output, parallel mix. JUCE-free.
#pragma once
#include "FetModel.h"
#include "FittedParams.h"
#include "FittedParamsV2.h"
#include "FittedParamsV3.h"
#include <axell/dsp/Filters.h>
#include <axell/dsp/Oversampler.h>
#include <array>
#include <atomic>
#include <vector>

namespace axell76
{
struct Params
{
    float inputDb = 0.0f, outputDb = 0.0f;   // honest dB
    float attack = 4.0f, release = 4.0f;     // 1..7, 7 fastest (hardware convention)
    int ratio = 0;                           // 4, 8, 12, 20, ALL
    float mix = 100.0f;                      // %
    bool bypass = false;
    float scHpfHz = 0.0f;                    // side-chain high-pass, 0 = off
    bool extSc = false;                      // detect from the external side-chain input
    int meter = 0;                           // 0 GR, 1 output +4, 2 output +8 (display only)
};

// attack / release knob -> milliseconds as measured (63 % point of a 10 dB step; display only)
inline float attackMs (float knob) { const auto& m = fittedParams(); return knobTime (knob, m.atkSlowMs, m.atkFastMs, m.atkCurve); }
inline float releaseMs (float knob) { const auto& m = fittedParams(); return knobTime (knob, m.relSlowMs, m.relFastMs, m.relCurve); }

class Engine
{
public:
    int kOsLog2 = 1;   // oversampling (log2), set before prepare(): QUALITY
    // the internal reference: INPUT 0 dB puts the 4:1 knee at -18 dBFS (sine RMS)
    static float internalOffsetDb() { return fittedParams().law[0].thrDb + 18.0f - 3.0103f; }

    void setOversamplingLog2 (int l) noexcept { kOsLog2 = std::clamp (l, 0, 3); }
    // research: run trial model parameters through the exact shipping path (call before prepare)
    void setModelParams (const ModelParams& m) noexcept { custom = m; useCustom = true; }
    // which fitted model: 4 = current (3.2: fitted to the hardware 1176LN as well), 3 = 3.0 / 3.1 exactly, 2 = 2.0 exactly.
    // Real-time safe: the model keeps its state, only its constants change.
    void setModelGeneration (int gen) noexcept
    {
        custom = gen == 2 ? fittedParamsV2() : gen == 3 ? fittedParamsV3() : fittedParams(); useCustom = true;
        model.setParams (modelParams());
        model.setControls (p.ratio, p.attack, p.release);
    }
    const ModelParams& modelParams() const noexcept { return useCustom ? custom : fittedParams(); }
    // real-time safe QUALITY change (everything was sized in prepare): new factor, new latency, states cleared
    void switchOversampling (int l) noexcept
    {
        l = std::clamp (l, 0, 3);
        if (l == kOsLog2) return;
        kOsLog2 = l;
        model.prepare (fs * (1 << kOsLog2));
        model.setParams (modelParams());
        model.setControls (p.ratio, p.attack, p.release);
        dc.prepare (fs * (1 << kOsLog2));
        lat = os.latencyFor (kOsLog2);
        os.reset();
        osKey.reset();
        for (auto& d : delay) std::fill (d.begin(), d.end(), 0.0f);
        dpos = 0;
    }
    int oversamplingLog2() const noexcept { return kOsLog2; }
    void prepare (double sampleRate, int maxBlock, int numCh)
    {
        fs = sampleRate; channels = std::clamp (numCh, 1, 2); block = std::max (1, maxBlock);
        os.prepare (block, fs);
        model.prepare (fs * (1 << kOsLog2));
        model.setParams (modelParams());
        for (auto& b : dry) b.assign ((size_t) block, 0.0f);
        for (auto& b : key) b.assign ((size_t) block, 0.0f);
        osKey.prepare (block, fs);
        lat = os.latencyFor (kOsLog2);
        int maxLat = 0;
        for (int l = 0; l <= 3; ++l) maxLat = std::max (maxLat, os.latencyFor (l));
        for (auto& d : delay) d.assign ((size_t) (maxLat + 1), 0.0f);
        dpos = 0;
        dc.prepare (fs * (1 << kOsLog2));
        lowCut.prepare (fs);
        hpSet = -1.0f;   // re-tune the side-chain filter for this rate
        setParams (p, true);
        reset();
    }
    int latencySamples() const noexcept { return lat; }
    void reset() noexcept
    {
        os.reset(); osKey.reset(); model.reset(); dc.resetState(); lowCut.reset(); scHp.resetState();
        for (auto& d : delay) std::fill (d.begin(), d.end(), 0.0f);
        gIn = tIn; gOut = tOut; wMix = tMix;
    }
    void setParams (const Params& np, bool snap = false) noexcept
    {
        p = np;
        model.setControls (p.ratio, p.attack, p.release);
        tIn = dbToGain (p.inputDb + internalOffsetDb());
        tOut = dbToGain (p.outputDb - internalOffsetDb() + model.gainOffsetDb());
        tMix = std::clamp (p.mix, 0.0f, 100.0f) * 0.01f;
        if (p.scHpfHz > 0.0f && std::abs (p.scHpfHz - hpSet) > 0.01f) { scHp.setHighPass (fs, std::clamp (p.scHpfHz, 20.0f, 1000.0f)); hpSet = p.scHpfHz; }
        if (snap) { gIn = tIn; gOut = tOut; wMix = tMix; }
    }

    // in place, numCh <= 2, n <= maxBlock
    // sc (optional): the external side-chain (scCh channels; mono feeds both)
    void process (float* const* ch, int n, const float* const* sc = nullptr, int scCh = 0) noexcept
    {
        const int nc = channels;
        // dry copy, delayed by the oversampling latency (mix stays phase-aligned)
        for (int c = 0; c < nc; ++c)
        {
            auto& d = delay[(size_t) c];
            int pos = dpos;
            for (int i = 0; i < n; ++i)
            {
                // a NaN / Inf from upstream never gets in (it would live forever in the filter and envelope states)
                if (! (std::abs (ch[c][i]) <= 3.0e38f)) ch[c][i] = 0.0f;
                d[(size_t) pos] = ch[c][i];
                int rd = pos - lat; if (rd < 0) rd += lat + 1;
                dry[(size_t) c][(size_t) i] = d[(size_t) rd];
                if (++pos > lat) pos = 0;
            }
        }
        dpos = (dpos + n) % (lat + 1);
        if (p.bypass) { for (int c = 0; c < nc; ++c) std::copy (dry[(size_t) c].begin(), dry[(size_t) c].begin() + n, ch[c]); grDb = 0.0f; return; }

        const float sm = 1.0f - std::exp (-1.0f / (0.02f * (float) fs)); // 20 ms knob smoothing (per base sample)
        // input gain at the base rate
        const bool ext = p.extSc && sc != nullptr && scCh > 0;
        const bool keyed = ext || p.scHpfHz > 0.0f;
        float gi = gIn;
        for (int i = 0; i < n; ++i)
        {
            gi += (tIn - gi) * sm;
            for (int c = 0; c < nc; ++c)
            {
                ch[c][i] *= gi;
                if (ext) { const float s = sc[std::min (c, scCh - 1)][i]; key[(size_t) c][(size_t) i] = (std::abs (s) <= 3.0e38f ? s : 0.0f) * gi; }   // INPUT still sets the amount
            }
        }
        gIn = gi;
        float* hi[2] = {};
        float* kh[2] = {};
        for (int c = 0; c < nc; ++c) for (int i = 0; i < n; ++i) ch[c][i] = lowCut.process (ch[c][i], c);
        if (keyed)
        {
            for (int c = 0; c < nc; ++c)
            {
                auto& kb = key[(size_t) c];
                if (! ext) std::copy (ch[c], ch[c] + n, kb.begin());
                if (p.scHpfHz > 0.0f)
                    for (int i = 0; i < n; ++i) kb[(size_t) i] = (float) scHp.process (kb[(size_t) i], c);
                kh[c] = osKey.up (kb.data(), n, c, kOsLog2);
            }
        }
        for (int c = 0; c < nc; ++c) hi[c] = os.up (ch[c], n, c, kOsLog2);
        const int N = n << kOsLog2;
        float maxGr = 0.0f;
        for (int i = 0; i < N; ++i)
        {
            float fr[2] = { hi[0][i], nc > 1 ? hi[1][i] : 0.0f }, ds[2] = {};
            if (keyed)
            {
                const float kf[2] = { kh[0][i], nc > 1 ? kh[1][i] : 0.0f };
                maxGr = std::max (maxGr, model.process (fr, nc, ds, kf));
            }
            else
                maxGr = std::max (maxGr, model.process (fr, nc, ds));
            for (int c = 0; c < nc; ++c) hi[c][i] = fr[c] + (float) dc.process (ds[c], c); // only the distortion is DC-blocked
        }
        for (int c = 0; c < nc; ++c) os.down (ch[c], n, c, kOsLog2);
        float go = gOut, wm = wMix;
        for (int i = 0; i < n; ++i)
        {
            go += (tOut - go) * sm; wm += (tMix - wm) * sm;
            for (int c = 0; c < nc; ++c) ch[c][i] = wm * ch[c][i] * go + (1.0f - wm) * dry[(size_t) c][(size_t) i];
        }
        gOut = go; wMix = wm;
        grDb = 20.0f * std::log10 (1.0f + maxGr);   // the model returns its control voltage v (GR = 1 + v)
        float ms = 0.0f;
        for (int c = 0; c < nc; ++c) { float s = 0.0f; for (int i = 0; i < n; ++i) s += ch[c][i] * ch[c][i]; ms = std::max (ms, s / (float) std::max (1, n)); }
        outMs = ms;
    }
    float gainReductionDb() const noexcept { return grDb; }
    float outputMeanSquare() const noexcept { return outMs; }   // the last block, loudest channel (the VU in +4/+8)
    const Params& params() const noexcept { return p; }

private:
    static float dbToGain (float db) noexcept { return std::pow (10.0f, db / 20.0f); }
    axell::Oversampler os, osKey;
    axell::Biquad scHp;
    float hpSet = -1.0f, outMs = 0.0f;
    axell::DcBlocker dc;
    FetModel model;
    ModelParams custom; bool useCustom = false;
    LowCut lowCut;
    Params p;
    double fs = 48000.0;
    int channels = 2, block = 512, lat = 0, dpos = 0;
    std::array<std::vector<float>, 2> dry, delay, key;
    float tIn = 1, tOut = 1, tMix = 1, gIn = 1, gOut = 1, wMix = 1;
    float grDb = 0.0f;
};
} // namespace axell76
