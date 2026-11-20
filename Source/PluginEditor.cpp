#include "PluginEditor.h"
#include <axell/gui/ScreenReader.h>
#include <axell/gui/LookAndFeel.h>
#include <random>
#include "A76Fonts.h"

using namespace juce;

namespace vintage
{
//==============================================================================
// palette and type: one system for every character on the panel
namespace c
{
// the skin of the panel being painted (set by Face::paint before anything on it is drawn)
static bool darkSkin = false;
// screen-printed legends: dark ink on silver; on the black skin, light engraving with its shadow edge
static Colour ink() { return darkSkin ? Colour (0xffded6c4) : Colour (0xff1b1916); }
static Colour inkSoft() { return darkSkin ? Colour (0xffa7a093) : Colour (0xff3a3631); }
static Colour inkHi() { return darkSkin ? Colour (0x99000000) : Colour (0x59ffffff); }
const Colour cream (0xfff3e6c4);
const Colour amber (0xffffb347);
const Colour readout (0xffffc46b);
const Colour lampRed (0xffff3b2a);
const Colour vuInk (0xff2a2117);         // slightly faded print on the meter paper
const Colour vuRed (0xff9e2a1c);
} // namespace c

namespace type
{
Font legend (float h = 13.0f) { return ui::caps (h, 1); }       // control names: tracked caps
Font header() { return ui::caps (10.0f, 1); }                     // section headers
Font numeral (float h = 11.5f) { return ui::font (h, 1); }      // scale numerals
Font readout() { return ui::font (12.5f, 1); }                   // value windows
} // namespace type

// the nameplate's handwriting (OFL fonts, Resources/fonts)
static Font script (float h)
{
    static Typeface::Ptr face = Typeface::createSystemTypefaceFor (A76Fonts::GreatVibesRegular_ttf, A76Fonts::GreatVibesRegular_ttfSize);
    return Font (FontOptions (face).withHeight (h));
}

namespace tip
{
const char* input = "INPUT drives the signal into the FET's fixed threshold - turn it up for more compression, as on the hardware (there is no threshold knob). 0 dB puts the 4:1 knee at about -18 dBFS.";
const char* output = "OUTPUT: make-up gain after the compressor, in true dB.";
const char* attack = "ATTACK 1-7, 7 is the fastest (the hardware convention). On the UADx 1176 a 10 dB step reaches 63 % in about 3 ms at 1 and 0.2 ms at 7.";
const char* release = "RELEASE 1-7, 7 is the fastest. Program dependent: long, loud passages recover more slowly, as on the hardware.";
const char* ratio = "RATIO keys. ALL = every key in: the over-compressed, fast-releasing 'all buttons' sound.";
const char* mix = "MIX: parallel compression - the dry signal is delay-matched, so any blend is phase-coherent.";
const char* inOut = "IN / OUT (latency-compensated bypass).";
const char* vu = "Gain reduction, dB.";
} // namespace tip

//==============================================================================
// procedural textures
static float hash2 (int x, int y)
{
    uint32 h = (uint32) x * 374761393u + (uint32) y * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return (float) ((h ^ (h >> 16)) & 0xffffu) / 65535.0f;
}

static float valueNoise (float x, float y)
{
    const int xi = (int) std::floor (x), yi = (int) std::floor (y);
    const float fx = x - (float) xi, fy = y - (float) yi;
    const float sx = fx * fx * (3 - 2 * fx), sy = fy * fy * (3 - 2 * fy);
    const float a = hash2 (xi, yi), b = hash2 (xi + 1, yi), cc = hash2 (xi, yi + 1), d = hash2 (xi + 1, yi + 1);
    return a + (b - a) * sx + (cc - a) * sy + (a - b - cc + d) * sx * sy;
}

// walnut, grain along y
static Image makeWalnut (int w, int h, uint32 seed, float k = 1.0f)   // k: pixels per original texel
{
    Image img (Image::RGB, w, h, false);
    std::mt19937 rng (seed);
    std::uniform_real_distribution<float> u (0.0f, 1.0f);
    const float p1 = u (rng) * 6.28f, p2 = u (rng) * 6.28f, p3 = u (rng) * 6.28f, off = u (rng) * 300.0f;
    Image::BitmapData bd (img, Image::BitmapData::writeOnly);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            const float fy = (float) y / k, fx = (float) x / k + off;
            // the grain drifts slowly and irregularly, mostly straight like a real board (not a regular wave)
            const float wander = 7.0f * std::sin (fy * 0.0029f + p1) + 3.0f * std::sin (fy * 0.0097f + p2) + 1.0f * std::sin (fy * 0.043f + p3)
                               + 13.0f * (valueNoise (fy * 0.0017f, p1 * 3.0f) - 0.5f) + 4.0f * (valueNoise (fy * 0.006f, p2 * 3.0f + 9.0f) - 0.5f);
            const float g = fx + wander;
            // growth rings: never quite periodic - the ring width and the figure drift slowly along and across the board
            const float vary = valueNoise (g * 0.021f + 17.0f, fy * 0.0026f);
            const float phase = g * 0.15f + 2.6f * std::sin (g * 0.027f + fy * 0.0014f) + 0.9f * std::sin (fy * 0.0063f) + 2.4f * (vary - 0.5f);
            const float late = std::pow (0.5f + 0.5f * std::sin (phase), 3.2f + 1.6f * vary);
            // fine fibres: irregular spacing and strength (not a comb)
            const float fn = valueNoise (g * 0.9f, fy * 0.012f);
            const float fine = (0.5f + 0.5f * std::sin (g * 1.6f + 3.5f * fn)) * (0.6f + 0.8f * valueNoise (g * 0.35f + 5.0f, fy * 0.02f));
            const float figure = 0.5f + 0.5f * std::sin (fy * 0.041f + 3.0f * std::sin (g * 0.05f));
            // pores: short dark flecks along the grain with tapered ends, staggered, denser in the light earlywood
            const float gc = g * 0.9f; const int cx = (int) std::floor (gc);
            const float rowOff = 7.0f * hash2 (cx, 77);
            const float rv = (fy + rowOff) / 7.0f; const int cy = (int) std::floor (rv);
            float pore = 0.0f;
            if (hash2 (cx, cy) > 0.905f + 0.07f * late)
            {
                const float pu = gc - (float) cx, pv = rv - (float) cy;
                const float len = 0.35f + 0.6f * hash2 (cx, cy + 5), st = hash2 (cx + 3, cy) * (1.0f - len);
                const float a = (pv - st) / len;
                if (a > 0.0f && a < 1.0f)
                {
                    const float across = (pu - 0.5f) / 0.2f;
                    pore = std::pow (std::sin (3.14159265f * a), 0.7f) * std::exp (-across * across) * (0.55f + 0.45f * hash2 (cx, cy + 9));
                }
            }
            const float n = hash2 (x, y);
            const float r = 128.0f - 56.0f * late - 9.0f * fine + 16.0f * figure - 24.0f * pore + 6.0f * n;
            const float gg = 78.0f - 35.0f * late - 6.0f * fine + 9.0f * figure - 15.0f * pore + 4.0f * n;
            const float b = 44.0f - 19.0f * late - 3.0f * fine + 4.0f * figure - 8.0f * pore + 3.0f * n;
            bd.setPixelColour (x, y, Colour ((uint8) jlimit (0.0f, 255.0f, r), (uint8) jlimit (0.0f, 255.0f, gg), (uint8) jlimit (0.0f, 255.0f, b)));
        }
    return img;
}

// brushed metal, a shade darker than raw aluminium (satin)
static Image makeBrushed (int w, int h, uint32 seed, float top, float bottom, float k = 1.0f)
{
    // one-pole smoothing per original texel, re-derived per pixel so the brushing keeps its look at any k
    const float aL = std::pow (0.97f, 1.0f / k), bL = 0.03f * std::sqrt ((1.0f - aL * aL) / (1.0f - 0.97f * 0.97f));
    const float aR = std::pow (0.78f, 1.0f / k), bR = 0.22f * std::sqrt ((1.0f - aR * aR) / (1.0f - 0.78f * 0.78f));
    Image img (Image::RGB, w, h, false);
    std::mt19937 rng (seed);
    std::normal_distribution<float> nd (0.0f, 1.0f);
    std::vector<float> row ((size_t) h);
    float s = 0.0f;
    for (auto& v : row) { s = aR * s + bR * nd (rng); v = s; }
    Image::BitmapData bd (img, Image::BitmapData::writeOnly);
    for (int y = 0; y < h; ++y)
    {
        float line = 0.0f;
        const float base = top + (bottom - top) * (float) y / (float) h;
        for (int x = 0; x < w; ++x)
        {
            line = aL * line + bL * nd (rng);
            // age: faint cloudy oxidation and the odd fine scratch across the brushing
            const float cloud = 5.0f * (valueNoise ((float) x / k * 0.012f, (float) y / k * 0.02f) - 0.5f);
            const float v = base + 7.0f * row[(size_t) y] + 20.0f * line + 1.6f * nd (rng) + cloud;
            bd.setPixelColour (x, y, Colour ((uint8) jlimit (0.0f, 255.0f, v + 1.0f), (uint8) jlimit (0.0f, 255.0f, v), (uint8) jlimit (0.0f, 255.0f, v - 4.0f)));
        }
    }
    return img;
}

// aged meter paper: cream gone ivory, darker toward the edges, fibres, a few foxing spots
static Image makePaper (int w, int h, float k = 1.0f)
{
    Image img (Image::ARGB, w, h, true);
    Image::BitmapData bd (img, Image::BitmapData::writeOnly);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            const float u = (float) x / (float) w, v = (float) y / (float) h;
            const float edge = std::pow (std::abs (u - 0.5f) * 2.0f, 3.0f) * 0.6f + std::pow (std::abs (v - 0.45f) * 2.0f, 3.0f) * 0.5f;
            const float X = (float) x / k, Y = (float) y / k;
            const float fibre = valueNoise (X * 0.9f, Y * 0.12f) - 0.5f;
            const float blotch = valueNoise (X * 0.03f, Y * 0.03f) - 0.5f;
            float r = 246.0f - 26.0f * edge + 6.0f * fibre + 8.0f * blotch;
            float g = 232.0f - 36.0f * edge + 5.0f * fibre + 7.0f * blotch;
            float b = 196.0f - 52.0f * edge + 4.0f * fibre + 5.0f * blotch;
            // foxing: small brown spots, slightly soft
            float fox = 0.0f;
            for (int s = 0; s < 9; ++s)
            {
                const float fx = hash2 (s, 91) * (float) w / k, fy = hash2 (s, 17) * (float) h / k, fr = 1.5f + 5.0f * hash2 (s, 5);
                const float d = std::hypot (X - fx, Y - fy) / fr;
                fox = jmax (fox, std::exp (-d * d) * (0.25f + 0.35f * hash2 (s, 3)));
            }
            r -= 60.0f * fox; g -= 80.0f * fox; b -= 95.0f * fox;
            bd.setPixelColour (x, y, Colour ((uint8) jlimit (0.0f, 255.0f, r), (uint8) jlimit (0.0f, 255.0f, g), (uint8) jlimit (0.0f, 255.0f, b)));
        }
    return img;
}

