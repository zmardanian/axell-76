// Axell 76 plugin tests (JUCE): state round trip (parameters + skin), identical render after reload, bypass null.
#include "../Source/PluginProcessor.h"
#include <cstdio>
#include <functional>
static int fails = 0, passes = 0;
#define CHECK(c, ...) do { if (c) ++passes; else { ++fails; std::printf ("FAIL %s:%d  ", __FILE__, __LINE__); std::printf (__VA_ARGS__); std::printf ("\n"); } } while (0)
static void set (A76Processor& p, const char* id, float v) { auto* q = p.apvts.getParameter (id); q->setValueNotifyingHost (q->convertTo0to1 (v)); }
static juce::AudioBuffer<float> render (A76Processor& p, int seconds = 2)
{
    p.setPlayConfigDetails (2, 2, 48000.0, 512); p.prepareToPlay (48000.0, 512);
    juce::AudioBuffer<float> out (2, 48000 * seconds), b (2, 512); juce::MidiBuffer m;
    for (int s = 0; s < out.getNumSamples(); s += 512)
    {
        for (int i = 0; i < 512; ++i) { const float v = 0.3f * std::sin (0.031f * (float) (s + i)) * (((s + i) / 6000) % 2 ? 1.0f : 0.2f); b.setSample (0, i, v); b.setSample (1, i, 0.7f * v); }
        p.processBlock (b, m);
        for (int c = 0; c < 2; ++c) out.copyFrom (c, s, b, c, 0, std::min (512, out.getNumSamples() - s));   // the last block is partial
    }
    return out;
}
// audit mode: render through the real processor, as a session saved by 2.0 (no model mark) or a new one
// usage: A76PluginTests --render in.f32 out.f32 <2|3> id=normalised ...   (stereo, 48 kHz, latency removed)
static int renderMode (int argc, char** argv)
{
    A76Processor pr;
    if (std::atoi (argv[4]) < 4)                         // 2: a 2.0 session (no model mark); 3: a 3.0/3.1 session (mark 3)
    {
        juce::MemoryBlock s; pr.getStateInformation (s); auto xml = juce::AudioProcessor::getXmlFromBinary (s.getData(), (int) s.getSize());
        if (std::atoi (argv[4]) == 2) { xml->removeAttribute ("model"); xml->setAttribute ("stateVersion", 2); }
        else xml->setAttribute ("model", 3);
        juce::MemoryBlock old; juce::AudioProcessor::copyXmlToBinary (*xml, old);
        pr.setStateInformation (old.getData(), (int) old.getSize());
    }
    for (int i = 5; i < argc; ++i)
    {
        const juce::String kv (argv[i]); const auto id = kv.upToFirstOccurrenceOf ("=", false, false);
        if (auto* q = pr.apvts.getParameter (id)) q->setValueNotifyingHost (kv.fromFirstOccurrenceOf ("=", false, false).getFloatValue());
    }
    juce::File fi (argv[2]); juce::MemoryBlock in; fi.loadFileAsData (in);
    const int n = (int) (in.getSize() / (2 * sizeof (float))); const float* xi = (const float*) in.getData();
    pr.setPlayConfigDetails (2, 2, 48000.0, 512); pr.prepareToPlay (48000.0, 512);
    const int lat = pr.getLatencySamples(); std::vector<float> out ((size_t) n * 2, 0.0f);
    juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m;
    for (int s0 = 0; s0 < n + lat; s0 += 512)
    {
        for (int i = 0; i < 512; ++i) for (int c = 0; c < 2; ++c) { const int k = s0 + i; b.setSample (c, i, k < n ? xi[k * 2 + c] : 0.0f); }
        pr.processBlock (b, m);
        for (int i = 0; i < 512; ++i) { const int k = s0 + i - lat; if (k >= 0 && k < n) for (int c = 0; c < 2; ++c) out[(size_t) k * 2 + (size_t) c] = b.getSample (c, i); }
    }
    juce::File (argv[3]).replaceWithData (out.data(), out.size() * sizeof (float));
    std::printf ("rendered %d samples, model %d, latency %d\n", n, pr.modelGen.load(), lat);
    return 0;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    if (argc >= 5 && juce::String (argv[1]) == "--render") return renderMode (argc, argv);
    A76Processor a;
    set (a, "input", 14.0f); set (a, "output", -3.5f); set (a, "attack", 6.2f); set (a, "release", 2.4f); set (a, "ratio", 4); set (a, "mix", 72.0f);
    a.apvts.state.setProperty ("skin", 1, nullptr);
    juce::MemoryBlock st; a.getStateInformation (st);
    A76Processor b; b.setStateInformation (st.getData(), (int) st.getSize());
    for (auto* id : { "input", "output", "attack", "release", "ratio", "mix", "bypass" })
        CHECK (std::abs (a.apvts.getRawParameterValue (id)->load() - b.apvts.getRawParameterValue (id)->load()) < 1e-6f, "param %s not restored", id);
    CHECK ((int) b.apvts.state.getProperty ("skin", 0) == 1, "skin not restored");
    auto ra = render (a), rb = render (b);
    float d = 0; for (int c = 0; c < 2; ++c) for (int i = 0; i < ra.getNumSamples(); ++i) d = std::max (d, std::abs (ra.getSample (c, i) - rb.getSample (c, i)));
    CHECK (d == 0.0f, "reloaded instance renders differently (max diff %g)", d);
    // bypass = the input delayed by the reported latency, exactly
    A76Processor c; set (c, "bypass", 1.0f); set (c, "input", 20.0f);
    auto rc = render (c); const int lat = c.getLatencySamples(); float e = 0;
    for (int i = lat + 1; i < rc.getNumSamples(); ++i) { const int s = i - lat; const float v = 0.3f * std::sin (0.031f * (float) s) * ((s / 6000) % 2 ? 1.0f : 0.2f); e = std::max (e, std::abs (rc.getSample (0, i) - v)); }
    CHECK (e < 1e-6f && lat > 0, "bypass null %g (latency %d)", e, lat);
    // a garbage state does not crash and leaves defaults
    A76Processor d2; const char junk[] = "not a state at all"; d2.setStateInformation (junk, sizeof (junk));
    CHECK (std::abs (d2.apvts.getRawParameterValue ("input")->load()) < 1e-6f, "junk state changed parameters");
    // QUALITY: High by default; switching mid-stream changes the reported latency and stays finite
    { A76Processor q; CHECK (juce::roundToInt (q.apvts.getRawParameterValue ("quality")->load()) == 1, "default quality not High");
      auto r1 = render (q, 1); const int l4 = q.getLatencySamples(); set (q, "quality", 2.0f);
      juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m; b.clear(); for (int i = 0; i < 512; ++i) b.setSample (0, i, 0.5f * std::sin (0.1f * (float) i));
      q.processBlock (b, m); const int l8 = q.getLatencySamples(); bool fin = true; for (int i = 0; i < 512; ++i) fin &= std::isfinite (b.getSample (0, i));
      CHECK (l8 > l4 && fin, "quality switch: latency %d -> %d, finite %d", l4, l8, (int) fin); }
    // a state from before 2.0 (no quality) loads as Standard (2x) - it sounds as it did
    { A76Processor o; juce::MemoryBlock s2; o.getStateInformation (s2); auto xml = juce::AudioProcessor::getXmlFromBinary (s2.getData(), (int) s2.getSize());
      for (auto* e : xml->getChildIterator()) if (e->getStringAttribute ("id") == "quality") { xml->removeChildElement (e, true); break; }
      juce::MemoryBlock old; juce::AudioProcessor::copyXmlToBinary (*xml, old);
      A76Processor n; n.setStateInformation (old.getData(), (int) old.getSize());
      CHECK (juce::roundToInt (n.apvts.getRawParameterValue ("quality")->load()) == 0, "old state did not load as Standard"); }
    // 3.0: new sessions use the 3.0 model and keep it; a session from before 3.0 (no model mark) keeps the 2.0 model
    { A76Processor fresh; CHECK (fresh.modelGen.load() == 4, "a new instance is not on the 3.2 model");
      juce::MemoryBlock s3; fresh.getStateInformation (s3); A76Processor r3; r3.setStateInformation (s3.getData(), (int) s3.getSize());
      CHECK (r3.modelGen.load() == 4, "3.2 model lost on reload");
      auto xml = juce::AudioProcessor::getXmlFromBinary (s3.getData(), (int) s3.getSize()); xml->removeAttribute ("model"); xml->setAttribute ("stateVersion", 2);
      juce::MemoryBlock old; juce::AudioProcessor::copyXmlToBinary (*xml, old);
      A76Processor v2; v2.setStateInformation (old.getData(), (int) old.getSize());
      CHECK (v2.modelGen.load() == 2, "a 2.0 session did not keep the 2.0 model");
      juce::MemoryBlock again; v2.getStateInformation (again); A76Processor v2b; v2b.setStateInformation (again.getData(), (int) again.getSize());
      CHECK (v2b.modelGen.load() == 2, "a 2.0 session lost its model when saved again");
      // the two models really differ (loud, compressing): otherwise the mark would protect nothing
      auto render = [] (A76Processor& pr) {
          pr.setPlayConfigDetails (2, 2, 48000.0, 512); pr.prepareToPlay (48000.0, 512);
          if (auto* q = pr.apvts.getParameter ("input")) q->setValueNotifyingHost (q->convertTo0to1 (20.0f));
          juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m; double e = 0.0; std::vector<float> out;
          for (int k = 0; k < 60; ++k) { for (int i = 0; i < 512; ++i) { const float v = 0.4f * std::sin (0.04f * (float) (k * 512 + i)); b.setSample (0, i, v); b.setSample (1, i, v); }
                                         pr.processBlock (b, m); for (int i = 0; i < 512; ++i) out.push_back (b.getSample (0, i)); }
          (void) e; return out; };
      A76Processor m3; A76Processor m2; m2.setStateInformation (old.getData(), (int) old.getSize());
      const auto y3 = render (m3), y2 = render (m2); double d = 0.0; for (size_t i = 0; i < y3.size(); ++i) d = std::max (d, (double) std::abs (y3[i] - y2[i]));
      CHECK (d > 1.0e-5, "the 2.0 and 3.0 models render the same (max diff %g)", d); }
    // 3.1: every factory preset loads exactly its table values; pocket presets switch the external side-chain on;
    // the 2.0 presets keep their numbers (sessions and hosts recall programs by index)
    { A76Processor pp; int bad = 0, pockets = 0;
      for (int i = 0; i < axell76::kNumPresets; ++i)
      {
          pp.setCurrentProgram (i); const auto& pr = axell76::kPresets[i];
          auto near = [&] (const char* id, float v) { return std::abs (pp.apvts.getRawParameterValue (id)->load() - v) < 0.051f; };
          if (! (near ("input", pr.input) && near ("output", pr.output) && near ("attack", pr.attack) && near ("release", pr.release)
                 && juce::roundToInt (pp.apvts.getRawParameterValue ("ratio")->load()) == pr.ratio && near ("mix", pr.mix)
                 && (pp.apvts.getRawParameterValue ("extSc")->load() > 0.5f) == pr.extSc)) { ++bad; std::printf ("  preset %d (%s) did not load\n", i, pr.name); }
          pockets += pr.extSc ? 1 : 0;
      }
      CHECK (bad == 0, "%d presets did not load exactly", bad);
      CHECK (axell76::kNumPresets >= 52 && juce::String (axell76::kPresets[1].name) == "Rap Lead - Upfront" && juce::String (axell76::kPresets[11].name) == "Parallel - Crush",
             "the 2.0 presets moved (%d presets)", axell76::kNumPresets);
      CHECK (pockets >= 6, "only %d pocket presets use the side-chain", pockets); }
    // the text a host shows carries the number (the portable build once lost every decimal: "+ dB")
    { A76Processor tp; auto txt = [&] (const char* id, float v) { auto* q = tp.apvts.getParameter (id); return q->getText (q->convertTo0to1 (v), 64); };
      CHECK (txt ("input", 12.0f) == "+12.0 dB" && txt ("output", -3.5f) == "-3.5 dB" && txt ("attack", 4.0f).startsWith ("4.0"),
             "parameter text: '%s' '%s' '%s'", txt ("input", 12.0f).toRawUTF8(), txt ("output", -3.5f).toRawUTF8(), txt ("attack", 4.0f).toRawUTF8()); }
    // 3.2: a 3.0/3.1 session (model mark 3) keeps the 3.1 model and keeps it when saved again
    { A76Processor f; juce::MemoryBlock s; f.getStateInformation (s); auto xml = juce::AudioProcessor::getXmlFromBinary (s.getData(), (int) s.getSize());
      xml->setAttribute ("model", 3); juce::MemoryBlock m3; juce::AudioProcessor::copyXmlToBinary (*xml, m3);
      A76Processor v3; v3.setStateInformation (m3.getData(), (int) m3.getSize()); CHECK (v3.modelGen.load() == 3, "a 3.x session did not keep the 3.1 model");
      juce::MemoryBlock again; v3.getStateInformation (again); A76Processor v3b; v3b.setStateInformation (again.getData(), (int) again.getSize());
      CHECK (v3b.modelGen.load() == 3, "a 3.x session lost its model when saved again"); }
    // 3.2.2: the VU (+4 / +8) is a real VU movement - a sine reads its RMS, a step takes 300 ms to 99 %, and a drum-like
    // pulse train (50 ms hits, 0.5 s apart) moves it a few dB, not from the left stop to 0 on every hit
    { A76Processor vp; set (vp, "bypass", 1.0f);   // bypassed: the output is the input, the meter reads the signal itself
      vp.setPlayConfigDetails (2, 2, 48000.0, 512); vp.prepareToPlay (48000.0, 512);
      juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m;
      auto vuDb = [&] { return 10.0f * std::log10 (vp.outMs.load() + 1.0e-12f); };
      auto run = [&] (int blocks, std::function<float (int)> sig, std::vector<float>* trace)
      { static int s = 0; for (int k = 0; k < blocks; ++k) { for (int i = 0; i < 512; ++i, ++s) { const float v = sig (s); b.setSample (0, i, v); b.setSample (1, i, v); }
          vp.processBlock (b, m); if (trace) trace->push_back (vp.outMs.load()); } };
      const float a = 0.1778f;   // -15 dBFS peak = -18 dBFS RMS: 0 VU at +4
      auto sine = [a] (int s) { return a * std::sin (2.0f * 3.14159265f * 1000.0f * (float) s / 48000.0f); };
      run (94, [] (int) { return 0.0f; }, nullptr);                       // 1 s of silence
      std::vector<float> tr; run (188, sine, &tr);                        // then 2 s of the sine
      const float fin = 10.0f * std::log10 (tr.back());
      CHECK (std::abs (fin + 18.0f) < 0.2f, "VU calibration: a -18 dBFS RMS sine reads %.2f dB", fin);
      int k99 = 0; while (k99 < (int) tr.size() && std::sqrt (tr[(size_t) k99] / tr.back()) < 0.99f) ++k99;
      const float t99 = (float) (k99 + 1) * 512.0f / 48.0f - 10.0f;       // ms (the bypass delay of the latency aside)
      CHECK (t99 > 260.0f && t99 < 345.0f, "VU rise time to 99 %%: %.0f ms (300 +- 10 %%; plus up to one latency)", t99);
      // music-like: a sustained bed 10 dB under 60 ms hits, twice a second; the old meter was the RMS of each block
      auto beat = [a] (int s) { const float x = std::sin (0.13f * (float) s); return (s % 24000) < 2880 ? a * x : 0.316f * a * x; };
      run (188, beat, nullptr);
      float hi = -200.0f, lo = 200.0f, bhi = -200.0f, blo = 200.0f;
      for (int k = 0; k < 188; ++k)
      { run (1, beat, nullptr); hi = std::max (hi, vuDb()); lo = std::min (lo, vuDb());
        const float br = 20.0f * std::log10 (b.getRMSLevel (0, 0, 512) + 1.0e-6f); bhi = std::max (bhi, br); blo = std::min (blo, br); }
      CHECK (hi - lo < 0.6f * (bhi - blo) && hi - lo > 1.0f, "VU swing on a beat %.1f dB vs the block RMS %.1f dB (a VU integrates: well under it)", hi - lo, bhi - blo); }
    std::printf ("Axell 76 plugin: %d passed, %d failed\n", passes, fails);
    return fails ? 1 : 0;
}
