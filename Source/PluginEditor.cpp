#include "PluginEditor.h"

RageChorusEditor::RageChorusEditor (RageChorusProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&look);

    addKnob (rate,     "rate",     "RATE",  " Hz");
    addKnob (depth,    "depth",    "DEPTH", "");
    addKnob (delay,    "delay",    "DELAY", " ms");
    addKnob (feedback, "feedback", "FDBK",  "");
    addKnob (mix,      "mix",      "MIX",   "");

    setSize (580, 360);
    startTimerHz (30);
}

RageChorusEditor::~RageChorusEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void RageChorusEditor::addKnob (Knob& k, const juce::String& id, const juce::String& name, const juce::String& suffix)
{
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 18);
    k.slider.setTextValueSuffix (suffix);
    k.slider.setNumDecimalPlacesToDisplay (2);
    k.slider.setDoubleClickReturnValue (true, proc.apvts.getParameter (id)->convertFrom0to1 (
                                                  proc.apvts.getParameter (id)->getDefaultValue()));
    addAndMakeVisible (k.slider);

    k.label.setText (name, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setFont (RageLook::mono (12.0f, true));
    k.label.setColour (juce::Label::textColourId, juce::Colour (0xff9a9a9a));
    k.label.attachToComponent (&k.slider, false);
    addAndMakeVisible (k.label);

    k.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k.slider);
}

void RageChorusEditor::resized()
{
    auto r = getLocalBounds().reduced (20);
    headerArea = r.removeFromTop (50);
    footerArea = r.removeFromBottom (22);
    r.removeFromTop (6);
    waveArea = r.removeFromTop (96);
    r.removeFromTop (6);

    auto row = r.withTrimmedTop (22);
    const int w = row.getWidth() / 5;
    for (auto* k : { &rate, &depth, &delay, &feedback, &mix })
        k->slider.setBounds (row.removeFromLeft (w));
}

void RageChorusEditor::timerCallback()
{
    tickerOffset += 0.9f;
    repaint (waveArea);
    repaint (footerArea);
}

void RageChorusEditor::drawCross (juce::Graphics& g, float x, float y, float s)
{
    g.drawLine (x - s, y - s, x + s, y + s, 1.0f);
    g.drawLine (x - s, y + s, x + s, y - s, 1.0f);
}

void RageChorusEditor::drawWave (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour (juce::Colour (0xff0b0b0b));
    g.fillRect (r);
    g.setColour (juce::Colour (0xff222222));
    g.drawRect (r, 1.0f);

    const float rt = *proc.apvts.getRawParameterValue ("rate");
    const float dp = *proc.apvts.getRawParameterValue ("depth");
    const float mx = *proc.apvts.getRawParameterValue ("mix");
    const float t  = (float) (juce::Time::getMillisecondCounterHiRes() * 0.001);

    juce::Path dry, wet;
    for (float x = 0.0f; x <= r.getWidth(); x += 2.0f)
    {
        const float ph = x * 0.045f + t * 2.0f;
        const float yd = std::sin (ph);
        const float yw = std::sin (ph + dp * 2.5f * std::sin (t * juce::MathConstants<float>::twoPi * rt * 0.5f));
        const float cy = r.getCentreY();
        const float amp = r.getHeight() * 0.36f;

        if (x == 0.0f) { dry.startNewSubPath (r.getX() + x, cy - yd * amp);
                         wet.startNewSubPath (r.getX() + x, cy - yw * amp); }
        else           { dry.lineTo (r.getX() + x, cy - yd * amp);
                         wet.lineTo (r.getX() + x, cy - yw * amp); }
    }

    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.strokePath (dry, juce::PathStrokeType (1.2f));
    g.setColour (RageLook::accent.withAlpha (0.25f + 0.75f * mx));
    g.strokePath (wet, juce::PathStrokeType (1.0f + 2.0f * mx));

    // X markers in the corners.
    g.setColour (juce::Colour (0xff555555));
    drawCross (g, r.getX() + 10, r.getY() + 10, 3.0f);
    drawCross (g, r.getRight() - 10, r.getY() + 10, 3.0f);
    drawCross (g, r.getX() + 10, r.getBottom() - 10, 3.0f);
    drawCross (g, r.getRight() - 10, r.getBottom() - 10, 3.0f);
}

void RageChorusEditor::drawTicker (juce::Graphics& g, juce::Rectangle<float> r)
{
    const juce::String text = "WIDE  //  DETUNED  //  LOUD  //  RAGE CHORUS  //  ";
    auto font = RageLook::mono (11.0f, true);

    juce::GlyphArrangement ga;
    ga.addLineOfText (font, text, 0.0f, 0.0f);
    const float tw = ga.getBoundingBox (0, -1, true).getWidth();

    g.saveState();
    g.reduceClipRegion (r.toNearestInt());
    g.setFont (font);
    g.setColour (juce::Colour (0xff6a6a6a));
    const float off = std::fmod (tickerOffset, tw);
    for (float x = -off; x < r.getWidth(); x += tw)
        g.drawText (text, juce::Rectangle<float> (r.getX() + x, r.getY(), tw + 4.0f, r.getHeight()),
                    juce::Justification::centredLeft, false);
    g.restoreState();
}

void RageChorusEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff050505));

    // Subtle background grain.
    juce::Random rnd (666);
    g.setColour (juce::Colours::white.withAlpha (0.035f));
    for (int i = 0; i < 400; ++i)
        g.fillRect (rnd.nextInt (getWidth()), rnd.nextInt (getHeight()), 1, 1);

    // Header.
    auto h = headerArea.toFloat();
    g.setFont (RageLook::mono (30.0f, true));
    g.setColour (juce::Colours::white);
    g.drawText ("RAGE", h.removeFromLeft (92.0f), juce::Justification::centredLeft);
    g.setColour (RageLook::accent);
    g.drawText ("CHORUS", h.removeFromLeft (150.0f), juce::Justification::centredLeft);

    g.setFont (RageLook::mono (11.0f));
    g.setColour (juce::Colour (0xff6a6a6a));
    g.drawText ("[ v1.0 ]", headerArea, juce::Justification::centredRight);

    g.setColour (RageLook::accent);
    g.fillRect (headerArea.getX(), headerArea.getBottom() - 2, 36, 2);

    drawWave (g, waveArea.toFloat());
    drawTicker (g, footerArea.toFloat());
}