//==============================================================================
// drawing helpers
static void print (Graphics& g, const String& t, Rectangle<float> r, const Font& f, Justification j, Colour col = c::ink())
{
    g.setFont (f);
    g.setColour (c::inkHi());
    g.drawText (t, r.translated (0.0f, 0.8f), j, false);
    g.setColour (col);
    g.drawText (t, r, j, false);
}

// a section header: tracked caps between two hairlines, like screen-printed panel graphics
static void sectionHeader (Graphics& g, const String& t, float x0, float x1, float y)
{
    const Font f = type::header();
    const float tw = GlyphArrangement::getStringWidth (f, t) + 14.0f, cx = 0.5f * (x0 + x1);
    print (g, t, { cx - tw * 0.5f, y - 7.0f, tw, 14.0f }, f, Justification::centred, c::inkSoft());
    for (auto seg : { std::pair<float, float> { x0, cx - tw * 0.5f }, std::pair<float, float> { cx + tw * 0.5f, x1 } })
    {
        g.setColour (c::inkHi());
        g.drawLine (seg.first, y + 0.8f, seg.second, y + 0.8f, 0.7f);
        g.setColour (c::inkSoft().withAlpha (0.8f));
        g.drawLine (seg.first, y, seg.second, y, 0.7f);
    }
    g.drawLine (x0, y - 3.0f, x0, y + 3.0f, 0.7f);
    g.drawLine (x1, y - 3.0f, x1, y + 3.0f, 0.7f);
}

//------------------------------------------------------------------------------
// light model: one key light from the top left, ~45 degrees up. A part `height` units above the panel casts a shadow
// down-right whose offset and softness grow with the height; every part also has a tight contact shadow where it
// meets the panel (ambient occlusion). Recessed parts get an inner shadow under their top-left rim.
static void castShadow (Graphics& g, const Path& shape, float height, float strength = 1.0f)
{
    DropShadow (Colours::black.withAlpha (0.42f * strength), jmax (2, roundToInt (1.5f + height * 0.9f)),
                { roundToInt (height * 0.45f), roundToInt (height * 0.70f) }).drawForPath (g, shape);
    DropShadow (Colours::black.withAlpha (0.50f * strength), 2, { 0, 1 }).drawForPath (g, shape);
}

static void innerShadow (Graphics& g, const Path& hole, float depth, float strength = 1.0f)
{
    Graphics::ScopedSaveState ss (g);
    g.reduceClipRegion (hole);
    Path outside;
    outside.addRectangle (hole.getBounds().expanded (depth * 4.0f + 20.0f));
    outside.addPath (hole);
    outside.setUsingNonZeroWinding (false);
    DropShadow (Colours::black.withAlpha (0.55f * strength), roundToInt (2.0f + depth * 0.9f),
                { roundToInt (depth * 0.35f), roundToInt (depth * 0.55f) }).drawForPath (g, outside);
}

static Path ellipsePath (Rectangle<float> r) { Path p; p.addEllipse (r); return p; }
static Path roundRectPath (Rectangle<float> r, float c) { Path p; p.addRoundedRectangle (r, c); return p; }

static void screw (Graphics& g, Point<float> p, float r, float angle)
{
    castShadow (g, ellipsePath (Rectangle<float> (2 * r, 2 * r).withCentre (p)), 1.5f);
    g.setGradientFill (ColourGradient (Colour (0xffeeebe5), p.x - r, p.y - r, Colour (0xff6f6c66), p.x + r, p.y + r, false));
    g.fillEllipse (Rectangle<float> (2 * r, 2 * r).withCentre (p));
    g.setColour (Colours::black.withAlpha (0.5f));
    g.drawEllipse (Rectangle<float> (2 * r, 2 * r).withCentre (p), 0.8f);
    for (float a : { angle, angle + MathConstants<float>::halfPi })   // Phillips
    {
        Path slot;
        slot.addRoundedRectangle (-r * 0.62f, -r * 0.11f, r * 1.24f, r * 0.22f, r * 0.1f);
        slot.applyTransform (AffineTransform::rotation (a).translated (p));
        g.setColour (Colour (0xff2f2d2a));
        g.fillPath (slot);
    }
    g.setColour (Colours::white.withAlpha (0.35f));
    g.drawEllipse (Rectangle<float> (2 * r - 2.0f, 2 * r - 2.0f).withCentre (p.translated (-0.3f, -0.3f)), 0.6f);
}

// a small inset window with an amber-lit value
static void readout (Graphics& g, const String& t, Rectangle<float> r)
{
    g.setColour (Colours::white.withAlpha (0.40f));                       // lit lower lip of the cut-out
    g.fillRoundedRectangle (r.translated (0.6f, 1.0f), 3.0f);
    g.setGradientFill (ColourGradient (Colour (0xff0b0a09), 0.0f, r.getY(), Colour (0xff1d1a16), 0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, 3.0f);
    innerShadow (g, roundRectPath (r, 3.0f), 3.0f);
    g.setColour (Colours::black.withAlpha (0.6f));
    g.drawRoundedRectangle (r, 3.0f, 0.8f);
    g.setFont (type::readout());
    g.setColour (c::readout.withAlpha (0.25f));
    g.drawText (t, r.translated (0.0f, 0.5f).expanded (1.0f, 0.0f), Justification::centred);
    g.setColour (c::readout);
    g.drawText (t, r, Justification::centred);
}

//==============================================================================
// look and feel
void LookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, Slider&)
{
    const auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const auto ctr = b.getCentre();
    const float R = 0.5f * jmin (b.getWidth(), b.getHeight()) * 0.64f;
    const float ang = a0 + pos * (a1 - a0);
    // the knob stands ~R/3 above the panel: a contact shadow under the skirt and a soft cast shadow down-right
    const float rs = R * 1.13f;
    castShadow (g, ellipsePath (Rectangle<float> (2 * rs, 2 * rs).withCentre (ctr)), R * 0.30f);
    // the cap is taller than the skirt: it shades the skirt's lower right
    {
        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (ellipsePath (Rectangle<float> (2 * rs, 2 * rs).withCentre (ctr)));
    }
    // knurled aluminium skirt with a chamfer
    g.setGradientFill (ColourGradient (Colour (0xfff4f2ee), ctr.x - rs, ctr.y - rs, Colour (0xff5c5954), ctr.x + rs, ctr.y + rs, false));
    g.fillEllipse (Rectangle<float> (2 * rs, 2 * rs).withCentre (ctr));
    for (int k = 0; k < 90; ++k)
    {
        const float a = (float) k * MathConstants<float>::twoPi / 90.0f + ang;
        g.setColour (Colours::black.withAlpha (k % 2 ? 0.22f : 0.38f));
        g.drawLine (Line<float> (ctr.getPointOnCircumference (rs * 0.91f, a), ctr.getPointOnCircumference (rs * 0.995f, a)), 0.8f);
    }
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.6f), ctr.x - rs, ctr.y - rs, Colours::black.withAlpha (0.4f), ctr.x + rs, ctr.y + rs, false));
    g.drawEllipse (Rectangle<float> (2 * rs * 0.905f, 2 * rs * 0.905f).withCentre (ctr), 1.0f);
    g.setColour (Colours::black.withAlpha (0.6f));
    g.drawEllipse (Rectangle<float> (2 * rs, 2 * rs).withCentre (ctr), 0.9f);
    g.setColour (Colour (0xff141210));
    g.drawLine (Line<float> (ctr.getPointOnCircumference (rs * 0.86f, ang), ctr.getPointOnCircumference (rs * 1.0f, ang)), 2.2f);
    // bakelite cap: it rises above the skirt and shades it toward the bottom right
    const float rc = R * 0.84f;
    {
        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (ellipsePath (Rectangle<float> (2 * rs, 2 * rs).withCentre (ctr)));
        DropShadow (Colours::black.withAlpha (0.45f), roundToInt (R * 0.16f), { roundToInt (R * 0.06f), roundToInt (R * 0.10f) })
            .drawForPath (g, ellipsePath (Rectangle<float> (2 * rc, 2 * rc).withCentre (ctr)));
    }
    g.setGradientFill (ColourGradient (Colour (0xff4d4743), ctr.x - rc * 0.5f, ctr.y - rc * 0.7f, Colour (0xff080707), ctr.x + rc * 0.6f, ctr.y + rc * 0.8f, true));
    g.fillEllipse (Rectangle<float> (2 * rc, 2 * rc).withCentre (ctr));
    for (int k = 0; k < 28; ++k)
    {
        const float a = (float) k * MathConstants<float>::twoPi / 28.0f + ang;
        g.setColour (Colours::white.withAlpha (0.05f));
        g.drawLine (Line<float> (ctr.getPointOnCircumference (rc * 0.80f, a), ctr.getPointOnCircumference (rc * 0.985f, a)), 1.8f);
        g.setColour (Colours::black.withAlpha (0.25f));
        g.drawLine (Line<float> (ctr.getPointOnCircumference (rc * 0.80f, a + 0.06f), ctr.getPointOnCircumference (rc * 0.985f, a + 0.06f)), 0.8f);
    }
    g.setGradientFill (ColourGradient (Colour (0xff3a3531), ctr.x - rc * 0.3f, ctr.y - rc * 0.4f, Colour (0xff100f0e), ctr.x + rc * 0.4f, ctr.y + rc * 0.5f, true));
    g.fillEllipse (Rectangle<float> (2 * rc * 0.78f, 2 * rc * 0.78f).withCentre (ctr));
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.28f), ctr.x - rc * 0.35f, ctr.y - rc * 0.5f, Colours::transparentWhite, ctr.x, ctr.y + rc * 0.1f, true));
    g.fillEllipse (Rectangle<float> (rc * 1.05f, rc * 0.7f).withCentre (ctr.translated (-rc * 0.2f, -rc * 0.36f)));
    g.setColour (Colours::black.withAlpha (0.85f));
    g.drawEllipse (Rectangle<float> (2 * rc, 2 * rc).withCentre (ctr), 1.0f);
    const float pw = jmax (2.0f, rc * 0.075f);
    g.setColour (Colours::black.withAlpha (0.5f));
    g.drawLine (Line<float> (ctr.getPointOnCircumference (rc * 0.30f, ang), ctr.getPointOnCircumference (rc * 0.86f, ang)), pw + 1.2f);
    g.setColour (c::cream);
    g.drawLine (Line<float> (ctr.getPointOnCircumference (rc * 0.30f, ang), ctr.getPointOnCircumference (rc * 0.86f, ang)), pw);
}

void LookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool over, bool down)
{
    if ((bool) b.getProperties()["display"])
    {
        // an amber-lit display window (the preset name)
        auto r = b.getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (Colours::white.withAlpha (0.12f));
        g.fillRoundedRectangle (r.translated (0.0f, 1.0f), 3.0f);
        g.setGradientFill (ColourGradient (Colour (0xff070605), 0.0f, r.getY(), Colour (0xff17130e), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 3.0f);
        innerShadow (g, roundRectPath (r, 3.0f), 3.0f);
        g.setColour (Colours::black.withAlpha (0.7f));
        g.drawRoundedRectangle (r, 3.0f, 0.8f);
        if (over) { g.setColour (c::amber.withAlpha (0.06f)); g.fillRoundedRectangle (r, 3.0f); }
        ignoreUnused (down);
        return;
    }
    auto r = b.getLocalBounds().toFloat().reduced (2.0f);
    const bool on = b.getToggleState();
    castShadow (g, roundRectPath (r, 4.0f), 3.0f);
    g.setGradientFill (ColourGradient (Colour (0xffe6e3dd), r.getX(), r.getY(), Colour (0xff4e4c48), r.getRight(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (Colour (0xff0d0c0b));
    g.fillRoundedRectangle (r.reduced (2.0f), 3.0f);
    innerShadow (g, roundRectPath (r.reduced (2.0f), 3.0f), 4.0f);   // the key sits in a well
    auto cap = r.reduced (3.5f).withTrimmedBottom (on || down ? 0.0f : 1.5f).translated (0.0f, on || down ? 1.0f : 0.0f);
    g.setGradientFill (ColourGradient (on ? Colour (0xff4a3418) : Colour (0xff35322f), cap.getX(), cap.getY(),
                                       on ? Colour (0xff1e150a) : Colour (0xff141312), cap.getX(), cap.getBottom(), false));
    // a raised key casts a little shadow into its well; a pressed one sits flush
    if (! (on || down))
        DropShadow (Colours::black.withAlpha (0.6f), 3, { 1, 2 }).drawForPath (g, roundRectPath (cap, 2.5f));
    g.fillRoundedRectangle (cap, 2.5f);
    if (on)
    {
        g.setGradientFill (ColourGradient (c::amber.withAlpha (0.6f), cap.getCentreX(), cap.getCentreY(), c::amber.withAlpha (0.0f), cap.getRight() + 8.0f, cap.getCentreY(), true));
        g.fillRoundedRectangle (cap, 2.5f);
    }
    g.setColour (Colours::white.withAlpha (over ? 0.20f : 0.10f));
    g.drawLine (cap.getX() + 3.0f, cap.getY() + 0.8f, cap.getRight() - 3.0f, cap.getY() + 0.8f, 1.0f);
}

void LookAndFeel::drawButtonText (Graphics& g, TextButton& b, bool, bool down)
{
    if ((bool) b.getProperties()["display"])
    {
        g.setFont (ui::font (13.0f, 1));
        g.setColour (c::readout);
        g.drawText (b.getButtonText(), b.getLocalBounds().reduced (24, 0), Justification::centred);
        // a small drop-down caret at the right
        Path caret;
        const float cx = (float) b.getWidth() - 14.0f, cy = (float) b.getHeight() * 0.5f;
        caret.addTriangle (cx - 4.0f, cy - 2.0f, cx + 4.0f, cy - 2.0f, cx, cy + 3.0f);
        g.setColour (c::readout.withAlpha (0.8f));
        g.fillPath (caret);
        return;
    }
    const bool on = b.getToggleState();
    const auto r = b.getLocalBounds().toFloat().translated (0.0f, on || down ? 1.0f : 0.0f);
    g.setFont (ui::caps (b.getHeight() < 30 ? 11.0f : (b.getButtonText().length() > 2 ? 12.5f : 14.5f), 2));
    if (on)
    {
        g.setColour (c::amber.withAlpha (0.35f));
        g.drawText (b.getButtonText(), r.translated (0.0f, 0.5f), Justification::centred);
    }
    g.setColour (on ? Colour (0xffffe7b8) : Colour (0xffcfc8b8));
    g.drawText (b.getButtonText(), r, Justification::centred);
}

void LookAndFeel::drawToggleButton (Graphics& g, ToggleButton& b, bool over, bool down)
{
    // a panel-mount bat-handle toggle, seen from the front: knurled locking ring, hex nut (each facet lit on its own),
    // threaded bushing, and a chrome bat lever leaning out of the bushing toward the viewer. Light from the top left.
    const auto r = b.getLocalBounds().toFloat();
    const auto ctr = r.getCentre();
    const float u = jmin (r.getWidth(), r.getHeight() * 0.72f) * 0.5f;     // unit: the locking ring radius
    const Point<float> light (-0.6f, -0.8f);
    auto lit = [&] (Point<float> n) { return jlimit (0.0f, 1.0f, 0.5f + 0.5f * (n.x * light.x + n.y * light.y)); };

    // the locking ring sits low on the panel
    castShadow (g, ellipsePath (Rectangle<float> (2 * u, 2 * u).withCentre (ctr)), 3.0f);
    // knurled locking ring
    g.setGradientFill (ColourGradient (Colour (0xfff2f0eb), ctr.x - u, ctr.y - u, Colour (0xff55524d), ctr.x + u, ctr.y + u, false));
    g.fillEllipse (Rectangle<float> (2 * u, 2 * u).withCentre (ctr));
    for (int k = 0; k < 64; ++k)
    {
        const float a = (float) k * MathConstants<float>::twoPi / 64.0f;
        g.setColour (Colours::black.withAlpha (k % 2 ? 0.18f : 0.34f));
        g.drawLine (Line<float> (ctr.getPointOnCircumference (u * 0.84f, a), ctr.getPointOnCircumference (u * 0.99f, a)), 0.8f);
    }
    g.setColour (Colours::black.withAlpha (0.55f));
    g.drawEllipse (Rectangle<float> (2 * u, 2 * u).withCentre (ctr), 0.8f);
    // hex nut: six facets on a chamfered top
    const float hr = u * 0.80f, top = u * 0.56f;
    for (int k = 0; k < 6; ++k)
    {
        const float a0 = (float) k * MathConstants<float>::pi / 3.0f + MathConstants<float>::pi / 6.0f, a1 = a0 + MathConstants<float>::pi / 3.0f;
        const float am = 0.5f * (a0 + a1);
        Path f;
        f.startNewSubPath (ctr.getPointOnCircumference (hr, a0));
        f.lineTo (ctr.getPointOnCircumference (hr, a1));
        f.lineTo (ctr.getPointOnCircumference (top, a1));
        f.lineTo (ctr.getPointOnCircumference (top, a0));
        f.closeSubPath();
        const float l = lit ({ std::sin (am), -std::cos (am) });
        g.setColour (Colour::fromFloatRGBA (0.30f + 0.62f * l, 0.30f + 0.61f * l, 0.29f + 0.59f * l, 1.0f));
        g.fillPath (f);
        g.setColour (Colours::black.withAlpha (0.28f));
        g.strokePath (f, PathStrokeType (0.5f));
    }
    Path hexTop;
    for (int k = 0; k < 6; ++k)
    {
        const auto p = ctr.getPointOnCircumference (top, (float) k * MathConstants<float>::pi / 3.0f + MathConstants<float>::pi / 6.0f);
        k == 0 ? hexTop.startNewSubPath (p) : hexTop.lineTo (p);
    }
    hexTop.closeSubPath();
    g.setGradientFill (ColourGradient (Colour (0xffe4e1db), ctr.x - top, ctr.y - top, Colour (0xff8d8a84), ctr.x + top, ctr.y + top, false));
    g.fillPath (hexTop);
    // threaded bushing: a short chrome cylinder end with thread rings and a dark bore
    const float br = u * 0.40f;
    g.setGradientFill (ColourGradient (Colour (0xfffbfaf8), ctr.x - br, ctr.y - br, Colour (0xff5f5c57), ctr.x + br, ctr.y + br, false));
    g.fillEllipse (Rectangle<float> (2 * br, 2 * br).withCentre (ctr));
    for (float k : { 0.92f, 0.80f, 0.68f })
    {
        g.setColour (Colours::black.withAlpha (0.30f));
        g.drawEllipse (Rectangle<float> (2 * br * k, 2 * br * k).withCentre (ctr.translated (0.3f, 0.4f)), 0.6f);
        g.setColour (Colours::white.withAlpha (0.35f));
        g.drawEllipse (Rectangle<float> (2 * br * k, 2 * br * k).withCentre (ctr.translated (-0.3f, -0.4f)), 0.4f);
    }
    g.setGradientFill (ColourGradient (Colour (0xff050505), ctr.x, ctr.y, Colour (0xff2b2926), ctr.x + br * 0.5f, ctr.y + br * 0.5f, true));
    g.fillEllipse (Rectangle<float> (br * 1.0f, br * 1.0f).withCentre (ctr));

    // the bat lever: up = IN. It leans out toward the viewer, so it is foreshortened and widens toward the ball.
    const bool up = b.getToggleState() != down;     // while pressed it shows where it is going
    const float dir = up ? -1.0f : 1.0f;
    const float len = r.getHeight() * 0.33f;
    const Point<float> base = ctr, tip = ctr.translated (0.0f, dir * len);
    const float w0 = br * 0.42f, w1 = br * 0.62f, ball = br * 0.86f;
    auto leverPath = [&] (Point<float> off)
    {
        Path p;
        p.startNewSubPath (base.translated (-w0, 0.0f) + off);
        p.cubicTo (base.translated (-w0, dir * len * 0.4f) + off, tip.translated (-w1, -dir * len * 0.35f) + off, tip.translated (-w1, 0.0f) + off);
        p.lineTo (tip.translated (w1, 0.0f) + off);
        p.cubicTo (tip.translated (w1, -dir * len * 0.35f) + off, base.translated (w0, dir * len * 0.4f) + off, base.translated (w0, 0.0f) + off);
        p.closeSubPath();
        return p;
    };
    // cast shadow: the lever rises from the bushing toward the viewer, so its shadow starts at the bushing and stretches
    // down-right, longer and softer toward the ball (the highest point)
    {
        Path sh = leverPath ({});
        sh.addEllipse (Rectangle<float> (2 * ball, 2 * ball).withCentre (tip));
        const float hgt = len * 0.55f;
        // skew the shadow: points further from the bushing are displaced more
        sh.applyTransform (AffineTransform (1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));
        Path skewed;
        for (int i = 0; i < 6; ++i)
        {
            Path seg = sh;
            const float t = (float) (i + 1) / 6.0f;
            seg.applyTransform (AffineTransform::translation (hgt * 0.45f * t, hgt * 0.70f * t));
            DropShadow (Colours::black.withAlpha (0.12f), roundToInt (1.0f + 4.0f * t), {}).drawForPath (g, seg);
        }
        DropShadow (Colours::black.withAlpha (0.5f), 2, { 0, 1 }).drawForPath (g, ellipsePath (Rectangle<float> (2 * w0 + 2, 2 * w0 + 2).withCentre (base)));
    }
    // body: a polished cylinder - dark edges, a bright stripe left of centre, a warm reflection of the panel on the right
    const auto body = leverPath ({});
    ColourGradient cyl (Colour (0xff3d3b38), base.x - w1, 0.0f, Colour (0xff2e2c29), base.x + w1, 0.0f, false);
    cyl.addColour (0.18, Colour (0xffb9b6b0));
    cyl.addColour (0.34, Colour (0xffffffff));
    cyl.addColour (0.48, Colour (0xffd2cfc9));
    cyl.addColour (0.70, Colour (0xff6e6b66));
    cyl.addColour (0.86, Colour (0xff9a958c));
    g.setGradientFill (cyl);
    g.fillPath (body);
    // where the lever enters the bushing it falls into shadow
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.55f), base.x, base.y, Colours::black.withAlpha (0.0f), base.x, base.y + dir * len * 0.35f, false));
    g.fillPath (body);
    g.setColour (Colours::black.withAlpha (0.45f));
    g.strokePath (body, PathStrokeType (0.6f));
    // ball end: a chrome sphere with a sharp specular point and the panel reflected in its lower half
    const auto ballR = Rectangle<float> (2 * ball, 2 * ball).withCentre (tip);
    ColourGradient sph (Colours::white, tip.x - ball * 0.38f, tip.y - ball * 0.42f, Colour (0xff3a3835), tip.x + ball * 1.0f, tip.y + ball * 1.0f, true);
    sph.addColour (0.35, Colour (0xffc9c6c0));
    sph.addColour (0.70, Colour (0xff6c6964));
    g.setGradientFill (sph);
    g.fillEllipse (ballR);
    g.setGradientFill (ColourGradient (Colour (0x00ffffff), tip.x, tip.y, Colour (0x55d8d2c4), tip.x, tip.y + ball, false));
    g.fillEllipse (ballR.withTrimmedTop (ball * 0.9f));
    g.setColour (Colours::white.withAlpha (0.95f));
    g.fillEllipse (Rectangle<float> (ball * 0.42f, ball * 0.32f).withCentre (tip.translated (-ball * 0.34f, -ball * 0.38f)));
    g.setColour (Colours::black.withAlpha (0.5f));
    g.drawEllipse (ballR, 0.6f);
    if (over)
    {
        g.setColour (Colours::white.withAlpha (0.06f));
        g.fillEllipse (Rectangle<float> (2.2f * u, 2.2f * u).withCentre (ctr));
    }
}

void LookAndFeel::drawTooltip (Graphics& g, const String& text, int w, int h)
{
    g.setColour (Colour (0xfff6edd5));
    g.fillRect (0, 0, w, h);
    g.setColour (Colour (0xff6b5a3a));
    g.drawRect (0, 0, w, h, 1);
    g.setColour (c::ink());
    g.setFont (ui::font (13.0f));
    g.drawFittedText (text, Rectangle<int> (w, h).reduced (8, 5), Justification::centredLeft, 6);
}

//==============================================================================
// VU meter: an old movement behind old glass, lit by two tired incandescent bulbs
void VuMeter::setGr (float db)
{
    // the needle moves in scale position (0 = left stop, 1 = right): GR or the classic VU output scale
    float target;
    if (mode == 0)
        target = 1.0f - std::pow (jlimit (0.0f, 20.0f, db) / 20.0f, 0.62f);
    else
    {
        static const float vuDb[] = { -20, -10, -7, -5, -3, -2, -1, 0, 1, 2, 3 };
        static const float vuPos[] = { 0.0f, 0.22f, 0.33f, 0.43f, 0.55f, 0.62f, 0.69f, 0.76f, 0.84f, 0.92f, 1.0f };
        const float v = jlimit (-22.0f, 4.0f, outLevel - (mode == 1 ? -18.0f : -14.0f));
        target = v <= -20.0f ? (v + 22.0f) / 2.0f * 0.0f : vuPos[10];
        for (int k = 0; k < 10; ++k)
            if (v <= vuDb[k + 1]) { target = vuPos[k] + (vuPos[k + 1] - vuPos[k]) * (v - vuDb[k]) / (vuDb[k + 1] - vuDb[k]); break; }
        if (v < -20.0f) target = 0.0f;
    }
    if (mode == 0)
        vel = 0.55f * vel + 0.30f * (target - shown);
    else    // 3.2.2: the processor models the VU movement itself; here only the 40 Hz steps are smoothed (~25 ms)
        vel = 0.6f * (target - shown);
    shown = jlimit (-0.03f, 1.03f, shown + vel);
    // the backlight breathes: a slow random drift of a couple of percent, now and then a tiny dip
    if ((++ticks % 12u) == 0u)
        lampTarget = 0.965f + 0.035f * hash2 ((int) ticks, 77) - (hash2 ((int) ticks, 13) > 0.97f ? 0.05f : 0.0f);
    lamp += (lampTarget - lamp) * 0.08f;
    repaint();
}

void VuMeter::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // chrome bezel
    g.setGradientFill (ColourGradient (Colour (0xffeeebe5), b.getX(), b.getY(), Colour (0xff4f4d48), b.getRight(), b.getBottom(), false));
    g.fillRoundedRectangle (b, 9.0f);
    g.setColour (Colours::white.withAlpha (0.5f));
    g.drawRoundedRectangle (b.reduced (1.0f), 8.0f, 0.8f);
    auto bez = b.reduced (5.0f);
    g.setColour (Colour (0xff0f0e0d));
    g.fillRoundedRectangle (bez, 6.0f);
    auto face = bez.reduced (8.0f);
    {
        // the paper follows the pixel density it is drawn at (never below the start value, at most 2 steps above 4x)
        const float px = g.getInternalContext().getPhysicalPixelScaleFactor();
        float ts = std::max (vintage::textureScale, std::min (std::ceil (px * 2.0f) / 2.0f, std::max (vintage::textureScale, vintage::kMaxTextureScale)));
        if (paper.isNull() || ts > paperScale + 0.01f)
        {
            paperScale = ts;
            paper = makePaper ((int) ((float) (int) face.getWidth() * ts), (int) ((float) (int) face.getHeight() * ts), ts / 2.0f);
        }
    }
    // aged paper, then the backlight: two bulbs low left and right, the left one a little stronger, both breathing
    g.setOpacity (1.0f);
    g.drawImage (paper, face);
    {
        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (face.toNearestInt());
        const float L = lamp;
        for (auto [fx, k] : { std::pair<float, float> { 0.28f, 1.0f }, std::pair<float, float> { 0.74f, 0.82f } })
        {
            const Point<float> bulb { face.getX() + face.getWidth() * fx, face.getBottom() + 6.0f };
            g.setGradientFill (ColourGradient (Colour (0xffffc35c).withAlpha (0.42f * k * L), bulb.x, bulb.y, Colour (0xffffb347).withAlpha (0.0f), bulb.x + face.getWidth() * 0.55f, bulb.y, true));
            g.fillRect (face);
        }
        // the top corners stay in the dark
        g.setGradientFill (ColourGradient (Colours::transparentBlack, face.getCentreX(), face.getBottom(), Colour (0xff2a1a08).withAlpha (0.38f + 0.1f * (1.0f - L)), face.getX(), face.getY(), true));
        g.fillRect (face);
    }
    g.saveState();
    g.reduceClipRegion (face.toNearestInt());
    const Point<float> pivot { face.getCentreX(), face.getBottom() + face.getHeight() * 0.38f };
    const float rad = face.getHeight() * 1.12f;
    const float aL = -0.74f, aR = 0.74f;
    auto angleFor = [&] (float gr) { const float u = 1.0f - std::pow (jlimit (0.0f, 20.0f, gr) / 20.0f, 0.62f); return aL + u * (aR - aL); };
    const float rArc = rad - 22.0f;
    if (mode == 0)
    {
    Path arc, red;
    arc.addCentredArc (pivot.x, pivot.y, rArc, rArc, 0.0f, angleFor (10.0f), aR, true);
    red.addCentredArc (pivot.x, pivot.y, rArc + 2.0f, rArc + 2.0f, 0.0f, aL, angleFor (10.0f), true);
    g.setColour (c::vuInk);
    g.strokePath (arc, PathStrokeType (1.3f));
    g.setColour (c::vuRed.withAlpha (0.92f));
    g.strokePath (red, PathStrokeType (4.5f));
    for (int d = 0; d <= 10; ++d)
    {
        const float a = angleFor ((float) d);
        g.setColour (c::vuInk);
        g.drawLine (Line<float> (pivot.getPointOnCircumference (rArc, a), pivot.getPointOnCircumference (rArc - 5.0f, a)), 0.9f);
    }
    g.setFont (ui::font (14.0f, 1));
    for (float d : { 0.0f, 1.0f, 2.0f, 3.0f, 5.0f, 7.0f, 10.0f, 15.0f, 20.0f })
    {
        const float a = angleFor (d);
        g.setColour (d > 9.5f ? c::vuRed : c::vuInk);
        g.drawLine (Line<float> (pivot.getPointOnCircumference (rArc, a), pivot.getPointOnCircumference (rArc + 9.0f, a)), 1.6f);
        const auto pt = pivot.getPointOnCircumference (rArc + 19.0f, a);
        g.drawText (d < 0.5f ? "0" : String ((int) d), Rectangle<float> (30, 15).withCentre (pt), Justification::centred);
    }
    g.setColour (c::vuInk.withAlpha (0.9f));
    g.setFont (ui::caps (11.0f, 1));
    g.drawText ("GAIN  REDUCTION", face.withTrimmedTop (face.getHeight() * 0.60f).withHeight (14.0f), Justification::centred);
    g.setFont (ui::font (9.5f, 1));
    g.drawText ("dB", face.withTrimmedTop (face.getHeight() * 0.60f + 14.0f).withHeight (12.0f), Justification::centred);
    }
    else
    {
        // the classic VU scale: -20 .. +3, red from 0 VU
        static const float vuDb[] = { -20, -10, -7, -5, -3, -2, -1, 0, 1, 2, 3 };
        static const float vuPos[] = { 0.0f, 0.22f, 0.33f, 0.43f, 0.55f, 0.62f, 0.69f, 0.76f, 0.84f, 0.92f, 1.0f };
        auto ang = [&] (float u) { return aL + u * (aR - aL); };
        Path arc2, red2;
        arc2.addCentredArc (pivot.x, pivot.y, rArc, rArc, 0.0f, aL, ang (0.76f), true);
        red2.addCentredArc (pivot.x, pivot.y, rArc + 2.0f, rArc + 2.0f, 0.0f, ang (0.76f), aR, true);
        g.setColour (c::vuInk);
        g.strokePath (arc2, PathStrokeType (1.3f));
        g.setColour (c::vuRed.withAlpha (0.92f));
        g.strokePath (red2, PathStrokeType (4.5f));
        g.setFont (ui::font (14.0f, 1));
        for (int k = 0; k < 11; ++k)
        {
            const float a = ang (vuPos[k]);
            const bool redZone = vuDb[k] > 0.5f;
            g.setColour (redZone ? c::vuRed : c::vuInk);
            g.drawLine (Line<float> (pivot.getPointOnCircumference (rArc, a), pivot.getPointOnCircumference (rArc + 9.0f, a)), 1.6f);
            if (k != 5 && k != 9)   // -2 and +2 stay unlabelled, as on the classic dial
            {
                const auto pt = pivot.getPointOnCircumference (rArc + 19.0f, a);
                const String t = vuDb[k] > 0.5f ? "+" + String ((int) vuDb[k]) : String ((int) vuDb[k]);
                g.drawText (t, Rectangle<float> (30, 15).withCentre (pt), Justification::centred);
            }
        }
        g.setColour (c::vuInk.withAlpha (0.9f));
        g.setFont (ui::caps (15.0f, 2));
        g.drawText ("VU", face.withTrimmedTop (face.getHeight() * 0.56f).withHeight (18.0f), Justification::centred);
        g.setFont (ui::caps (9.5f, 1));
        g.drawText (mode == 1 ? "OUTPUT  +4" : "OUTPUT  +8", face.withTrimmedTop (face.getHeight() * 0.56f + 18.0f).withHeight (12.0f), Justification::centred);
    }
    g.setColour (Colour (0xff7a6748));
    g.setFont (ui::caps (8.0f, 1));
    g.drawText ("AXELL", face.withTrimmedBottom (4.0f).withTrimmedLeft (8.0f), Justification::bottomLeft);
    g.drawText ("FET", face.withTrimmedBottom (4.0f).withTrimmedRight (8.0f), Justification::bottomRight);
    // needle: rests a hair right of zero (an old movement), with its shadow on the paper
    const float a = aL + shown * (aR - aL) + 0.004f;
    auto needle = [&] (Point<float> off, Colour col, float wdt)
    {
        g.setColour (col);
        g.drawLine (Line<float> (pivot.getPointOnCircumference (rad * 0.40f, a) + off, pivot.getPointOnCircumference (rad - 9.0f, a) + off), wdt);
    };
    needle ({ 4.0f, 4.5f }, Colours::black.withAlpha (0.14f), 2.4f);
    needle ({}, Colour (0xff15110c), 1.5f);
    g.setColour (Colour (0xff1a1714));
    g.fillEllipse (Rectangle<float> (face.getWidth() * 0.34f, face.getHeight() * 0.42f).withCentre ({ face.getCentreX(), face.getBottom() + face.getHeight() * 0.08f }));
    g.restoreState();
    // old glass: a reflection, dust, one hairline scratch, a darker inner edge
    Path refl;
    refl.startNewSubPath (face.getTopLeft());
    refl.lineTo (face.getX() + face.getWidth() * 0.58f, face.getY());
    refl.lineTo (face.getX() + face.getWidth() * 0.18f, face.getBottom());
    refl.lineTo (face.getX(), face.getBottom());
    refl.closeSubPath();
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.20f), face.getX(), face.getY(), Colours::white.withAlpha (0.0f), face.getX() + face.getWidth() * 0.4f, face.getBottom(), false));
    g.fillPath (refl);
    for (int k = 0; k < 46; ++k)
    {
        const float x = face.getX() + hash2 (k, 401) * face.getWidth(), y = face.getY() + hash2 (k, 402) * face.getHeight();
        const float s = 0.5f + 1.3f * hash2 (k, 403);
        g.setColour ((hash2 (k, 404) > 0.3f ? Colours::white : Colour (0xff3b2a14)).withAlpha (0.10f + 0.18f * hash2 (k, 405)));
        g.fillEllipse (x, y, s, s);
    }
    g.setColour (Colours::white.withAlpha (0.16f));
    g.drawLine (face.getX() + face.getWidth() * 0.62f, face.getY() + face.getHeight() * 0.18f, face.getX() + face.getWidth() * 0.80f, face.getY() + face.getHeight() * 0.32f, 0.5f);
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.0f), face.getCentreX(), face.getCentreY(), Colours::black.withAlpha (0.25f), face.getX(), face.getY(), true));
    g.fillRoundedRectangle (face, 3.0f);
    // the paper sits deep behind the bezel: its top-left falls into shadow
    innerShadow (g, roundRectPath (face, 3.0f), 9.0f, 1.1f);
    innerShadow (g, roundRectPath (bez, 6.0f), 3.0f, 0.8f);
    g.setColour (Colours::black.withAlpha (0.45f));
    g.drawRoundedRectangle (face, 3.0f, 1.6f);
}

