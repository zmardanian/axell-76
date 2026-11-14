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
    if (auto* p = proc.apvts.getParameter ("input")) p->setValueNotifyingHost (p->convertTo0to1 (3.0f));
    juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m;
    for (int k = 0; k < 200; ++k)
    {
        for (int i = 0; i < 512; ++i) { const float v = 0.3f * std::sin (2.0f * 3.14159265f * 440.0f * (float) (k * 512 + i) / 48000.0f); b.setSample (0, i, v); b.setSample (1, i, v); }
        proc.processBlock (b, m);
    }
    std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
    ed->setSize (vintage::kTotalW, vintage::kTotalH);
    for (int k = 0; k < 80; ++k) dynamic_cast<A76Editor*> (ed.get())->timerCallback();
    auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, argc > 2 ? (float) std::atof (argv[2]) : 1.6f);
    juce::FileOutputStream os (out); os.setPosition (0); os.truncate();
    juce::PNGImageFormat().writeImageToStream (img, os);
    std::printf ("saved %s\n", out.getFullPathName().toRawUTF8());
    return 0;
}
