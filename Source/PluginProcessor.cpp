#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;

namespace
{
juce::AudioParameterFloatAttributes fmt (std::function<juce::String (float)> f)
{
    return juce::AudioParameterFloatAttributes().withStringFromValueFunction ([f] (float v, int) { return f (v); })
        .withValueFromStringFunction ([] (const juce::String& t) { return t.getFloatValue(); });
}
// one decimal place with integer arithmetic only: juce::String (float, places) goes through std::ostream, which comes
// out empty in the portable build (static libstdc++ inside a host) - hosts and the panel showed "+ dB"
juce::String fixed1 (float v)
{
    const int t = juce::roundToInt (v * 10.0f), a = std::abs (t);
    return juce::String (t < 0 ? "-" : "") + juce::String (a / 10) + "." + juce::String (a % 10);
}
juce::String dbText (float v) { return (v > 0.05f ? "+" : "") + fixed1 (v) + " dB"; }
juce::String timeText (float ms) { return ms < 1.0f ? juce::String (juce::roundToInt (ms * 1000.0f)) + " us" : ms < 10.0f ? fixed1 (ms) + " ms" : juce::String (juce::roundToInt (ms)) + " ms"; }
} // namespace

APVTS::ParameterLayout A76Processor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    // IDs are frozen from 1.0 on; new parameters are appended
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "input", 1 }, "Input", NormalisableRange<float> (-24.0f, 36.0f, 0.1f), 0.0f, fmt (dbText)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "output", 1 }, "Output", NormalisableRange<float> (-36.0f, 24.0f, 0.1f), 0.0f, fmt (dbText)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "attack", 1 }, "Attack", NormalisableRange<float> (1.0f, 7.0f, 0.01f), 4.0f,
        fmt ([] (float v) { return fixed1 (v) + "  (" + timeText (axell76::attackMs (v)) + ")"; })));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "release", 1 }, "Release", NormalisableRange<float> (1.0f, 7.0f, 0.01f), 4.0f,
        fmt ([] (float v) { return fixed1 (v) + "  (" + timeText (axell76::releaseMs (v)) + ")"; })));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "ratio", 1 }, "Ratio", StringArray { "4", "8", "12", "20", "ALL" }, 0));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix", NormalisableRange<float> (0.0f, 100.0f, 1.0f), 100.0f,
        fmt ([] (float v) { return String (roundToInt (v)) + " %"; })));
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "bypass", 1 }, "Bypass", false));
    // 2.0 (appended): oversampling. New instances: High; projects saved before 2.0 load as Standard (their 1.x oversampling)
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "quality", 2 }, "Quality", StringArray { "Standard (2x)", "High (4x)", "Ultra (8x)" }, 1));
    NormalisableRange<float> hp (0.0f, 500.0f, 1.0f);
    hp.setSkewForCentre (100.0f);
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "scHpf", 2 }, "Side-Chain High-Pass", hp, 0.0f,
        fmt ([] (float v) { return v < 20.0f ? String ("Off") : String (roundToInt (v)) + " Hz"; })));
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "extSc", 2 }, "External Side-Chain", false));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "meter", 2 }, "Meter", StringArray { "GR", "+4", "+8" }, 0));
    return { p.begin(), p.end() };
}

A76Processor::A76Processor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                                       .withInput ("Sidechain", juce::AudioChannelSet::stereo(), false)),
      apvts (*this, nullptr, "AXELL76", createLayout())
{
    pIn = apvts.getRawParameterValue ("input"); pOut = apvts.getRawParameterValue ("output");
    pAtk = apvts.getRawParameterValue ("attack"); pRel = apvts.getRawParameterValue ("release");
    pRatio = apvts.getRawParameterValue ("ratio"); pMix = apvts.getRawParameterValue ("mix");
    pBypass = apvts.getRawParameterValue ("bypass");
    pQuality = apvts.getRawParameterValue ("quality");
    pScHpf = apvts.getRawParameterValue ("scHpf"); pExtSc = apvts.getRawParameterValue ("extSc"); pMeter = apvts.getRawParameterValue ("meter");
}

bool A76Processor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto o = l.getMainOutputChannelSet();
    if (! ((o == juce::AudioChannelSet::mono() || o == juce::AudioChannelSet::stereo()) && l.getMainInputChannelSet() == o)) return false;
    if (l.inputBuses.size() > 1)
    {
        const auto sc = l.getChannelSet (true, 1);
        if (! (sc.isDisabled() || sc == juce::AudioChannelSet::mono() || sc == juce::AudioChannelSet::stereo())) return false;
    }
    return true;
}

axell76::Params A76Processor::readParams() const
{
    axell76::Params p;
    p.inputDb = pIn->load(); p.outputDb = pOut->load(); p.attack = pAtk->load(); p.release = pRel->load();
    p.ratio = juce::roundToInt (pRatio->load()); p.mix = pMix->load(); p.bypass = pBypass->load() > 0.5f;
    p.scHpfHz = pScHpf->load() < 20.0f ? 0.0f : pScHpf->load(); p.extSc = pExtSc->load() > 0.5f; p.meter = juce::roundToInt (pMeter->load());
    return p;
}

void A76Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.setParams (readParams(), true);
    engine.setOversamplingLog2 (1 + juce::roundToInt (pQuality->load()));
    modelApplied = modelGen.load(); engine.setModelGeneration (modelApplied);
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    setLatencySamples (engine.latencySamples());
    vuY = vuV = 0.0f;
    vuDt = (float) (1.0 / sampleRate);
}