//==============================================================================
namespace geo
{
const Rectangle<int> input { 52, 140, 172, 172 }, output { 246, 140, 172, 172 }, attack { 446, 116, 100, 100 }, release { 446, 262, 100, 100 },
                     mix { 852, 300, 86, 86 }, vu { 654, 92, 300, 178 }, inOut { 666, 298, 70, 90 };
constexpr float lampX = 778.0f, lampY = 336.0f;
constexpr int ratioX = 584, ratioY = 116, ratioW = 50, ratioH = 40, ratioGap = 10;
} // namespace geo

Face::Face (A76Processor& p) : proc (p)
{
    setLookAndFeel (&lnf);
    buildTextures (vintage::textureScale);

    plate.setInterceptsMouseClicks (false, true);
    addAndMakeVisible (plate);
    plate.setBounds (kSide, kRail, kW, kH);
    plate.addAndMakeVisible (vu);
    vu.setBounds (geo::vu);
    vu.setTooltip (tip::vu);
    addKnob (input, "input", tip::input, geo::input);
    addKnob (output, "output", tip::output, geo::output);
    addKnob (attack, "attack", tip::attack, geo::attack);
    addKnob (release, "release", tip::release, geo::release);
    addKnob (mix, "mix", tip::mix, geo::mix);
    specs = { { &input, "INPUT", { { -24, "-24" }, { -12, "-12" }, { 0, "0" }, { 12, "+12" }, { 24, "+24" }, { 36, "+36" } }, 3, true },
              { &output, "OUTPUT", { { -36, "-36" }, { -24, "-24" }, { -12, "-12" }, { 0, "0" }, { 12, "+12" }, { 24, "+24" } }, 3, true },
              { &attack, "ATTACK", { { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" } }, 1, false },
              { &release, "RELEASE", { { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" } }, 1, false },
              { &mix, "MIX", { { 0, "DRY" }, { 50, "50" }, { 100, "WET" } }, 4, false } };

    const char* names[] = { "4", "8", "12", "20", "ALL" };
    for (int i = 0; i < 5; ++i)
    {
        auto* b = ratio.add (new TextButton (names[i]));
        b->setClickingTogglesState (false);
        b->setTooltip (tip::ratio);
        b->setBounds (geo::ratioX, geo::ratioY + i * (geo::ratioH + geo::ratioGap), geo::ratioW, geo::ratioH);
        b->onClick = [this, i] { if (ratioAtt) ratioAtt->setValueAsCompleteGesture ((float) i); };
        plate.addAndMakeVisible (b);
    }
    ratioAtt = std::make_unique<ParameterAttachment> (*proc.apvts.getParameter ("ratio"), [this] (float v) {
        ratioSel = roundToInt (v);
        for (int i = 0; i < ratio.size(); ++i) ratio[i]->setToggleState (i == ratioSel, dontSendNotification);
        repaint();
    });
    ratioAtt->sendInitialUpdate();

    inOut.setTooltip (tip::inOut);
    inOut.setBounds (geo::inOut);
    inOut.setClickingTogglesState (false);
    inOut.onClick = [this] { if (bypassAtt) bypassAtt->setValueAsCompleteGesture (inOut.getToggleState() ? 1.0f : 0.0f); };
    plate.addAndMakeVisible (inOut);
    bypassAtt = std::make_unique<ParameterAttachment> (*proc.apvts.getParameter ("bypass"), [this] (float v) {
        active = v < 0.5f; inOut.setToggleState (active, dontSendNotification); repaint();
    });
    bypassAtt->sendInitialUpdate();
    dark = (int) proc.apvts.state.getProperty ("skin", 0) == 1;
    skin.dark = dark;
    skin.setTooltip ("Panel: silver or black.");
    skin.setBounds (300, 444, 38, 18);
    skin.onFlip = [this] {
        dark = ! dark;
        skin.dark = dark;
        proc.apvts.state.setProperty ("skin", dark ? 1 : 0, nullptr);
        repaint();
        for (auto* ch : plate.getChildren()) ch->repaint();
    };
    plate.addAndMakeVisible (skin);

    // 2.0 utility strip: side-chain high-pass, internal / external side-chain, oversampling
    addKnob (scHpf, "scHpf", "HPF: a high-pass on the side-chain only - bass stops pumping the compression; the audio is untouched. OFF = the measured 1176 behaviour.", { 48, 352, 80, 80 });
    specs.push_back ({ &scHpf, "", { { 0, "OFF" }, { 100, "100" }, { 500, "500" } }, 1, false });
    extSw.setTooltip ("Side-chain source: INT = the input; EXT = the plugin's side-chain input (route a track to it in your DAW).");
    extSw.setBounds (232, 383, 38, 18);
    extSw.onFlip = [this] { if (extAtt) extAtt->setValueAsCompleteGesture (extSw.dark ? 0.0f : 1.0f); };
    plate.addAndMakeVisible (extSw);
    extAtt = std::make_unique<ParameterAttachment> (*proc.apvts.getParameter ("extSc"), [this] (float v) { extSw.dark = v > 0.5f; extSw.repaint(); repaint(); });
    extAtt->sendInitialUpdate();
    const char* qn[] = { "2x", "4x", "8x" };
    for (int i = 0; i < 3; ++i)
    {
        auto* b = quality.add (new TextButton (qn[i]));
        b->setClickingTogglesState (false);
        b->setTooltip ("QUALITY - oversampling: 2x Standard, 4x High (default), 8x Ultra. Higher = less aliasing on hard, fast settings; a little more CPU and latency.");
        b->setBounds (318 + i * 38, 378, 34, 26);
        b->onClick = [this, i] { if (qualityAtt) qualityAtt->setValueAsCompleteGesture ((float) i); };
        plate.addAndMakeVisible (b);
    }
    qualityAtt = std::make_unique<ParameterAttachment> (*proc.apvts.getParameter ("quality"), [this] (float v) {
        qualitySel = roundToInt (v);
        for (int i = 0; i < quality.size(); ++i) quality[i]->setToggleState (i == qualitySel, dontSendNotification);
    });
    qualityAtt->sendInitialUpdate();
    // the meter: click to cycle GR / +4 / +8, like the hardware's meter keys
    meterAtt = std::make_unique<ParameterAttachment> (*proc.apvts.getParameter ("meter"), [this] (float v) { meterSel = roundToInt (v); vu.repaint(); });
    meterAtt->sendInitialUpdate();
    vu.setTooltip ("Click to switch the meter: GAIN REDUCTION, OUTPUT +4 (0 VU = -18 dBFS), OUTPUT +8 (0 VU = -14 dBFS).");
    vu.onClick = [this] { if (meterAtt) meterAtt->setValueAsCompleteGesture ((float) ((meterSel + 1) % 3)); };
    // presets: an amber display in the name band, with arrows
    presetName.getProperties().set ("display", true);
    presetName.setTooltip ("Factory presets");
    presetName.onClick = [this] { showPresetMenu(); };
    presetPrev.onClick = [this] { proc.setCurrentProgram ((proc.getCurrentProgram() + proc.getNumPrograms() - 1) % proc.getNumPrograms()); repaint(); };
    presetNext.onClick = [this] { proc.setCurrentProgram ((proc.getCurrentProgram() + 1) % proc.getNumPrograms()); repaint(); };
    presetPrev.setBounds (606, 17, 28, 28);
    presetName.setBounds (638, 17, 236, 28);
    presetNext.setBounds (878, 17, 28, 28);
    for (auto* b : { &presetPrev, &presetName, &presetNext }) plate.addAndMakeVisible (b);
    setSize (kTotalW, kTotalH);
}

Face::~Face() { setLookAndFeel (nullptr); }

void Face::addKnob (Slider& s, const char* id, const char* tp, Rectangle<int> bounds)
{
    s.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    s.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
    s.setMouseDragSensitivity (220);
    s.setTooltip (tp);
    s.onValueChange = [this] { repaint(); };
    s.setBounds (bounds);
    plate.addAndMakeVisible (s);
    sliderAtt.push_back (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, s));
}

void Face::paintKnobLegend (Graphics& g, const KnobSpec& k)
{
    const auto b = k.s->getBounds().toFloat();
    const auto ctr = b.getCentre();
    const float R = 0.5f * jmin (b.getWidth(), b.getHeight()) * 0.64f;
    const auto rp = k.s->getRotaryParameters();
    const float a0 = rp.startAngleRadians - MathConstants<float>::twoPi, a1 = rp.endAngleRadians - MathConstants<float>::twoPi;
    const float r0 = R * 1.22f, rMinor = R * 1.29f, rMajor = R * 1.36f;
    Path arc;
    arc.addCentredArc (ctr.x, ctr.y, r0, r0, 0.0f, a0, a1, true);
    g.setColour (c::inkHi());
    g.strokePath (arc, PathStrokeType (0.7f), AffineTransform::translation (0.0f, 0.8f));
    g.setColour (c::ink().withAlpha (0.8f));
    g.strokePath (arc, PathStrokeType (0.7f));
    std::vector<float> majors;
    for (auto& [v, txt] : k.scale) majors.push_back ((float) k.s->valueToProportionOfLength (v));
    for (size_t i = 0; i < majors.size(); ++i)
    {
        const float a = a0 + (a1 - a0) * majors[i];
        g.setColour (c::ink());
        g.drawLine (Line<float> (ctr.getPointOnCircumference (r0, a), ctr.getPointOnCircumference (rMajor, a)), 1.2f);
        if (i + 1 < majors.size())
            for (int m = 1; m <= k.minorPerMajor; ++m)
            {
                const float am = a0 + (a1 - a0) * (majors[i] + (majors[i + 1] - majors[i]) * (float) m / (float) (k.minorPerMajor + 1));
                g.setColour (c::ink().withAlpha (0.75f));
                g.drawLine (Line<float> (ctr.getPointOnCircumference (r0, am), ctr.getPointOnCircumference (rMinor, am)), 0.7f);
            }
        const auto pt = ctr.getPointOnCircumference (R * (k.big ? 1.56f : 1.62f), a);
        const auto& txt = k.scale[i].second;
        print (g, txt, Rectangle<float> (36.0f, 13.0f).withCentre (pt), type::numeral (txt.length() > 2 ? 10.5f : 11.5f), Justification::centred,
               txt == "0" ? c::ink() : c::inkSoft());
    }
    if (String (k.caption).isEmpty())
    {
        readout (g, k.s->getTextFromValue (k.s->getValue()), Rectangle<float> (58.0f, 19.0f).withCentre ({ b.getRight() + 33.0f, ctr.y }));
        print (g, "HPF", Rectangle<float> (58.0f, 12.0f).withCentre ({ b.getRight() + 33.0f, ctr.y - 18.0f }), ui::caps (9.0f, 1), Justification::centred);
        return;
    }
    print (g, k.caption, Rectangle<float> (b.getX() - 30.0f, b.getY() - (k.big ? 26.0f : 22.0f), b.getWidth() + 60.0f, 16.0f), type::legend (k.big ? 14.5f : 12.5f), Justification::centred);
    readout (g, k.s->getTextFromValue (k.s->getValue()).replace ("  ", " "), Rectangle<float> (k.big ? 90.0f : 86.0f, 19.0f).withCentre ({ ctr.x, b.getBottom() + (k.big ? 10.0f : 5.0f) }));
}

void Face::paintCabinet (Graphics& g)
{
    const float W = (float) kTotalW, H = (float) kTotalH, s = (float) kSide, r = (float) kRail;
    auto piece = [&] (const Path& clip, std::function<void()> draw) { Graphics::ScopedSaveState ss (g); g.reduceClipRegion (clip); draw(); };
    Path left, right, top, bottom;
    left.addQuadrilateral (0, 0, s, r, s, H - r, 0, H);
    right.addQuadrilateral (W, 0, W, H, W - s, H - r, W - s, r);
    top.addQuadrilateral (0, 0, W, 0, W - s, r, s, r);
    bottom.addQuadrilateral (0, H, s, H - r, W - s, H - r, W, H);
    g.setOpacity (1.0f);
    piece (left, [&] { g.drawImage (woodLeft, Rectangle<float> (0, 0, s, H)); });
    piece (right, [&] { g.drawImage (woodRight, Rectangle<float> (W - s, 0, s, H)); });
    piece (top, [&] { g.drawImageTransformed (woodTop, AffineTransform::scale (1.0f / texScale).rotated (-MathConstants<float>::halfPi).translated (0.0f, r)); });
    piece (bottom, [&] { g.drawImageTransformed (woodBottom, AffineTransform::scale (1.0f / texScale).rotated (MathConstants<float>::halfPi).translated (W, H - r)); });

    // Shading from one light (upper left, in front) on the true shape of each board: a flat oiled face, a rounded
    // outer edge, an eased inner edge. Each edge is shaded from its own normal and clipped to its own board, so at
    // the mitres the profiles meet exactly as two real mitred mouldings do.
    const float Lx = -0.42f, Ly = -0.55f, Lz = 0.72f;
    const float Ln = std::sqrt (Lx * Lx + Ly * Ly + Lz * Lz), lx = Lx / Ln, ly = Ly / Ln, lz = Lz / Ln;
    auto tone = [&] (float nx, float ny, float nz)                       // overlay colour for a surface normal
    {
        const float k = std::max (0.0f, nx * lx + ny * ly + nz * lz) / lz;   // 1 = the flat face
        return k >= 1.0f ? Colours::white.withAlpha (std::min (0.32f, (k - 1.0f) * 0.55f))
                         : Colour (0xff0b0603).withAlpha (std::min (0.80f, (1.0f - k) * 0.85f));
    };
    // an edge of width e along one side of a board: o = the outward direction (in the plane) of that edge,
    // edge = the coordinate of the edge line, profile = quarter round (outer) or a small easing (inner)
    auto edge = [&] (const Path& clip, float ox, float oy, float at, float e, float lift)
    {
        Graphics::ScopedSaveState ss (g); g.reduceClipRegion (clip);
        constexpr int N = 20;
        for (int i = 0; i < N; ++i)
        {
            const float d0 = e * (float) i / N, d1 = e * (float) (i + 1) / N;
            auto colAt = [&] (float d) { const float t = std::pow (1.0f - d / e, lift); return tone (ox * t, oy * t, std::sqrt (std::max (0.0f, 1.0f - t * t))); };
            Rectangle<float> rr; Point<float> p0, p1;
            if (std::abs (ox) > 0.0f) { const float x0 = at - ox * d0, x1 = at - ox * d1; rr = { std::min (x0, x1), 0.0f, std::abs (x1 - x0) + 0.05f, H }; p0 = { x0, 0.0f }; p1 = { x1, 0.0f }; }
            else            { const float y0 = at - oy * d0, y1 = at - oy * d1; rr = { 0.0f, std::min (y0, y1), W, std::abs (y1 - y0) + 0.05f }; p0 = { 0.0f, y0 }; p1 = { 0.0f, y1 }; }
            g.setGradientFill (ColourGradient (colAt (d0), p0, colAt (d1), p1, false));
            g.fillRect (rr);
        }
    };
    const float eo = 7.0f, ei = 2.6f;                                   // outer round-over, inner easing
    edge (left, -1.0f, 0.0f, 0.0f, eo, 1.0f);      edge (left, 1.0f, 0.0f, s, ei, 1.6f);
    edge (right, 1.0f, 0.0f, W, eo, 1.0f);         edge (right, -1.0f, 0.0f, W - s, ei, 1.6f);
    edge (top, 0.0f, -1.0f, 0.0f, eo, 1.0f);       edge (top, 0.0f, 1.0f, r, ei, 1.6f);
    edge (bottom, 0.0f, 1.0f, H, eo, 1.0f);        edge (bottom, 0.0f, -1.0f, H - r, ei, 1.6f);
    // the oil finish: light falls off softly across the whole cabinet (one gradient, continuous over the joints),
    // and each board's sheen runs along its grain, so the joints show the gentle tone change of real mitred wood
    {
        Path all; all.addRectangle (0, 0, W, H); all.setUsingNonZeroWinding (false); all.addRectangle (s, r, W - 2 * s, H - 2 * r);
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.05f), 0.0f, 0.0f, Colour (0xff0b0603).withAlpha (0.22f), W, H, false));
        g.fillPath (all);
    }
    piece (left, [&] { ColourGradient sh (Colours::white.withAlpha (0.0f), 8.0f, 0.0f, Colours::white.withAlpha (0.0f), s - 4.0f, 0.0f, false);
                       sh.addColour (0.42, Colours::white.withAlpha (0.075f)); g.setGradientFill (sh); g.fillRect (0.0f, 0.0f, s, H); });
    piece (right, [&] { ColourGradient sh (Colours::white.withAlpha (0.0f), W - s + 4.0f, 0.0f, Colours::white.withAlpha (0.0f), W - 8.0f, 0.0f, false);
                        sh.addColour (0.55, Colours::white.withAlpha (0.035f)); g.setGradientFill (sh); g.fillRect (W - s, 0.0f, s, H); });
    piece (top, [&] { ColourGradient sh (Colours::white.withAlpha (0.0f), 0.0f, 6.0f, Colours::white.withAlpha (0.0f), 0.0f, r - 2.0f, false);
                      sh.addColour (0.45, Colours::white.withAlpha (0.06f)); g.setGradientFill (sh); g.fillRect (0.0f, 0.0f, W, r); });
    // board-to-board colour: no two planks are quite the same
    piece (right, [&] { g.setColour (Colour (0xff3a1606).withAlpha (0.06f)); g.fillRect (W - s, 0.0f, s, H); });
    piece (top, [&] { g.setColour (Colour (0xffd8a070).withAlpha (0.035f)); g.fillRect (0.0f, 0.0f, W, r); });
    piece (bottom, [&] { g.setColour (Colour (0xff1a0a04).withAlpha (0.05f)); g.fillRect (0.0f, H - r, W, r); });
    // the mitre joints: a hairline seam where two eased arrises meet - a dark core, the wall facing the light a touch
    // brighter, the other in shade; glue line and fit vary a little along the joint
    struct Seam { Point<float> a, b; };
    for (auto sm : { Seam { { 0, 0 }, { s, r } }, Seam { { W, 0 }, { W - s, r } }, Seam { { 0, H }, { s, H - r } }, Seam { { W, H }, { W - s, H - r } } })
    {
        const auto dir = (sm.b - sm.a) / sm.a.getDistanceFrom (sm.b);
        const Point<float> pn { -dir.y, dir.x };                         // perpendicular, in the plane
        const auto wallA = tone (pn.x * 0.7f, pn.y * 0.7f, 0.71f), wallB = tone (-pn.x * 0.7f, -pn.y * 0.7f, 0.71f);
        constexpr int K = 9;
        for (int i = 0; i < K; ++i)
        {
            const auto p0 = sm.a + (sm.b - sm.a) * ((float) i / K), p1 = sm.a + (sm.b - sm.a) * ((float) (i + 1) / K);
            const float j = 0.75f + 0.5f * hash2 ((int) sm.a.x + 3 * i, (int) sm.a.y + 11);
            g.setColour (wallA.withMultipliedAlpha (j)); g.drawLine (Line<float> (p0 + pn * 0.55f, p1 + pn * 0.55f), 0.55f);
            g.setColour (wallB.withMultipliedAlpha (j)); g.drawLine (Line<float> (p0 - pn * 0.55f, p1 - pn * 0.55f), 0.55f);
            g.setColour (Colour (0xff120904).withAlpha (0.62f * j));      g.drawLine (Line<float> (p0, p1), 0.5f);
        }
    }
    // the plate sits in a rebate below the wood: the rebate's lower-right faces are lit, the upper-left ones in shade
    const auto pr = Rectangle<float> (s, r, (float) kW, (float) kH);
    g.setColour (Colour (0xff0d0805));
    g.drawRect (pr.expanded (1.5f), 1.5f);
    // (the rebate's lit lower and right walls are the boards' eased inner edges above, mitred with them)
}

void Face::paintPlate (Graphics& g)
{
    const float W = (float) kW, H = (float) kH;
    g.setOpacity (1.0f);   // drawImage uses the current colour's opacity
    g.drawImage (dark ? brushedDark : brushed, Rectangle<float> (0, 0, W, H));
    // anodised name band
    g.setGradientFill (ColourGradient (Colour (0xff201e1c), 0.0f, 0.0f, Colour (0xff0d0c0b), 0.0f, 60.0f, false));
    g.fillRect (Rectangle<float> (0.0f, 0.0f, W, 60.0f));
    for (int y = 1; y < 60; y += 2)
    {
        g.setColour (Colours::white.withAlpha (0.012f + 0.01f * hash2 (y, 3)));
        g.drawHorizontalLine (y, 0.0f, W);
    }
    g.setColour (Colours::white.withAlpha (0.10f));
    g.drawHorizontalLine (0, 0.0f, W);
    g.setColour (Colours::black.withAlpha (0.7f));
    g.drawHorizontalLine (60, 0.0f, W);
    g.setColour (Colours::white.withAlpha (0.35f));
    g.drawHorizontalLine (61, 0.0f, W);
    // nameplate: the name signed by hand - "Axell" in ivory ink, "76" in gold leaf
    const Font sig = script (46.0f), num = script (50.0f);
    const float base = 44.0f, x0 = 34.0f;
    const float aw = GlyphArrangement::getStringWidth (sig, "Axell");
    auto ink = [&] (const String& t, const Font& f, float x, float y, std::function<void()> fill)
    {
        g.setFont (f);
        g.setColour (Colours::black.withAlpha (0.55f));
        g.drawSingleLineText (t, roundToInt (x + 1.0f), roundToInt (y + 2.0f));
        fill();
        g.drawSingleLineText (t, roundToInt (x), roundToInt (y));
    };
    ink ("Axell", sig, x0, base, [&] { g.setGradientFill (ColourGradient (Colour (0xfff7efdc), 0.0f, 14.0f, Colour (0xffcfc3a6), 0.0f, 52.0f, false)); });
    const float nx = x0 + aw + 12.0f;
    ink ("76", num, nx, base + 1.0f, [&] { g.setGradientFill (ColourGradient (Colour (0xffffe2a6), 0.0f, 12.0f, Colour (0xffb06f26), 0.0f, 52.0f, false)); });
    const float bw = aw + 12.0f + GlyphArrangement::getStringWidth (num, "76") - 64.0f;   // subtitles sit after the signature
    g.setColour (Colour (0xffb9b2a4));
    g.setFont (ui::caps (12.0f, 1));
    g.drawText ("FET LIMITING AMPLIFIER", Rectangle<float> (36.0f + bw + 100.0f, 16.0f, 260.0f, 14.0f), Justification::centredLeft);
    g.setColour (Colour (0xff7f786c));
    g.setFont (ui::caps (9.5f, 1));
    g.drawText (String (CharPointer_UTF8 ("SOLID STATE  \xc2\xb7  CLASS A OUTPUT")), Rectangle<float> (36.0f + bw + 100.0f, 32.0f, 260.0f, 12.0f), Justification::centredLeft);
    g.setColour (Colour (0xff8a8377));
    g.setFont (ui::caps (8.0f, 1));
    g.drawText ("PRESET", Rectangle<float> (530.0f, 25.0f, 70.0f, 12.0f), Justification::centredRight);
    g.setColour (Colour (0xffb9b2a4));
    g.setFont (ui::font (10.0f, 1));
    g.drawText (String ("v") + AXELL_VERSION_STRING, Rectangle<float> (W - 92.0f, 24.0f, 62.0f, 13.0f), Justification::centredRight);

    // warm spill from the meter's lamps onto the metal around the bezel
    {
        const auto v = geo::vu.toFloat();
        g.setGradientFill (ColourGradient (Colour (0xffffb04a).withAlpha (0.16f), v.getCentreX(), v.getCentreY() + 20.0f, Colour (0xffffb04a).withAlpha (0.0f), v.getCentreX() + v.getWidth() * 0.75f, v.getCentreY(), true));
        g.fillRect (v.expanded (60.0f, 40.0f));
    }
    // the VU's bezel stands proud of the panel
    castShadow (g, roundRectPath (geo::vu.toFloat(), 9.0f), 7.0f);
    sectionHeader (g, "GAIN", 40.0f, 430.0f, 82.0f);
    sectionHeader (g, "TIME", 444.0f, 548.0f, 82.0f);
    sectionHeader (g, "RATIO", 566.0f, 652.0f, 82.0f);
    sectionHeader (g, "METER", 666.0f, 960.0f, 82.0f);
    for (float x : { 437.0f, 557.0f, 659.0f })
    {
        g.setColour (c::ink().withAlpha (0.28f));
        g.drawLine (x, 96.0f, x, 432.0f, 0.8f);
        g.setColour (c::inkHi());
        g.drawLine (x + 1.0f, 96.0f, x + 1.0f, 432.0f, 0.8f);
    }
    for (auto& k : specs) paintKnobLegend (g, k);
    for (int i = 0; i < 5; ++i)
    {
        const float y = (float) (geo::ratioY + i * (geo::ratioH + geo::ratioGap)) + (float) geo::ratioH * 0.5f;
        const Point<float> lp { (float) geo::ratioX + (float) geo::ratioW + 13.0f, y };
        const bool on = i == ratioSel;
        castShadow (g, ellipsePath (Rectangle<float> (10.5f, 10.5f).withCentre (lp)), 2.0f);
        g.setGradientFill (ColourGradient (Colour (0xffe9e6e0), lp.x - 5, lp.y - 5, Colour (0xff57544f), lp.x + 5, lp.y + 5, false));
        g.fillEllipse (Rectangle<float> (10.5f, 10.5f).withCentre (lp));
        if (on)
        {
            g.setGradientFill (ColourGradient (c::amber.withAlpha (0.55f), lp.x, lp.y, c::amber.withAlpha (0.0f), lp.x + 15.0f, lp.y, true));
            g.fillEllipse (Rectangle<float> (30.0f, 30.0f).withCentre (lp));
        }
        g.setGradientFill (ColourGradient (on ? Colour (0xffffe3a6) : Colour (0xff4a3a22), lp.x - 1.5f, lp.y - 2.0f, on ? Colour (0xffe08a1e) : Colour (0xff1b140b), lp.x + 3.0f, lp.y + 3.0f, true));
        g.fillEllipse (Rectangle<float> (6.5f, 6.5f).withCentre (lp));
    }
    const auto sw = geo::inOut.toFloat();
    print (g, "IN", { sw.getX() - 10.0f, sw.getY() - 19.0f, sw.getWidth() + 20.0f, 15.0f }, type::legend (12.0f), Justification::centred);
    print (g, "OUT", { sw.getX() - 10.0f, sw.getBottom() + 2.0f, sw.getWidth() + 20.0f, 15.0f }, type::legend (12.0f), Justification::centred);
    const Point<float> lamp { geo::lampX, geo::lampY };
    castShadow (g, ellipsePath (Rectangle<float> (26, 26).withCentre (lamp)), 4.0f);
    g.setGradientFill (ColourGradient (Colour (0xffeeebe5), lamp.x - 12, lamp.y - 12, Colour (0xff57544f), lamp.x + 12, lamp.y + 12, false));
    g.fillEllipse (Rectangle<float> (26, 26).withCentre (lamp));
    if (active)
    {
        g.setGradientFill (ColourGradient (c::lampRed.withAlpha (0.5f), lamp.x, lamp.y, c::lampRed.withAlpha (0.0f), lamp.x + 34.0f, lamp.y, true));
        g.fillEllipse (Rectangle<float> (68, 68).withCentre (lamp));
    }
    g.setGradientFill (ColourGradient (active ? Colour (0xffffb7a8) : Colour (0xff6a2a24), lamp.x - 4, lamp.y - 5, active ? c::lampRed : Colour (0xff2a0d0b), lamp.x + 6, lamp.y + 7, true));
    g.fillEllipse (Rectangle<float> (17, 17).withCentre (lamp));
    g.setColour (Colours::black.withAlpha (0.25f));
    for (int k = 0; k < 8; ++k)
        g.drawLine (Line<float> (lamp, lamp.getPointOnCircumference (8.0f, (float) k * MathConstants<float>::pi / 4.0f)), 0.5f);
    g.setColour (Colours::white.withAlpha (0.6f));
    g.fillEllipse (Rectangle<float> (5.0f, 3.5f).withCentre (lamp.translated (-3.0f, -4.0f)));
    print (g, "PILOT", { lamp.x - 30.0f, lamp.y + 17.0f, 60.0f, 12.0f }, type::header(), Justification::centred, c::inkSoft());
    print (g, String (CharPointer_UTF8 ("SER. No. 0076  \xc2\xb7  MADE ON LINUX")), { 40.0f, 447.0f, 300.0f, 12.0f }, ui::caps (9.0f, 1), Justification::centredLeft, c::inkSoft().withAlpha (0.8f));
    // the utility strip
    sectionHeader (g, "SIDE-CHAIN", 40.0f, 300.0f, 344.0f);
    sectionHeader (g, "QUALITY", 312.0f, 430.0f, 344.0f);
    {
        const auto e = extSw.getBounds().toFloat();
        print (g, "INT", { e.getX() - 40.0f, e.getY(), 34.0f, e.getHeight() }, ui::caps (9.0f, 1), Justification::centredRight, extSw.dark ? c::inkSoft() : c::ink());
        print (g, "EXT", { e.getRight() + 6.0f, e.getY(), 34.0f, e.getHeight() }, ui::caps (9.0f, 1), Justification::centredLeft, extSw.dark ? c::ink() : c::inkSoft());
        if (extSw.dark && ! proc.scConnected.load())
            print (g, "NO INPUT", { e.getX() - 30.0f, e.getBottom() + 3.0f, e.getWidth() + 60.0f, 11.0f }, ui::caps (7.5f, 1), Justification::centred, Colour (0xffb3241a));
    }
    {
        const auto sk = skin.getBounds().toFloat();
        print (g, "SILVER", { sk.getX() - 60.0f, sk.getY(), 54.0f, sk.getHeight() }, ui::caps (8.5f, 1), Justification::centredRight, dark ? c::inkSoft() : c::ink());
        print (g, "BLACK", { sk.getRight() + 6.0f, sk.getY(), 54.0f, sk.getHeight() }, ui::caps (8.5f, 1), Justification::centredLeft, dark ? c::ink() : c::inkSoft());
    }
    for (auto pt : { Point<float> (18.0f, 78.0f), Point<float> (W - 18.0f, 78.0f), Point<float> (18.0f, H - 18.0f), Point<float> (W - 18.0f, H - 18.0f) })
        screw (g, pt, 6.5f, 0.3f + pt.x * 0.011f + pt.y * 0.017f);
    g.setColour (Colours::white.withAlpha (0.35f));
    g.drawRect (Rectangle<float> (0.5f, 61.5f, W - 1.0f, H - 62.0f), 0.8f);
    // the walnut frame stands above the plate: its top and left edges throw a soft shadow onto the metal
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.42f), 0.0f, 0.0f, Colours::transparentBlack, 0.0f, 14.0f, false));
    g.fillRect (Rectangle<float> (0, 0, W, 14.0f));
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.36f), 0.0f, 0.0f, Colours::transparentBlack, 11.0f, 0.0f, false));
    g.fillRect (Rectangle<float> (0, 0, 11.0f, H));
    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.10f), W - 4.0f, 0.0f, Colours::transparentBlack, W, 0.0f, false));
    g.fillRect (Rectangle<float> (W - 4.0f, 0, 4.0f, H));
}

