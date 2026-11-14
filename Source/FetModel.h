// Axell 76 - FET compressor model. JUCE-free, unit-testable.
// Clean-room: the structure is a textbook FET limiter (rectifier -> RC attack/release -> FET attenuator);
// the numbers are fitted to black-box measurements (research/measure, research/fit). No vendor code or data tables.
#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace axell76
{
// the low-cut of the input/output coupling (measured: -1 dB at 10 Hz, -8 dB at 5 Hz; a 2nd-order section at 6.66 Hz,
// Q 0.924, and a 1st-order one at 7.14 Hz fit the magnitude and phase 7..60 Hz within 0.15 dB / 2 degrees)
class LowCut
{
public:
    void prepare (double fs, double f0 = 6.66, double q = 0.924, double f1 = 7.14)
    {
        const double k = std::tan (M_PI * f0 / fs), n = 1.0 / (1.0 + k / q + k * k);
        b0 = n; b1 = -2.0 * n; b2 = n; a1 = 2.0 * (k * k - 1.0) * n; a2 = (1.0 - k / q + k * k) * n;
        const double k1 = std::tan (M_PI * f1 / fs); c0 = 1.0 / (1.0 + k1); d1 = (k1 - 1.0) / (1.0 + k1);
        reset();
    }
    void reset() { for (auto& s : st) s = {}; }
    float process (float x, int ch) noexcept
    {
        auto& s = st[(size_t) ch];
        const double y = b0 * x + s.z1; s.z1 = b1 * x - a1 * y + s.z2; s.z2 = b2 * x - a2 * y;
        const double o = c0 * (y - s.x1) - d1 * s.y1; s.x1 = y; s.y1 = o;
        return (float) o;
    }
private:
    struct S { double z1 = 0, z2 = 0, x1 = 0, y1 = 0; };
    std::array<S, 2> st {};
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, c0 = 1, d1 = 0;
};

struct RatioLaw { float thrDb, s0, s1, kneeDb; };

struct ModelParams
{
    RatioLaw law[5] = { { -30.0f, 0.80f, 0.000f, 4.0f },  // 4:1
                        { -28.0f, 0.88f, 0.000f, 3.0f },  // 8:1
                        { -27.0f, 0.92f, 0.000f, 2.5f },  // 12:1
                        { -26.0f, 0.96f, 0.000f, 2.0f },  // 20:1
                        { -28.0f, 0.96f, 0.000f, 1.0f } };// ALL
    // attack/release time constants (ms) at knob 1 and 7 and a curve (log-domain interpolation, 0 = straight)
    float atkSlowMs = 3.0f, atkFastMs = 0.1f, atkCurve = 0.0f;
    float relSlowMs = 1100.0f, relFastMs = 50.0f, relCurve = 0.0f;
    float atkBoost = 0.3f;      // attack speeds up with the overshoot (per unit of linear control error)
    float memTauMs = 1500.0f;   // program memory: slow follower of the control
    float memRel = 1.0f;        // release slows by (1 + memRel * mem)
    float creep = 0.0f;         // extra GR (dB) per unit memory: long notes compress a little more
    float allAtkMul = 1.3f, allRelMul = 0.055f, allGainDb = 1.2f;
    float fetDist = 0.0f;       // even-order FET distortion, grows with GR
    float feedback = 0.0f;      // detector source: 0 = the input (feed-forward), 1 = the output (feedback, as the 1176's
                                // side chain: it samples the preamp output, after the FET - UA 1176 manual)
    float ampDist = 0.0f;       // even-order bend of the amplifier, independent of GR
    float fetDistIn = 0.0f;
    float allAtkBoost = -1.0f;  // ALL mode's own overshoot speed-up (< 0: same as atkBoost)
    float detBlend = 0.0f;      // detector: 0 = peak, 1 = averaged (RC rectifier): sine-identical, crest-sensitive
    float detTauMs = 5.0f;      // the averaging time constant
    float allLagMs = 0.0f;
    float allAtkSlowMul = -1.0f; // ALL: attack multiplier at knob 1 (< 0: allAtkMul at both ends); measured ~3x slower than 4:1      // ALL: the control arrives late (UA's manual: distortion rises 'due to a lag time')     // FET distortion driven by the signal before the gain reduction (measured: IMD at slow release)
    float detAsym = 0.0f;       // detector asymmetry: > 0 negative half-waves count (1 - a), < 0 positive count (1 + a)
    float maxGrDb = 60.0f;      // the FET's range: measured, gain reduction stops at about 39 dB
    float atkTilt = 0.0f, allAtkTilt = 0.0f; // measured: faster attack = slightly less steady GR (fraction at knob 7)
    float ripple = 1.0f;        // 1 = rectified detector (ripple at low frequencies); unused weight hook
    // 3.0
    float kneeQuad = 0.0f;      // knee shape: 0 = softplus (2.0; an endless tail below threshold), 1 = quadratic (exactly
                                // zero below thr - knee/2: measured, the UADx does no gain reduction 15 dB under the knee)
    float ampOdd = 0.0f;        // odd-order (cubic) bend of the amplifier, independent of GR
    float fetOdd = 0.0f;        // odd-order FET distortion, grows with GR
    float relBias = 0.0f;       // release discharges toward -relBias (control units): the timing capacitor discharges to
                                // 0 V while the FET only conducts above its threshold - more ripple, a finite release
    // 3.0: thump. The class-A output stage's operating point shifts with the signal level squared; the output coupling
    // capacitor passes the changes as a low-frequency bump (measured on the UADx with LF-free 5 kHz steps, and present
    // in the hardware 1176LN's notes). DC only: no harmonics.
    float thump = 0.0f;         // strength (signed); 0 = off
    float thumpHz = 8.0f;       // the coupling high-pass the shift passes through
    float thumpMs = 1.0f;       // smoothing of the squared level (removes its 2nd harmonic)
    float thumpThr = 0.0f;      // the shift starts above this output level (internal units): measured, the UADx thumps
                                // only when the output overshoots toward full scale (ALL, slow attack)
    float thumpCv = 0.0f;       // control feedthrough, per dB of gain reduction: the FET's control voltage leaks into the
                                // audio, so every clamp gives a soft low bump (smoothed by thumpMs, through the thumpHz
                                // coupling). The gain reduction is softly capped (kThumpCapDb), so heavy limiting stays
                                // delicate. 0 = off
};

inline float knobTime (float knob, float slow, float fast, float curve) noexcept
{
    // knob 1..7, 7 fastest (hardware convention)
    float u = std::clamp ((knob - 1.0f) / 6.0f, 0.0f, 1.0f);
    u = curve >= 0.0f ? std::pow (u, 1.0f + curve) : 1.0f - std::pow (1.0f - u, 1.0f - curve);
    return std::exp (std::log (slow) + u * (std::log (fast) - std::log (slow)));
}

class FetModel
{
public:
    void prepare (double sampleRate) noexcept { fs = (float) sampleRate; update(); reset(); }
    void setParams (const ModelParams& p) noexcept { mp = p; update(); }
    void setControls (int ratioIdx, float attackKnob, float releaseKnob) noexcept
    {
        ratio = std::clamp (ratioIdx, 0, 4); atkKnob = attackKnob; relKnob = releaseKnob; update();
    }
    void reset() noexcept { v = 0.0f; mem = 0.0f; grDb = 0.0f; lastOut = 0.0f; ms = 0.0f; lagBuf.fill (0.0f); lagPos = 0; thEnv.fill (0.0f); thX1.fill (0.0f); thY1.fill (0.0f); thCv.fill (0.0f); }

    // one linked frame: in -> out (gain already applied before, output gain after). Returns the control voltage v
    // (gain reduction = 20 log10 (1 + v) dB).
    // dist (optional): receives the generated even-order part separately (the caller DC-blocks it and adds it)
    // key (optional): the side-chain signal the detector reads instead of the audio (same channel count)
    float process (float* x, int numCh, float* dist = nullptr, const float* key = nullptr) noexcept
    {
        const float* det = key != nullptr ? key : x;
        float peak = 0.0f;
        const float negW = 1.0f - std::max (0.0f, mp.detAsym), posW = 1.0f + std::min (0.0f, mp.detAsym);
        for (int c = 0; c < numCh; ++c) peak = std::max (peak, det[c] >= 0.0f ? posW * det[c] : -negW * det[c]);
        const float inPeak = peak;
        peak = (1.0f - mp.feedback) * peak + mp.feedback * lastOut;
        if (mp.detBlend > 0.0f)
        {
            // a rectifier into a capacitor reads the average more than the instant: x 1.414 makes a steady sine read
            // the same as the peak detector, so the static curve is kept while spiky material reads lower
            ms += (peak * peak - ms) * kd;
            peak = (1.0f - mp.detBlend) * peak + mp.detBlend * std::sqrt (2.0f * ms);
        }
        const RatioLaw& L = mp.law[ratio];
        const float lev = 20.0f * std::log10 (peak + 1e-9f);
        const float over = lev - L.thrDb;
        const float w = std::max (0.05f, L.kneeDb);
        // soft knee; far below the knee it is exactly 0 to float precision (exp(-20) ~ 2e-9), so skip the math
        float sp = over > 30.0f * w ? over : (over < -20.0f * w ? 0.0f : w * std::log1p (std::exp (over / w)));
        if (mp.kneeQuad > 0.0f)
        {
            const float q = over <= -0.5f * w ? 0.0f : (over >= 0.5f * w ? over : (over + 0.5f * w) * (over + 0.5f * w) / (2.0f * w));
            sp += mp.kneeQuad * (q - sp);
        }
        float target = (L.s0 * sp + 0.5f * L.s1 * sp * sp) * tilt + mp.creep * mem;
        // the FET runs out of range softly (a 3 dB knee into the limit), not with a hard clamp
        target = std::max (0.0f, target);
        {
            const float room = mp.maxGrDb - target;
            target = mp.maxGrDb - (room > 12.0f ? room : 3.0f * std::log1p (std::exp (room / 3.0f)));
            target = std::max (0.0f, target);
        }
        float vt = target <= 0.0f ? 0.0f : std::exp (target * 0.11512925f) - 1.0f; // linear "control voltage"
        if (lagN > 0)
        {
            lagBuf[(size_t) lagPos] = vt;
            int rd = lagPos - lagN; if (rd < 0) rd += kLagMax;
            vt = lagBuf[(size_t) rd];
            if (++lagPos == kLagMax) lagPos = 0;
        }
        if (vt > v)
            v += (vt - v) * std::min (1.0f, ka * (1.0f + boost * (vt - v)));
        else
            v = std::max (vt, v + (vt - mp.relBias - v) * (kr / (1.0f + mp.memRel * mem)));
        if (! (v <= 1.0e6f && v >= 0.0f)) v = 0.0f;   // state guard (never reached with finite input)
        mem += (v - mem) * km;
        const float g = 1.0f / (1.0f + v);
        lastOut = inPeak * g;
        // grDb is computed on demand (gainReductionDb), not per sample
        const float d = mp.ampDist + mp.fetDist * (1.0f - g);
        // thump: gain reduction in dB, softly capped - only computed when the thump is on
        const bool thOn = std::abs (mp.thump) > 0.0f || std::abs (mp.thumpCv) > 0.0f;
        const float thGr = thOn && std::abs (mp.thumpCv) > 0.0f ? kThumpCapDb * std::tanh (8.6858896f * std::log1p (v) / kThumpCapDb) : 0.0f;
        for (int c = 0; c < numCh; ++c)
        {
            const float s = x[c] * g;
            float th = 0.0f;
            const float e = d * s * s + mp.fetDistIn * (1.0f - g) * x[c] * s       // even-order bends (DC removed by the caller)
                          + (mp.ampOdd + mp.fetOdd * (1.0f - g)) * s * s * s;      // odd-order
            if (thOn)
            {
                thEnv[(size_t) c] += (s * s - thEnv[(size_t) c]) * kth;
                const float above = std::max (0.0f, std::sqrt (2.0f * thEnv[(size_t) c]) - mp.thumpThr);   // peak level above the threshold
                thCv[(size_t) c] += (thGr - thCv[(size_t) c]) * kth;
                const float b = mp.thump * above * above + mp.thumpCv * thCv[(size_t) c];
                th = rth * (thY1[(size_t) c] + b - thX1[(size_t) c]); thX1[(size_t) c] = b; thY1[(size_t) c] = th;
            }
            if (dist != nullptr) { x[c] = s + th; dist[c] = e; }
            else x[c] = s + e + th;
        }
        return v;
    }
    float gainOffsetDb() const noexcept { return ratio == 4 ? mp.allGainDb : 0.0f; }
    const ModelParams& params() const noexcept { return mp; }

private:
    void update() noexcept
    {
        const bool all = ratio == 4;
        float allMul = mp.allAtkMul;
        if (all && mp.allAtkSlowMul > 0.0f)
        {
            const float u = std::clamp ((atkKnob - 1.0f) / 6.0f, 0.0f, 1.0f);
            allMul = std::exp (std::log (mp.allAtkSlowMul) * (1.0f - u) + std::log (std::max (0.01f, mp.allAtkMul)) * u);
        }
        const float ta = knobTime (atkKnob, mp.atkSlowMs, mp.atkFastMs, mp.atkCurve) * (all ? allMul : 1.0f);
        const float tr = knobTime (relKnob, mp.relSlowMs, mp.relFastMs, mp.relCurve) * (all ? mp.allRelMul : 1.0f);
        ka = 1.0f - std::exp (-1000.0f / (std::max (0.005f, ta) * fs));
        kr = 1.0f - std::exp (-1000.0f / (std::max (0.1f, tr) * fs));
        km = 1.0f - std::exp (-1000.0f / (std::max (1.0f, mp.memTauMs) * fs));
        kd = 1.0f - std::exp (-1000.0f / (std::max (0.05f, mp.detTauMs) * fs));
        boost = (all && mp.allAtkBoost >= 0.0f) ? mp.allAtkBoost : mp.atkBoost;
        lagN = all ? std::clamp ((int) std::lround (mp.allLagMs * 0.001f * fs), 0, kLagMax - 1) : 0;
        tilt = 1.0f - (all ? mp.allAtkTilt : mp.atkTilt) * std::clamp ((atkKnob - 1.0f) / 6.0f, 0.0f, 1.0f);
        kth = 1.0f - std::exp (-1000.0f / (std::max (0.05f, mp.thumpMs) * fs));
        rth = std::exp (-2.0f * 3.14159265f * std::max (0.1f, mp.thumpHz) / fs);
    }
    ModelParams mp;
    float fs = 48000.0f, ka = 0.1f, kr = 0.001f, km = 0.0001f, tilt = 1.0f, kd = 0.01f, boost = 0.3f;
    int ratio = 0; float atkKnob = 4.0f, relKnob = 4.0f;
    float v = 0.0f, mem = 0.0f, grDb = 0.0f, lastOut = 0.0f, ms = 0.0f;
    static constexpr int kLagMax = 16384;   // 3 ms at 8x of 192 kHz = 4608 samples: room to spare
    std::array<float, kLagMax> lagBuf {};
    int lagPos = 0, lagN = 0;
    float kth = 0.02f, rth = 0.999f;
    static constexpr float kThumpCapDb = 12.0f;
    std::array<float, 2> thEnv {}, thX1 {}, thY1 {}, thCv {};
};
} // namespace axell76
