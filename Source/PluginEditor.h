#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Axell 76 - a 1970s hi-fi piece: a honey walnut cabinet, a champagne brushed-aluminium faceplate with screen-printed
// legends, an enamel nameplate, spun-aluminium knobs, ivory push-keys with amber lamps, an amber-lit VU meter under
// glass. Every texture is drawn procedurally.
namespace vintage
{
constexpr int kW = 1000, kH = 470;       // faceplate (2.0: a utility strip at the bottom)
constexpr int kSide = 74, kRail = 28;    // walnut cabinet around it
constexpr int kTotalW = kW + 2 * kSide, kTotalH = kH + 2 * kRail;
// texture resolution in pixels per panel unit to start with (2); each panel then follows the pixel density it is
// actually drawn at (HiDPI screens, an enlarged window: up to kMaxTextureScale). The snapshot tool raises the start.
inline float textureScale = 2.0f;
constexpr float kMaxTextureScale = 4.0f;

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool over, bool down) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawTooltip (juce::Graphics&, const juce::String& text, int w, int h) override;
};

class VuMeter : public juce::Component, public juce::SettableTooltipClient
{
public:
    void setGr (float db);
    void setValues (float grDb, float outDb, int meterMode) { mode = meterMode; outLevel = outDb; setGr (grDb); }
    std::function<void()> onClick;
    void mouseUp (const juce::MouseEvent&) override { if (onClick) onClick(); }
    int mode = 0;          // 0 GR, 1 output +4 (0 VU = -18 dBFS), 2 output +8 (0 VU = -14 dBFS)
    float outLevel = -100.0f;
    void paint (juce::Graphics&) override;
private:
    float shown = 1.0f, vel = 0.0f;
    float lamp = 1.0f, lampTarget = 1.0f;   // the incandescent backlight breathes a little
    juce::uint32 ticks = 0;
    juce::Image paper;                      // aged scale paper (fibres, foxing), made at the screen's pixel density
    float paperScale = 0.0f;
};

// panel skin: a miniature slide switch, SILVER <-> BLACK (saved with the project, not automatable)
class SkinSwitch : public juce::Component, public juce::SettableTooltipClient
{
public:
    std::function<void()> onFlip;
    bool dark = false;
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override { if (onFlip) onFlip(); }
};

class Face : public juce::Component
{
public:
    explicit Face (A76Processor&);
    ~Face() override;
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void tick();

private:
    struct KnobSpec { juce::Slider* s; const char* caption; std::vector<std::pair<float, juce::String>> scale; int minorPerMajor; bool big; };
    void addKnob (juce::Slider& s, const char* id, const char* tip, juce::Rectangle<int> bounds);
    void paintCabinet (juce::Graphics&);
    void paintPlate (juce::Graphics&);
    void paintKnobLegend (juce::Graphics&, const KnobSpec&);
    void buildTextures (float scale);
    float texScale = 2.0f; bool rebuildPending = false;

    A76Processor& proc;
    LookAndFeel lnf;
    juce::Component plate;   // every control, in faceplate coordinates
    juce::Image woodLeft, woodRight, woodTop, woodBottom, brushed, brushedDark;   // four boards, as a real cabinet
    SkinSwitch skin, extSw;
    bool dark = false;
    juce::Slider scHpf;
    juce::OwnedArray<juce::TextButton> quality;
    std::unique_ptr<juce::ParameterAttachment> qualityAtt, extAtt, meterAtt;
    int qualitySel = 1, meterSel = 0;
    juce::TextButton presetPrev { "<" }, presetNext { ">" }, presetName;
    void showPresetMenu();
    VuMeter vu;
    juce::Slider input, output, attack, release, mix;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAtt;
    std::vector<KnobSpec> specs;
    juce::OwnedArray<juce::TextButton> ratio;
    std::unique_ptr<juce::ParameterAttachment> ratioAtt;
    juce::ToggleButton inOut;
    std::unique_ptr<juce::ParameterAttachment> bypassAtt;
    bool active = true;
    int ratioSel = 0;
};
} // namespace vintage

class A76Editor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit A76Editor (A76Processor&);
    ~A76Editor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    A76Processor& proc;
    vintage::LookAndFeel lnf;
    vintage::Face face;
    juce::TooltipWindow tips { this, 800 };
};
