#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

// DSP Components
#include "../Utils/ParameterSmoother.h"
#include "../Utils/DSPUtils.h"
#include "../Filters/SchroederAllpassChain.h"
#include "../Filters/EQFilters.h"
#include "../Effects/StereoEnhancer.h"
#include "../Effects/MakeItLoud.h"

namespace DSP {
namespace Core {

template<typename SampleType>
class ChasmDSPProcessor
{
public:
    ChasmDSPProcessor()
    {
        using FilterType = juce::dsp::StateVariableTPTFilterType;

        lowCutFilter.setType(FilterType::highpass);
        highCutFilter.setType(FilterType::lowpass);

        lowCutFilter.setResonance(static_cast<SampleType>(0.707));
        highCutFilter.setResonance(static_cast<SampleType>(0.707));
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        samplesPerBlock = static_cast<int>(spec.maximumBlockSize);
        numChannels = static_cast<int>(spec.numChannels);

        leftAllpassChain.prepare(sampleRate);
        rightAllpassChain.prepare(sampleRate);
        brightnessEQ.prepare(spec);

        stereoEnhancer.setWidth(SampleType{100.0});

        prepareParameterSmoothers();

        wetBuffer.setSize(numChannels, samplesPerBlock);
        dryBuffer.setSize(numChannels, samplesPerBlock);

        lowCutFilter.prepare(spec);
        lowCutFilter.reset();

        highCutFilter.prepare(spec);
        highCutFilter.reset();

        makeItLoud.prepare(spec);

        reset();
    }

    void updateParameters(SampleType inputGainDb, SampleType outputGainDb, SampleType mixPercent,
                          SampleType delayMs, SampleType brightnessDb, SampleType characterQ,
                          SampleType lowCutPercent, SampleType highCutPercent, SampleType widthPercent,
                        SampleType mil_InputGain, SampleType mil_BoostValue)
    {
        inputGainSmoother.setTargetValue(Utils::DSPUtils::dbToGain(inputGainDb));
        outputGainSmoother.setTargetValue(Utils::DSPUtils::dbToGain(outputGainDb));
        mixSmoother.setTargetValue(Utils::DSPUtils::percentageToNormalized(mixPercent));
        delaySmoother.setTargetValue(delayMs);
        brightnessSmoother.setTargetValue(brightnessDb);
        characterSmoother.setTargetValue(characterQ);
        lowCutSmoother.setTargetValue(lowCutPercent);
        highCutSmoother.setTargetValue(highCutPercent);
        widthSmoother.setTargetValue(widthPercent);

        mil_BoostSmoother.setTargetValue(Utils::DSPUtils::dbToGain(mil_BoostValue));
        mil_InputGainSmoother.setTargetValue(Utils::DSPUtils::dbToGain(mil_InputGain));

    }

    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        jassert(buffer.getNumChannels() >= 1);

        const int numSamples = buffer.getNumSamples();

        if (wetBuffer.getNumSamples() != numSamples)
        {
            wetBuffer.setSize(numChannels, numSamples, false, false, true);
            dryBuffer.setSize(numChannels, numSamples, false, false, true);
        }

        dryBuffer.makeCopyOf(buffer);

        for (int i = 0; i < numSamples; ++i)
        {
            const auto inputGain = inputGainSmoother.getNextValue();
            const auto delay = delaySmoother.getNextValue();
            const auto brightness = brightnessSmoother.getNextValue();
            const auto character = characterSmoother.getNextValue();
            const auto width = widthSmoother.getNextValue();
            
            const auto lowCutFreq = lowCutSmoother.getNextValue();
            if (!juce::approximatelyEqual(lowCutFreq, lastLowCut)) {
                lowCutFilter.setCutoffFrequency(lowCutFreq);
                lowCutActive = lowCutFreq > SampleType{1.0};
                lastLowCut = lowCutFreq;
            }
            
            const auto highCutFreq = highCutSmoother.getNextValue();
            if (!juce::approximatelyEqual(highCutFreq, lastHighCut)) {
                highCutFilter.setCutoffFrequency(highCutFreq);
                highCutActive = highCutFreq < SampleType{19999.0};
                lastHighCut = highCutFreq;
            }
            
            if (i == 0 || shouldUpdateDSPComponents(i))
            updateDSPComponents(delay, brightness, character, width);
            
            processSingleSample(buffer, i, inputGain);
        }
        
        // After allpasschains to tr regain some high end.
        brightnessEQ.processBlock(wetBuffer);

        // Apply stereo enhancement
        // TODO: Finalize Stereo Enhancer implementation

        // Apply MakeItLoud effect
        makeItLoud.setInputGain(mil_InputGainSmoother.getNextValue());
        makeItLoud.setBoostValue(mil_BoostSmoother.getNextValue());
        makeItLoud.processBlock(wetBuffer);
        
        for (int i = 0; i < numSamples; ++i)
        {
            const auto mix = mixSmoother.getCurrentValue();
            const auto outputGain = outputGainSmoother.getCurrentValue();
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                const auto drySample = dryBuffer.getSample(channel, i);
                const auto wetSample = wetBuffer.getSample(channel, i);
                channelData[i] = (drySample * (SampleType{1.0} - mix) + wetSample * mix) * outputGain;
            }
        }

    }