void SkinSwitch::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    castShadow (g, roundRectPath (r, r.getHeight() * 0.5f), 1.5f, 0.7f);
    g.setGradientFill (ColourGradient (Colour (0xffe9e6e0), r.getX(), r.getY(), Colour (0xff5a5752), r.getRight(), r.getBottom(), false));
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    auto slot = r.reduced (3.0f);
    g.setColour (Colour (0xff0e0d0c));
    g.fillRoundedRectangle (slot, slot.getHeight() * 0.5f);
    innerShadow (g, roundRectPath (slot, slot.getHeight() * 0.5f), 2.0f);
    const float w = slot.getWidth() * 0.5f;
    auto knob = Rectangle<float> (slot.getX() + (dark ? w : 0.0f), slot.getY(), w, slot.getHeight()).reduced (0.5f);
    castShadow (g, roundRectPath (knob, 2.0f), 1.5f);
    g.setGradientFill (ColourGradient (Colour (0xfffaf9f6), knob.getX(), knob.getY(), Colour (0xff77746e), knob.getX(), knob.getBottom(), false));
    g.fillRoundedRectangle (knob, 2.0f);
    g.setColour (Colours::black.withAlpha (0.35f));
    for (int k = 1; k <= 3; ++k)
    {
        const float x = knob.getX() + knob.getWidth() * (float) k / 4.0f;
        g.drawLine (x, knob.getY() + 2.0f, x, knob.getBottom() - 2.0f, 0.7f);
    }
    g.setColour (Colours::black.withAlpha (0.5f));
    g.drawRoundedRectangle (knob, 2.0f, 0.6f);
}

