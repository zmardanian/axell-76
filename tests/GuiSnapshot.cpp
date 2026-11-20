#include <cstdlib>
// Axell 76 - headless GUI snapshot: the real processor + editor, a loud tone through it so the meter moves; saves a PNG.
#include "../Source/PluginEditor.h"
#include "../Source/PluginProcessor.h"
int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File out (argc > 1 ? argv[1] : "a76.png");
    // textures at the snapshot pixel density; argv[4] sets it (0 = the plugin default, 2)
    {
        const float sc = argc > 2 ? (float) std::atof (argv[2]) : 1.6f;
        const float t = argc > 4 ? (float) std::atof (argv[4]) : 0.0f;
        vintage::textureScale = t > 0.0f ? t : (argc > 4 ? 2.0f : std::max (2.0f, sc));
    }
    A76Processor proc;
    proc.setPlayConfigDetails (2, 2, 48000.0, 512);
    proc.prepareToPlay (48000.0, 512);
    if (argc > 3 && argv[3][0] == '1') proc.apvts.state.setProperty ("skin", 1, nullptr);
    if (std::getenv ("AXELL_VIDEO_DIR")) if (auto* p = proc.apvts.getParameter ("meter")) p->setValueNotifyingHost (p->convertTo0to1 (1.0f));   // filming: VU +4, the needle follows the output
    if (auto* p = proc.apvts.getParameter ("input")) p->setValueNotifyingHost (p->convertTo0to1 (3.0f));
    std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
    ed->setSize (vintage::kTotalW, vintage::kTotalH);
    juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m;
    const bool axVideo = std::getenv ("AXELL_VIDEO_DIR") != nullptr;
    // filming on a beat: AXELL_VIDEO_BEAT = interleaved stereo float32 at 48 kHz; AXELL_VIDEO_IN / _OUT set the gain knobs (dB)
    std::vector<float> axBeat;
    if (const char* bp = std::getenv ("AXELL_VIDEO_BEAT")) { juce::FileInputStream in ((juce::File (bp))); if (in.openedOk()) { axBeat.resize ((size_t) in.getTotalLength() / 4); in.read (axBeat.data(), (int) in.getTotalLength()); } }
    for (const char* id : { "input", "output" }) if (const char* g = std::getenv (juce::String (id) == "input" ? "AXELL_VIDEO_IN" : "AXELL_VIDEO_OUT")) if (auto* p = proc.apvts.getParameter (id)) p->setValueNotifyingHost (p->convertTo0to1 ((float) std::atof (g)));
    // AXELL_VIDEO_SET="attack=2,release=1.5,ratio=0": any parameter by ID, in its own units (a choice by index)
    if (const char* s = std::getenv ("AXELL_VIDEO_SET")) for (auto& kv : juce::StringArray::fromTokens (s, ",", "")) if (auto* p = proc.apvts.getParameter (kv.upToFirstOccurrenceOf ("=", false, false).trim())) p->setValueNotifyingHost (p->convertTo0to1 (kv.fromFirstOccurrenceOf ("=", false, false).getFloatValue()));
    float axPeakGr = 0.0f, axPeakVu = -100.0f;   // filming: a longer, pulsing signal so the needle moves
    for (int k = 0; k < (axVideo ? (axBeat.empty() ? 470 : (int) (axBeat.size() / 2 / 512)) : 200); ++k)
    {
        for (int i = 0; i < 512; ++i) { const float tt = (float) (k * 512 + i) / 48000.0f; float v = 0.3f * std::sin (2.0f * 3.14159265f * 440.0f * tt); if (axBeat.size() >= (size_t) (k * 512 + i + 1) * 2) { b.setSample (0, i, axBeat[(size_t) (k * 512 + i) * 2]); b.setSample (1, i, axBeat[(size_t) (k * 512 + i) * 2 + 1]); continue; } b.setSample (0, i, v); b.setSample (1, i, v); }
        proc.processBlock (b, m);
        if (axVideo) { axPeakGr = std::max (axPeakGr, proc.grMeter.load()); float ms = 0; for (int i = 0; i < 512; ++i) ms += b.getSample (0, i) * b.getSample (0, i); axPeakVu = std::max (axPeakVu, 10.0f * std::log10 (ms / 512.0f + 1e-12f) + 18.0f); if (k % 60 == 59) std::fprintf (stderr, "peak GR %.1f dB, peak VU(+4) %.1f\n", axPeakGr, axPeakVu); }
        // the editor ticks at 40 Hz in a host: filming keeps that pace in audio time (one tick per 1200 samples), so the
        // needle moves on screen exactly as fast as in a DAW
        if (axVideo) for (int t = (k * 512) / 1200; t < ((k + 1) * 512) / 1200; ++t) dynamic_cast<A76Editor*> (ed.get())->timerCallback();
{ /* AXELL_VIDEO_DIR set: also save a frame every 3 blocks (~31 fps) for the website loops; first take only */ static void* axOwner = nullptr; static int axBlk = 0, axFr = 0; if (const char* axDir = std::getenv ("AXELL_VIDEO_DIR")) { if (axOwner == nullptr) axOwner = (void*) (ed.get()); if (axOwner != (void*) (ed.get()) || axFr >= 300) std::exit (0); if (++axBlk % 3 == 0) { juce::MessageManager::getInstance()->runDispatchLoopUntil (2); auto axIm = (ed.get())->createComponentSnapshot ((ed.get())->getLocalBounds(), true, 1.0f); juce::File axF = juce::File (axDir).getChildFile (juce::String::formatted ("f%05d.png", axFr++)); axF.deleteFile(); { juce::FileOutputStream axOs (axF); juce::PNGImageFormat().writeImageToStream (axIm, axOs); } } } }
    }
    for (int k = 0; k < 80; ++k) dynamic_cast<A76Editor*> (ed.get())->timerCallback();
    auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, argc > 2 ? (float) std::atof (argv[2]) : 1.6f);
    juce::FileOutputStream os (out); os.setPosition (0); os.truncate();
    juce::PNGImageFormat().writeImageToStream (img, os);
    std::printf ("saved %s\n", out.getFullPathName().toRawUTF8());
    return 0;
}
