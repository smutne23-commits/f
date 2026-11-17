#pragma once
#include "PluginProcessor.h"

// Minimal dark look: black with a red accent and a monospace typeface.
class RageLook : public juce::LookAndFeel_V4
{
public:
    static inline const juce::Colour accent { 0xffff2b2b };

    static juce::Font mono (float h, bool bold = false)
    {
        auto o = juce::FontOptions().withName (juce::Font::getDefaultMonospacedFontName()).withHeight (h);
        if (bold) o = o.withStyle ("Bold");
        return juce::Font (o);
    }

    RageLook()
    {
        setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId, juce::Colours::white);
    }

    juce::Font getLabelFont (juce::Label&) override { return mono (12.0f); }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startA, float endA, juce::Slider&) override
    {
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (10.0f);
        const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
        const auto c = b.getCentre();
        const float ang = startA + pos * (endA - startA);

        juce::Path track, val;
        track.addCentredArc (c.x, c.y, r, r, 0.0f, startA, endA, true);
        val.addCentredArc   (c.x, c.y, r, r, 0.0f, startA, ang,  true);

        g.setColour (juce::Colour (0xff262626));
        g.strokePath (track, juce::PathStrokeType (2.0f));
        g.setColour (accent);
        g.strokePath (val, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));

        // Thin white indicator.
        const float d0 = r * 0.30f, d1 = r * 0.78f;
        g.setColour (juce::Colours::white);
        g.drawLine (c.x + std::sin (ang) * d0, c.y - std::cos (ang) * d0,
                    c.x + std::sin (ang) * d1, c.y - std::cos (ang) * d1, 2.0f);

        // Centre dot.
        g.fillEllipse (c.x - 2.0f, c.y - 2.0f, 4.0f, 4.0f);
    }
};

class RageChorusEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RageChorusEditor (RageChorusProcessor&);
    ~RageChorusEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawWave (juce::Graphics&, juce::Rectangle<float>);
    void drawTicker (juce::Graphics&, juce::Rectangle<float>);
    void drawCross (juce::Graphics&, float x, float y, float s);

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    void addKnob (Knob& k, const juce::String& id, const juce::String& name, const juce::String& suffix);

    RageChorusProcessor& proc;
    RageLook look;
    Knob rate, depth, delay, feedback, mix;
    juce::Rectangle<int> headerArea, waveArea, footerArea;
    float tickerOffset = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RageChorusEditor)
};
