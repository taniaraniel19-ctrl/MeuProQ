#include "PluginProcessor.h"
#include "PluginEditor.h"

MeuProQAudioProcessor::MeuProQAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout MeuProQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "FREQ_1", "Frequencia",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 1000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "GAIN_1", "Ganho",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Q_1", "Fator Q",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 0.5f), 1.0f));

    return layout;
}

void MeuProQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    band1Filter.prepare(spec);
}

void MeuProQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    float freq = apvts.getRawParameterValue("FREQ_1")->load();
    float gainDb = apvts.getRawParameterValue("GAIN_1")->load();
    float q = apvts.getRawParameterValue("Q_1")->load();

    // Atualiza os coeficientes do filtro Peak (Bell)
    *band1Filter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, freq, q, juce::Decibels::decibelsToGain(gainDb));

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    band1Filter.process(context);
}

void MeuProQAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MeuProQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* MeuProQAudioProcessor::createEditor()
{
    return new MeuProQAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MeuProQAudioProcessor();
}