void Face::buildTextures (float ts)
{
    const float tk = ts / 2.0f;
    texScale = ts;
    woodLeft = makeWalnut ((int) (kSide * ts), (int) (kTotalH * ts), 7, tk);
    woodRight = makeWalnut ((int) (kSide * ts), (int) (kTotalH * ts), 41, tk);
    woodTop = makeWalnut ((int) (kRail * ts), (int) (kTotalW * ts), 23, tk);
    woodBottom = makeWalnut ((int) (kRail * ts), (int) (kTotalW * ts), 59, tk);
    brushed = makeBrushed ((int) (kW * ts), (int) (kH * ts), 11, 172.0f, 150.0f, tk);
    brushedDark = makeBrushed ((int) (kW * ts), (int) (kH * ts), 11, 52.0f, 38.0f, tk);   // black anodised, still brushed
}

void Face::paint (Graphics& g)
{
    c::darkSkin = dark;
    {
        // textures follow the pixel density the panel is drawn at (HiDPI, enlarged window): if they are coarser than
        // the screen, rebuild them after this paint (never inside it) and repaint
        const float px = g.getInternalContext().getPhysicalPixelScaleFactor();
        const float want = std::min (std::ceil (px * 2.0f) / 2.0f, vintage::kMaxTextureScale);
        if (want > texScale + 0.01f && ! rebuildPending)
        {
            rebuildPending = true;
            juce::Component::SafePointer<Face> self (this);
            juce::MessageManager::callAsync ([self, want] { if (self != nullptr) { self->buildTextures (want); self->rebuildPending = false; self->repaint(); } });
        }
    }
    paintCabinet (g);
    Graphics::ScopedSaveState ss (g);
    g.addTransform (AffineTransform::translation ((float) kSide, (float) kRail));
    g.reduceClipRegion (Rectangle<int> (0, 0, kW, kH));
    paintPlate (g);
}