void A76Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    auto mainBus = getBusBuffer (buffer, true, 0);
    auto scBus = getBusCount (true) > 1 && getBus (true, 1)->isEnabled() ? getBusBuffer (buffer, true, 1) : juce::AudioBuffer<float>();
    const int nc = std::min (2, mainBus.getNumChannels()), n = buffer.getNumSamples();
    if (const int g = modelGen.load(); g != modelApplied) { engine.setModelGeneration (g); modelApplied = g; }   // a session was loaded
    engine.setParams (readParams());
    const int q = 1 + juce::roundToInt (pQuality->load());
    if (q != engine.oversamplingLog2())
    {
        engine.switchOversampling (q);
        setLatencySamples (engine.latencySamples());
    }
    float ip = 0.0f;
    for (int c = 0; c < nc; ++c) ip = std::max (ip, buffer.getMagnitude (c, 0, n));
    // the engine's block size is fixed at prepare; hosts may send more, so process in chunks
    const int maxN = std::max (1, getBlockSize());
    for (int s = 0; s < n; s += maxN)
    {
        const int k = std::min (maxN, n - s);
        float* ch[2] = { buffer.getWritePointer (0, s), nc > 1 ? buffer.getWritePointer (1, s) : nullptr };
        const float* sc[2] = { nullptr, nullptr };
        int scN = 0;
        if (scBus.getNumChannels() > 0)
        {
            scN = std::min (2, scBus.getNumChannels());
            for (int c = 0; c < scN; ++c) sc[c] = scBus.getReadPointer (c, s);
        }
        scConnected.store (scN > 0);
        engine.process (ch, k, scN > 0 ? sc : nullptr, scN);
    }
    float op = 0.0f;
    for (int c = 0; c < nc; ++c) op = std::max (op, buffer.getMagnitude (c, 0, n));
    grMeter.store (engine.gainReductionDb());
    // 3.2.2 the VU (+4 / +8) as a real VU movement (IEC 60268-17): the full-wave rectified output (louder channel) through
    // a second-order movement - damping 0.81, 2.14 Hz: 99 % of a step in 300 ms, 1.3 % overshoot - read as a sine's RMS
    // (x 1.1107). Before, the needle chased the RMS of the last block alone (~10 ms): on drums it slammed between -20 and
    // 0 on every hit; a VU integrates, so the gaps between hits barely move it
    {
        constexpr float w = 13.4222f, z = 0.81f;
        const float* l = buffer.getReadPointer (0);
        const float* r = nc > 1 ? buffer.getReadPointer (1) : l;
        float y = vuY, v = vuV;
        for (int i = 0; i < n; ++i)
        {
            const float x = std::max (std::abs (l[i]), std::abs (r[i]));
            v += (w * w * (x - y) - 2.0f * z * w * v) * vuDt;
            y += v * vuDt;
        }
        vuY = std::isfinite (y) ? y : 0.0f;
        vuV = std::isfinite (v) ? v : 0.0f;
        const float rms = 1.1107f * std::max (0.0f, vuY);
        outMs.store (rms * rms);
    }
    inPeak.store (juce::Decibels::gainToDecibels (ip, -100.0f)); outPeak.store (juce::Decibels::gainToDecibels (op, -100.0f));
}

void A76Processor::getStateInformation (juce::MemoryBlock& destData)
{
    auto st = apvts.copyState();
    st.setProperty ("stateVersion", 3, nullptr);
    st.setProperty ("model", modelGen.load(), nullptr);
    st.setProperty ("program", currentProgram.load(), nullptr);   // the factory preset last chosen, so hosts see the same Program after a reload
    if (auto xml = st.createXml()) copyXmlToBinary (*xml, destData);
}

void A76Processor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            // a state from before 2.0 has no QUALITY: it loads at the oversampling it was made with (2x)
            if (! tree.getChildWithProperty ("id", "quality").isValid())
            {
                juce::ValueTree c ("PARAM");
                c.setProperty ("id", "quality", nullptr);
                c.setProperty ("value", 0.0f, nullptr);
                tree.appendChild (c, nullptr);
            }
            // a state from before 3.0 has no model mark: it keeps the 2.0 model, so old sessions sound as they did
            modelGen.store (tree.hasProperty ("model") ? juce::jlimit (2, 4, (int) tree.getProperty ("model")) : 2);
            // only the index: the parameters come from the state itself, the preset is not applied again
            if (tree.hasProperty ("program"))
                currentProgram.store (juce::jlimit (0, juce::jmax (0, getNumPrograms() - 1), (int) tree.getProperty ("program")));
            tree.removeProperty ("program", nullptr);
            apvts.replaceState (tree);
            // replaceState only moves parameters whose stored value changed; a host may have left a switch at an
            // in-between value (0.37 reads as 'off' but is reported as 0.37), so every parameter gets its saved value
            for (auto* prm : getParameters())
                if (auto* r = dynamic_cast<juce::RangedAudioParameter*> (prm))
                {
                    const auto c = tree.getChildWithProperty ("id", r->paramID);
                    if (c.isValid() && c.hasProperty ("value")) r->setValueNotifyingHost (r->convertTo0to1 ((float) c.getProperty ("value")));
                }
        }
}

void A76Processor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, axell76::kNumPresets)) return;
    const auto& pr = axell76::kPresets[index];
    auto set = [this] (const char* id, float v) { if (auto* q = apvts.getParameter (id)) q->setValueNotifyingHost (q->convertTo0to1 (v)); };
    set ("input", pr.input); set ("output", pr.output); set ("attack", pr.attack); set ("release", pr.release);
    set ("ratio", (float) pr.ratio); set ("mix", pr.mix); set ("scHpf", pr.scHpf); set ("extSc", pr.extSc ? 1.0f : 0.0f);
    currentProgram.store (index);
    apvts.state.setProperty ("preset", index, nullptr);
}

juce::AudioProcessorEditor* A76Processor::createEditor() { return new A76Editor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new A76Processor(); }
