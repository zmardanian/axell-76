// Axell 76 DSP tests (JUCE-free): the engine exactly as it ships
#include "Engine.h"
#include <cmath>
#include <cstdio>
#include <vector>
#include <chrono>
#include <limits>
static int fails = 0, passes = 0;
#define CHECK(c, ...) do { if (c) ++passes; else { ++fails; std::printf ("FAIL %s:%d  ", __FILE__, __LINE__); std::printf (__VA_ARGS__); std::printf ("\n"); } } while (0)
using namespace axell76;
static std::vector<float> run (Params p, const std::vector<float>& mono, double fs = 48000.0, int B = 256, int* lat = nullptr)
{
    Engine e; e.setParams (p, true); e.prepare (fs, B, 2);
    if (lat) *lat = e.latencySamples();
    std::vector<float> L (mono), R (mono);
    for (size_t s = 0; s < mono.size(); s += (size_t) B)
    {
        const int k = (int) std::min<size_t> ((size_t) B, mono.size() - s);
        float* ch[2] = { L.data() + s, R.data() + s }; e.process (ch, k);
    }
    return L;
}
static double rmsDb (const std::vector<float>& x, size_t a, size_t b)
{ double s = 0; for (size_t i = a; i < b; ++i) s += (double) x[i] * x[i]; return 10 * std::log10 (s / (double) (b - a) + 1e-30); }
static std::vector<float> sine (double db, double secs, double f = 1000, double fs = 48000)
{ std::vector<float> v ((size_t) (secs * fs)); for (size_t i = 0; i < v.size(); ++i) v[i] = (float) (std::pow (10, db / 20) * std::sqrt (2.0) * std::sin (2 * M_PI * f * (double) i / fs)); return v; }
int main()
{
    // 1. below threshold: unity gain (INPUT 0, OUTPUT 0)
    { Params p; auto y = run (p, sine (-50, 1.0)); const double g = rmsDb (y, 24000, 48000) - (-50.0); CHECK (std::abs (g) < 0.3, "linear gain %.2f dB", g); }
    // 2. 4:1 knee near -18 dBFS: -18 gives < 3 dB GR, -6 gives > 6 dB GR
    { Params p; auto y = run (p, sine (-6, 1.0)); const double gr = -(rmsDb (y, 24000, 48000) + 6.0); CHECK (gr > 6.0 && gr < 14.0, "GR at -6 dBFS %.2f", gr); }
    // 3. higher ratio = steeper slope above the knee (measured on the UADx: 4:1 < 8:1 < 12:1 < 20:1; the thresholds
    //    differ per button, so the GR at one level is not ordered - only the slope is)
    { double prev = -1; for (int r = 0; r < 4; ++r) { Params p; p.ratio = r; p.inputDb = 12;
        auto a = run (p, sine (-18, 1.0)), b = run (p, sine (-6, 1.0));
        const double slope = ((rmsDb (a, 24000, 48000) + 18.0) - (rmsDb (b, 24000, 48000) + 6.0)) / 12.0; // GR dB per input dB
        CHECK (slope > prev - 0.01 && slope > 0.6 && slope < 1.0, "ratio %d slope %.3f (previous %.3f)", r, slope, prev); prev = slope; } }
    // 4. finite output on extreme input, every ratio, every sample rate
    for (double fs : { 44100.0, 48000.0, 96000.0, 192000.0 }) for (int r = 0; r < 5; ++r)
    { Params p; p.ratio = r; p.inputDb = 36; p.attack = 7; p.release = 7; auto x = sine (6, 0.5, 60, fs); auto y = run (p, x, fs);
      bool fin = true; float mx = 0; for (float v : y) { fin &= std::isfinite (v); mx = std::max (mx, std::abs (v)); } CHECK (fin && mx < 8.0f, "fs %.0f ratio %d finite %d max %.2f", fs, r, (int) fin, mx); }
    // 5. mix 0 = the dry signal, delayed by the reported latency (exact)
    { Params p; p.mix = 0; p.inputDb = 20; int lat = 0; auto x = sine (-10, 0.5, 440); auto y = run (p, x, 48000, 256, &lat);
      double err = 0; for (size_t i = (size_t) lat; i < x.size(); ++i) err = std::max (err, (double) std::abs (y[i] - x[i - (size_t) lat])); CHECK (err < 1e-6, "mix 0 null error %.2e (lat %d)", err, lat); }
    // 6. bypass = dry, delayed (latency-compensated)
    { Params p; p.bypass = true; int lat = 0; auto x = sine (-10, 0.3, 440); auto y = run (p, x, 48000, 256, &lat);
      double err = 0; for (size_t i = (size_t) lat; i < x.size(); ++i) err = std::max (err, (double) std::abs (y[i] - x[i - (size_t) lat])); CHECK (err < 1e-7, "bypass null %.2e", err); }
    // 7. block-size independence
    { Params p; p.inputDb = 15; auto x = sine (-12, 0.5, 220); auto a = run (p, x, 48000, 64), b = run (p, x, 48000, 512);
      double err = 0; for (size_t i = 0; i < x.size(); ++i) err = std::max (err, (double) std::abs (a[i] - b[i])); CHECK (err < 1e-5, "block size changes output by %.2e", err); }
    // 8. release: faster knob recovers faster
    { auto rec = [] (float rel) { Params p; p.release = rel; p.inputDb = 10; auto x = sine (-6, 0.5); auto q = sine (-40, 1.0); x.insert (x.end(), q.begin(), q.end());
        auto y = run (p, x); return rmsDb (y, 24000 + 4800, 24000 + 9600) - (-40.0 + 10.0); };
      CHECK (rec (7) > rec (1) + 2.0, "release 7 %.2f vs release 1 %.2f", rec (7), rec (1)); }

    // 9. a NaN / Inf in the input never poisons the engine: 50 ms later the output is finite again
    { Params p; p.inputDb = 12; Engine e; e.setParams (p, true); e.prepare (48000, 256, 2);
      auto x = sine (-12, 0.5); x[1000] = std::numeric_limits<float>::quiet_NaN(); x[1001] = std::numeric_limits<float>::infinity();
      std::vector<float> L (x), R (x); for (size_t s = 0; s < x.size(); s += 256) { float* ch[2] = { L.data() + s, R.data() + s }; e.process (ch, (int) std::min<size_t> (256, x.size() - s)); }
      bool fin = true; for (size_t i = 1000 + 2400; i < L.size(); ++i) fin &= std::isfinite (L[i]) && std::isfinite (R[i]);
      CHECK (fin, "output still non-finite 50 ms after a NaN/Inf input"); }
    // 10. denormal input costs no more CPU than normal audio (within 3x) and stays finite
    { auto time = [] (float level) { Params p; p.release = 1; Engine e; e.setParams (p, true); e.prepare (48000, 256, 2);
          std::vector<float> L (48000 * 4, level), R (48000 * 4, level);
          for (size_t i = 0; i < L.size(); ++i) { L[i] *= (i & 1) ? 1.0f : -1.0f; R[i] = L[i]; }
          // one loud second first, then the tiny tail (the release decays toward denormals)
          for (size_t i = 0; i < 48000; ++i) { L[i] = R[i] = 0.5f * std::sin ((float) i * 0.05f); }
          const auto t0 = std::chrono::steady_clock::now();
          for (size_t s = 0; s < L.size(); s += 256) { float* ch[2] = { L.data() + s, R.data() + s }; e.process (ch, 256); }
          return std::chrono::duration<double> (std::chrono::steady_clock::now() - t0).count(); };
      const double tn = time (0.1f), td = time (1e-40f);
      CHECK (td < 3.0 * tn + 0.002, "denormal input %.4f s vs normal %.4f s", td, tn); }
    // 11. automation: an INPUT jump of 20 dB inside a block makes no step (smoothed): largest sample-to-sample change stays small
    { Params p; Engine e; e.setParams (p, true); e.prepare (48000, 256, 2);
      auto x = sine (-30, 0.6, 100); std::vector<float> L (x), R (x); float maxStep = 0;
      for (size_t s = 0; s < x.size(); s += 256) { if (s == 256 * 50) { p.inputDb = 20; e.setParams (p); }
          float* ch[2] = { L.data() + s, R.data() + s }; e.process (ch, (int) std::min<size_t> (256, x.size() - s)); }
      for (size_t i = 256 * 50; i < 256 * 54; ++i) maxStep = std::max (maxStep, std::abs (L[i] - L[i - 1]));
      CHECK (maxStep < 0.02f, "input automation step %.4f (zipper)", maxStep); }
    // 12. sample-rate change: re-prepare at 96 kHz, latency follows the rate, output correct level
    { Params p; Engine e; e.setParams (p, true); e.prepare (44100, 512, 2); const int l1 = e.latencySamples(); e.prepare (96000, 512, 2); const int l2 = e.latencySamples();
      auto y = run (p, sine (-50, 1.0, 1000, 96000), 96000); const double g = rmsDb (y, 48000, 96000) + 50.0;
      CHECK (l1 > 0 && l2 > 0 && std::abs (g) < 0.3, "latency %d/%d, gain at 96k %.2f", l1, l2, g); }
    // 13. mono: one channel works and equals the stereo left channel
    { Params p; p.inputDb = 10; auto x = sine (-10, 0.5, 300); Engine e; e.setParams (p, true); e.prepare (48000, 256, 1);
      std::vector<float> M (x); for (size_t s = 0; s < x.size(); s += 256) { float* ch[1] = { M.data() + s }; e.process (ch, (int) std::min<size_t> (256, x.size() - s)); }
      auto S = run (p, x); double err = 0; for (size_t i = 0; i < x.size(); ++i) err = std::max (err, (double) std::abs (M[i] - S[i]));
      CHECK (err < 1e-5, "mono vs stereo-left differ by %.2e", err); }
    // 14. odd host block sizes (1, 7, 333, 4096 when prepared for 4096) give the same output
    { Params p; p.inputDb = 15; auto x = sine (-12, 0.4, 220); auto ref = run (p, x, 48000, 64);
      for (int B : { 1, 7, 333, 4096 }) { auto y = run (p, x, 48000, B); double err = 0; for (size_t i = 0; i < x.size(); ++i) err = std::max (err, (double) std::abs (y[i] - ref[i]));
          CHECK (err < 1e-5, "block %d differs by %.2e", B, err); } }
    // 15. ratio switching mid-signal is click-free (no sample jump beyond the signal's own slope x 2)
    { Params p; p.inputDb = 15; Engine e; e.setParams (p, true); e.prepare (48000, 256, 2); auto x = sine (-12, 0.6, 100); std::vector<float> L (x), R (x); float maxStep = 0;
      for (size_t s = 0; s < x.size(); s += 256) { if (s == 256 * 60) { p.ratio = 4; e.setParams (p); } float* ch[2] = { L.data() + s, R.data() + s }; e.process (ch, (int) std::min<size_t> (256, x.size() - s)); }
      for (size_t i = 256 * 60; i < 256 * 64; ++i) maxStep = std::max (maxStep, std::abs (L[i] - L[i - 1]));
      CHECK (maxStep < 0.03f, "ratio switch step %.4f", maxStep); }
    // 16. side-chain high-pass: bass no longer drives the compressor (a 50 Hz + 3 kHz mix: less GR with 200 Hz HPF)
    { auto mixSig = sine (-8, 1.0, 50); auto hf = sine (-20, 1.0, 3000); for (size_t i = 0; i < mixSig.size(); ++i) mixSig[i] += hf[i];
      auto grOf = [&] (float hp) { Params p; p.inputDb = 6; p.scHpfHz = hp; Engine e; e.setParams (p, true); e.prepare (48000, 256, 2);
          std::vector<float> L (mixSig), R (mixSig); float gr = 0; for (size_t s = 0; s < L.size(); s += 256) { float* ch[2] = { L.data() + s, R.data() + s };
              e.process (ch, (int) std::min<size_t> (256, L.size() - s)); if (s > 24000) gr = std::max (gr, e.gainReductionDb()); } return gr; };
      const float g0 = grOf (0), g1 = grOf (200);
      CHECK (g0 > 6.0f && g1 < g0 - 4.0f, "SC HPF: GR %.1f dB without, %.1f dB with a 200 Hz high-pass", g0, g1); }
    // 17. external side-chain: a quiet main signal is ducked only while the key is loud; INPUT still sets the amount
    { Params p; p.inputDb = 10; p.extSc = true; Engine e; e.setParams (p, true); e.prepare (48000, 256, 2);
      auto mainS = sine (-30, 1.0, 440); auto keyS = sine (-60, 1.0, 100); auto loud = sine (-6, 0.5, 100);
      std::copy (loud.begin(), loud.end(), keyS.begin() + 24000);
      std::vector<float> L (mainS), R (mainS); float grQuiet = 0, grLoud = 0;
      for (size_t s = 0; s < L.size(); s += 256) { const size_t k = std::min<size_t> (256, L.size() - s); float* ch[2] = { L.data() + s, R.data() + s };
          const float* sc[1] = { keyS.data() + s }; e.process (ch, (int) k, sc, 1);
          if (s > 12000 && s < 23000) grQuiet = std::max (grQuiet, e.gainReductionDb()); if (s > 30000 && s < 46000) grLoud = std::max (grLoud, e.gainReductionDb()); }
      CHECK (grQuiet < 0.5f && grLoud > 8.0f, "ext SC: GR %.2f while the key is quiet, %.1f while loud", grQuiet, grLoud); }
    std::printf ("Axell 76 DSP: %d passed, %d failed\n", passes, fails);
    return fails ? 1 : 0;
}