void Face::paintOverChildren (Graphics& g)
{
    // drama, kept subtle: one key light from the top left, the far corner falling away
    const auto r = getLocalBounds().toFloat();
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.07f), r.getX() + r.getWidth() * 0.18f, r.getY(), Colours::transparentWhite, r.getX() + r.getWidth() * 0.55f, r.getY() + r.getHeight() * 0.7f, true));
    g.fillRect (r);
    g.setGradientFill (ColourGradient (Colours::transparentBlack, r.getX() + r.getWidth() * 0.35f, r.getY() + r.getHeight() * 0.3f, Colours::black.withAlpha (0.34f), r.getRight(), r.getBottom(), true));
    g.fillRect (r);
}

void Face::showPresetMenu()
{
    // grouped by what the preset is for; within a group in table order (the 2.0 presets keep their numbers)
    PopupMenu m;
    const int cur = proc.getCurrentProgram();
    for (int i = 0; i < axell76::kNumPresets; ++i)
        if (juce::String (axell76::kPresets[i].group).isEmpty()) m.addItem (i + 1, axell76::kPresets[i].name, true, i == cur);
    for (const char* g : { "Signature", "Vocal", "Beat", "Pocket", "Drums", "Bass", "Mix Bus" })
    {
        PopupMenu sub; bool any = false, has = false;
        for (int i = 0; i < axell76::kNumPresets; ++i)
            if (juce::String (axell76::kPresets[i].group) == g) { sub.addItem (i + 1, axell76::kPresets[i].name, true, i == cur); any = true; has |= i == cur; }
        if (any) m.addSubMenu (juce::String (g) + (juce::String (g) == "Pocket" ? "  (vocal -> side-chain)" : ""), sub, true, nullptr, has);
    }
    m.showMenuAsync (PopupMenu::Options().withTargetComponent (presetName), [this] (int r) { if (r > 0) { proc.setCurrentProgram (r - 1); repaint(); } });
}