    void reset()
    {
        leftAllpassChain.reset();
        rightAllpassChain.reset();
        brightnessEQ.reset();
        stereoEnhancer.reset();

        inputGainSmoother.reset(SampleType{1.0});
        outputGainSmoother.reset(SampleType{1.0});
        mixSmoother.reset(SampleType{0.5});
        delaySmoother.reset(SampleType{30.0});
        brightnessSmoother.reset(SampleType{0.0});
        characterSmoother.reset(SampleType{1.0});
        lowCutSmoother.reset(SampleType{0.0});
        highCutSmoother.reset(SampleType{0.0});
        widthSmoother.reset(SampleType{100.0});

        mil_BoostSmoother.reset(SampleType{0.0});
        mil_InputGainSmoother.reset(SampleType{1.0});
    }

private:
    void prepareParameterSmoothers()
    {
        inputGainSmoother.prepare(sampleRate, 1.0);
        outputGainSmoother.prepare(sampleRate, 1.0);
        mixSmoother.prepare(sampleRate, 5.0);
        delaySmoother.prepare(sampleRate, 5.0);
        brightnessSmoother.prepare(sampleRate, 5.0);
        characterSmoother.prepare(sampleRate, 5.0);
        lowCutSmoother.prepare(sampleRate, 5.0);
        highCutSmoother.prepare(sampleRate, 5.0);
        widthSmoother.prepare(sampleRate, 5.0);

        mil_BoostSmoother.prepare(sampleRate, 5.0);
        mil_InputGainSmoother.prepare(sampleRate, 5.0);
    }

    bool shouldUpdateDSPComponents(int sampleIndex) { return (sampleIndex % 32) == 0; }

    void updateDSPComponents(SampleType delay, SampleType brightness, SampleType character, SampleType width)
    {
        leftAllpassChain.setDelayTime(delay);
        rightAllpassChain.setDelayTime(delay);
        leftAllpassChain.setCharacter(character);
        rightAllpassChain.setCharacter(character);
        brightnessEQ.setBrightness(brightness);
        stereoEnhancer.setWidth(width);
    }

    void processSingleSample(juce::AudioBuffer<SampleType>& buffer, int sampleIndex, SampleType inputGain)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.getWritePointer(channel)[sampleIndex] *= inputGain;

        for (int channel = 0; channel < juce::jmin(wetBuffer.getNumChannels(), buffer.getNumChannels()); ++channel)
            wetBuffer.setSample(channel, sampleIndex, buffer.getSample(channel, sampleIndex));

        auto leftSample = wetBuffer.getSample(0, sampleIndex);
        auto rightSample = wetBuffer.getSample(1, sampleIndex);

        leftSample = leftAllpassChain.processSample(leftSample);
        rightSample = rightAllpassChain.processSample(rightSample);

        if (lowCutActive) {
            leftSample = lowCutFilter.processSample(0, leftSample);
            rightSample = lowCutFilter.processSample(1, rightSample);
        }

        if (highCutActive) {
            leftSample = highCutFilter.processSample(0, leftSample);
            rightSample = highCutFilter.processSample(1, rightSample);
        }

        wetBuffer.setSample(0, sampleIndex, leftSample);
        wetBuffer.setSample(1, sampleIndex, rightSample);
    }

    Filters::SchroederAllpassChain<SampleType> leftAllpassChain;
    Filters::SchroederAllpassChain<SampleType> rightAllpassChain;
    Filters::BrightnessEQ<SampleType> brightnessEQ;
    Effects::StereoEnhancer<SampleType> stereoEnhancer;

    juce::dsp::StateVariableTPTFilter<SampleType> lowCutFilter;
    juce::dsp::StateVariableTPTFilter<SampleType> highCutFilter;

    Utils::ParameterSmoother<SampleType> inputGainSmoother;
    Utils::ParameterSmoother<SampleType> outputGainSmoother;
    Utils::ParameterSmoother<SampleType> mixSmoother;
    Utils::ParameterSmoother<SampleType> delaySmoother;
    Utils::ParameterSmoother<SampleType> brightnessSmoother;
    Utils::ParameterSmoother<SampleType> characterSmoother;
    Utils::ParameterSmoother<SampleType> lowCutSmoother;
    Utils::ParameterSmoother<SampleType> highCutSmoother;
    Utils::ParameterSmoother<SampleType> widthSmoother;

    // MakeItLoud

    Effects::MakeItLoud<SampleType> makeItLoud;

    Utils::ParameterSmoother<SampleType> mil_InputGainSmoother;
    Utils::ParameterSmoother<SampleType> mil_BoostSmoother;

    juce::AudioBuffer<SampleType> wetBuffer;
    juce::AudioBuffer<SampleType> dryBuffer;

    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    int numChannels = 2;

    SampleType lastLowCut = SampleType{0.0};
    SampleType lastHighCut = SampleType{0.0};

    bool lowCutActive = false;
    bool highCutActive = false;
};

} // namespace Core
} // namespace DSP
