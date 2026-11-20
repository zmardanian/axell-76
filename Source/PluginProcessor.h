#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Engine.h"
#include "Presets.h"

class A76Processor : public juce::AudioProcessor
{
public:
    A76Processor();
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void reset() override { engine.reset(); }
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return axell76::kNumPresets; }
    int getCurrentProgram() override { return currentProgram.load(); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override
    { return juce::isPositiveAndBelow (index, axell76::kNumPresets) ? axell76::kPresets[index].name : ""; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> grMeter { 0.0f }, inPeak { -100.0f }, outPeak { -100.0f }, outMs { 0.0f };   // outMs: the VU reading, as a mean square
    std::atomic<bool> scConnected { false };
    std::atomic<int> currentProgram { 0 };
    std::atomic<int> modelGen { 4 };       // 4 = 3.2 model; 3 = a 3.0/3.1 session, 2 = a 2.0 session: each keeps its sound, bit-exactly
    int modelApplied = 0;
    // 3.2.2: the VU movement (+4 / +8) - see processBlock
    float vuY = 0.0f, vuV = 0.0f, vuDt = 1.0f / 48000.0f;

private:
    axell76::Params readParams() const;
    axell76::Engine engine;
    std::atomic<float>* pIn = nullptr; std::atomic<float>* pOut = nullptr; std::atomic<float>* pAtk = nullptr;
    std::atomic<float>* pRel = nullptr; std::atomic<float>* pRatio = nullptr; std::atomic<float>* pMix = nullptr;
    std::atomic<float>* pBypass = nullptr; std::atomic<float>* pQuality = nullptr;
    std::atomic<float>* pScHpf = nullptr; std::atomic<float>* pExtSc = nullptr; std::atomic<float>* pMeter = nullptr;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (A76Processor)
};