void Face::tick()
{
    const float ms = proc.outMs.load();
    vu.setValues (proc.grMeter.load(), 10.0f * std::log10 (ms + 1.0e-12f), meterSel);
    const String pn = proc.getProgramName (proc.getCurrentProgram());
    if (presetName.getButtonText() != pn) presetName.setButtonText (pn);
}
} // namespace vintage

//==============================================================================
A76Editor::A76Editor (A76Processor& p) : AudioProcessorEditor (&p), proc (p), face (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (face);
    setResizable (true, true);
    setResizeLimits (vintage::kTotalW * 3 / 5, vintage::kTotalH * 3 / 5, vintage::kTotalW * 2, vintage::kTotalH * 2);
    getConstrainer()->setFixedAspectRatio ((double) vintage::kTotalW / vintage::kTotalH);
    setSize (vintage::kTotalW * 7 / 10, vintage::kTotalH * 7 / 10);   // compact: 750 x 340 by default
    startTimerHz (40);
    axell::nameControlsForScreenReaders (*this);   // Narrator / VoiceOver read every control by name
}

A76Editor::~A76Editor() { setLookAndFeel (nullptr); }

void A76Editor::paint (Graphics& g) { g.fillAll (Colours::black); }

void A76Editor::resized() { face.setTransform (AffineTransform::scale ((float) getWidth() / (float) vintage::kTotalW)); }

void A76Editor::timerCallback() { face.tick(); }
