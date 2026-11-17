#pragma once
#include "PluginProcessor.h"

class MeuProQAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MeuProQAudioProcessorEditor(MeuProQAudioProcessor&);
    ~MeuProQAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override {}

    // Interação com o mouse (Arrastar nó e Scroll para Q)
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

private:
    void timerCallback() override { repaint(); }
    
    // Conversões matemáticas Tela <-> Áudio
    float freqToX(float freq) const;
    float xToFreq(float x) const;
    float gainToY(float gain) const;
    float yToGain(float y) const;

    MeuProQAudioProcessor& audioProcessor;
    bool isDraggingNode = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeuProQAudioProcessorEditor)
};
