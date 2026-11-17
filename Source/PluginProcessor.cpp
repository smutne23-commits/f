#include "PluginProcessor.h"
#include "PluginEditor.h"

RageChorusProcessor::RageChorusProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout RageChorusProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout l;

    l.add (std::make_unique<P> (juce::ParameterID { "rate", 1 },     "Rate",     R (0.1f, 10.0f, 0.01f, 0.4f), 1.5f));
    l.add (std::make_unique<P> (juce::ParameterID { "depth", 1 },    "Depth",    R (0.0f, 1.0f, 0.001f),       0.5f));
    l.add (std::make_unique<P> (juce::ParameterID { "delay", 1 },    "Delay",    R (1.0f, 100.0f, 0.1f, 0.5f), 12.0f));
    l.add (std::make_unique<P> (juce::ParameterID { "feedback", 1 }, "Feedback", R (-0.95f, 0.95f, 0.001f),    0.0f));
    l.add (std::make_unique<P> (juce::ParameterID { "mix", 1 },      "Mix",      R (0.0f, 1.0f, 0.001f),       0.5f));
    return l;
}

void RageChorusProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec { sampleRate,
                                  (juce::uint32) samplesPerBlock,
                                  (juce::uint32) juce::jmax (1, getTotalNumOutputChannels()) };
    chorus.prepare (spec);
    chorus.reset();
}

bool RageChorusProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void RageChorusProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    chorus.setRate        (*apvts.getRawParameterValue ("rate"));
    chorus.setDepth       (*apvts.getRawParameterValue ("depth"));
    chorus.setCentreDelay (*apvts.getRawParameterValue ("delay"));
    chorus.setFeedback    (*apvts.getRawParameterValue ("feedback"));
    chorus.setMix         (*apvts.getRawParameterValue ("mix"));

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    chorus.process (ctx);
}

juce::AudioProcessorEditor* RageChorusProcessor::createEditor()
{
    return new RageChorusEditor (*this);
}

void RageChorusProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void RageChorusProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RageChorusProcessor();
}
