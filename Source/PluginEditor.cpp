#include "PluginEditor.h"

MeuProQAudioProcessorEditor::MeuProQAudioProcessorEditor(MeuProQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(900, 450); // Proporção panorâmica estilo Pro-Q
    startTimerHz(60);   // Atualização a 60 FPS
}

float MeuProQAudioProcessorEditor::freqToX(float freq) const
{
    return juce::mapFromLog10(freq, 20.0f, 20000.0f) * getWidth();
}

float MeuProQAudioProcessorEditor::xToFreq(float x) const
{
    float norm = juce::jlimit(0.0f, 1.0f, x / (float)getWidth());
    return juce::mapToLog10(norm, 20.0f, 20000.0f);
}

float MeuProQAudioProcessorEditor::gainToY(float gain) const
{
    return juce::jmap(gain, -24.0f, 24.0f, (float)getHeight(), 0.0f);
}

float MeuProQAudioProcessorEditor::yToGain(float y) const
{
    return juce::jlimit(-24.0f, 24.0f, juce::jmap(y, (float)getHeight(), 0.0f, -24.0f, 24.0f));
}

void MeuProQAudioProcessorEditor::paint(juce::Graphics& g)
{
    // 1. Fundo escuro característico do Pro-Q
    g.fillAll(juce::Colour(0xff141619));

    // 2. Grade de Frequências e Ganho
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    float freqs[] = { 30, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 15000 };
    for (float f : freqs)
    {
        float x = freqToX(f);
        g.drawVerticalLine((int)x, 0.0f, (float)getHeight());
    }

    float gains[] = { -18, -12, -6, 0, 6, 12, 18 };
    for (float db : gains)
    {
        float y = gainToY(db);
        g.setColour(db == 0 ? juce::Colours::white.withAlpha(0.25f) : juce::Colours::white.withAlpha(0.08f));
        g.drawHorizontalLine((int)y, 0.0f, (float)getWidth());
    }

    // 3. Cálculo e Desenho da Curva de Resposta do Filtro
    float freq = audioProcessor.apvts.getRawParameterValue("FREQ_1")->load();
    float gain = audioProcessor.apvts.getRawParameterValue("GAIN_1")->load();
    float q = audioProcessor.apvts.getRawParameterValue("Q_1")->load();

    auto coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        44100.0, freq, q, juce::Decibels::decibelsToGain(gain));

    juce::Path responseCurve;
    for (int x = 0; x < getWidth(); ++x)
    {
        double f = xToFreq((float)x);
        double mag = coeffs->getMagnitudeForFrequency(f, 44100.0);
        float db = juce::Decibels::gainToDecibels((float)mag);
        float y = gainToY(db);

        if (x == 0) responseCurve.startNewSubPath((float)x, y);
        else responseCurve.lineTo((float)x, y);
    }

    // Preenchimento translúcido sob a curva + Linha brilhante amarela/dourada
    g.setColour(juce::Colour(0xfff5b041).withAlpha(0.15f));
    juce::Path fillPath = responseCurve;
    fillPath.lineTo((float)getWidth(), gainToY(0.0f));
    fillPath.lineTo(0.0f, gainToY(0.0f));
    fillPath.closeSubPath();
    g.fillPath(fillPath);

    g.setColour(juce::Colour(0xfff5b041));
    g.strokePath(responseCurve, juce::PathStrokeType(2.2f));

    // 4. Desenho do Nó (Bolinha interativa)
    float nodeX = freqToX(freq);
    float nodeY = gainToY(gain);

    g.setColour(juce::Colour(0xfff5b041));
    g.fillEllipse(nodeX - 7.0f, nodeY - 7.0f, 14.0f, 14.0f);
    g.setColour(juce::Colours::white);
    g.drawEllipse(nodeX - 7.0f, nodeY - 7.0f, 14.0f, 14.0f, 1.5f);
}

void MeuProQAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    float freq = audioProcessor.apvts.getRawParameterValue("FREQ_1")->load();
    float gain = audioProcessor.apvts.getRawParameterValue("GAIN_1")->load();
    juce::Point<float> nodePos(freqToX(freq), gainToY(gain));

    if (event.position.getDistanceFrom(nodePos) < 20.0f)
        isDraggingNode = true;
}

void MeuProQAudioProcessorEditor::mouseDrag(const juce::MouseEvent& event)
{
    if (!isDraggingNode) return;

    float newFreq = xToFreq(event.position.x);
    float newGain = yToGain(event.position.y);

    if (auto* pFreq = audioProcessor.apvts.getParameter("FREQ_1"))
        pFreq->setValueNotifyingHost(pFreq->convertTo0to1(newFreq));

    if (auto* pGain = audioProcessor.apvts.getParameter("GAIN_1"))
        pGain->setValueNotifyingHost(pGain->convertTo0to1(newGain));
}

void MeuProQAudioProcessorEditor::mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    float currentQ = audioProcessor.apvts.getRawParameterValue("Q_1")->load();
    float newQ = juce::jlimit(0.1f, 10.0f, currentQ + wheel.deltaY * 2.0f);

    if (auto* pQ = audioProcessor.apvts.getParameter("Q_1"))
        pQ->setValueNotifyingHost(pQ->convertTo0to1(newQ));
}
