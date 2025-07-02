#pragma once

#include "../Filters/AllpassFilter.h"
#include "../Utils/ParameterSmoother.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace DSP {
namespace Effects {

template<typename SampleType>
class HaasEffect
{
public:
    HaasEffect() = default;

    void prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;

        const double maxDelayMs = 50.0;
        rightDelay.prepare(sampleRate, maxDelayMs);
        rightDelay.setFeedback(SampleType{0.0});  // No feedback for Haas

        delaySmoother.prepare(sampleRate, 20.0);
        delaySmoother.setTargetValue(SampleType{20.0});  // Default 20ms
        delaySmoother.snapToTargetValue();

        reset();
    }

    void setDelayMs(SampleType delayMs)
    {
        // Clamp to 0–50ms Haas range
        delaySmoother.setTargetValue(juce::jlimit(SampleType{0.0}, SampleType{50.0}, delayMs));
    }

    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        jassert(buffer.getNumChannels() >= 2);

        auto* left = buffer.getWritePointer(0);
        auto* right = buffer.getWritePointer(1);

        for (int i = 0; i < numSamples; ++i)
        {
            updateParameters();

            // Process only right channel through delay
            right[i] = rightDelay.processSample(right[i]);
        }
    }

    void reset()
    {
        rightDelay.reset();
        delaySmoother.reset(SampleType{20.0});
    }

private:
    double sampleRate = 44100.0;

    DSP::Filters::AllpassFilter<SampleType> rightDelay;
    Utils::ParameterSmoother<SampleType> delaySmoother;

    void updateParameters()
    {
        auto delayMs = delaySmoother.getNextValue();
        rightDelay.setDelayTime(delayMs);
    }
};

} // namespace Effects
} // namespace DSP
