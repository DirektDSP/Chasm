#pragma once

#include "AllpassFilter.h"
#include "../Utils/ParameterSmoother.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

namespace DSP {
namespace Filters {

/**
 * A Schroeder allpass filter chain for creating dense, diffuse reverb textures.
 * Uses multiple allpass filters in series with carefully chosen delay times.
 */
template<typename SampleType>
class SchroederAllpassChain
{
public:
    static constexpr size_t NumAllpassFilters = 4;
    
    SchroederAllpassChain() = default;
    
    /** Prepares the chain with sample rate. */
    void prepare(double newSampleRate, SampleType initialDelayMs = SampleType{30.0}, SampleType initialCharacter = SampleType{1.0})
    {
        _sampleRate = newSampleRate;

        // Prime delay times for diffusion (in milliseconds)
        std::array<double, NumAllpassFilters> delayTimes = {12.3, 19.7, 29.1, 37.4};

        for (size_t i = 0; i < NumAllpassFilters; ++i)
        {
            allpassFilters[i].prepare(_sampleRate, 100.0); // Max 100ms delay
            allpassFilters[i].setDelayTime(delayTimes[i]);
            allpassFilters[i].setFeedback(static_cast<SampleType>(0.7)); // Default feedback
        }

        // Prepare parameter smoothers
        delayTimeSmoother.prepare(_sampleRate, 50.0); // 50ms smoothing
        characterSmoother.prepare(_sampleRate, 10.0); // 10ms smoothing

        // Set and snap to initial values to avoid ramping artifacts
        delayTimeSmoother.setTargetValue(initialDelayMs);
        delayTimeSmoother.snapToTargetValue();
        characterSmoother.setTargetValue(initialCharacter);
        characterSmoother.snapToTargetValue();
    }
    
    /** Sets the base delay time (will be scaled for each filter). */
    void setDelayTime(SampleType delayMs)
    {
        delayTimeSmoother.setTargetValue(juce::jlimit(SampleType{1.0}, SampleType{100.0}, delayMs));
    }
    
    /** Sets the character (feedback amount) - higher values = more resonant. */
    void setCharacter(SampleType character)
    {
        characterSmoother.setTargetValue(juce::jlimit(static_cast<SampleType>(0.1), static_cast<SampleType>(10.0), static_cast<SampleType>(character)));
    }
    
    /** Processes a single sample through the allpass chain. */
    SampleType processSample(SampleType input)
    {
        updateParameters();
        
        auto output = input;
        
        // Process through each allpass filter in series
        for (auto& filter : allpassFilters)
        {
            output = filter.processSample(output);
        }
        
        return output;
    }
    
    /** Processes a block of samples. */
    void processBlock(SampleType* samples, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            samples[i] = processSample(samples[i]);
        }
    }
    
    /** Resets the filter chain. */
    void reset(SampleType initialDelayMs = SampleType{30.0}, SampleType initialCharacter = SampleType{1.0})
    {
        for (auto& filter : allpassFilters)
        {
            filter.reset();
        }

        delayTimeSmoother.reset(initialDelayMs);
        delayTimeSmoother.setTargetValue(initialDelayMs);
        delayTimeSmoother.snapToTargetValue();
        characterSmoother.reset(initialCharacter);
        characterSmoother.setTargetValue(initialCharacter);
        characterSmoother.snapToTargetValue();
    }

private:
    std::array<AllpassFilter<SampleType>, NumAllpassFilters> allpassFilters;
    Utils::ParameterSmoother<SampleType> delayTimeSmoother;
    Utils::ParameterSmoother<SampleType> characterSmoother;
    
    double _sampleRate = 44100.0;
    
    void updateParameters()
    {
        auto baseDelayTime = delayTimeSmoother.getNextValue();
        auto character = characterSmoother.getNextValue();
        
        // Calculate feedback from character parameter (logarithmic scaling)
        auto feedback = static_cast<SampleType>(0.3 + 0.6 * (std::log(character) / std::log(10.0)));
        feedback = juce::jlimit(static_cast<SampleType>(0.1), static_cast<SampleType>(0.9), feedback);
        
        // Scale delay times with different ratios for each filter
        std::array<SampleType, NumAllpassFilters> delayScales = {
            static_cast<SampleType>(0.41), static_cast<SampleType>(0.66), static_cast<SampleType>(0.97), static_cast<SampleType>(1.25)
        };
        
        for (size_t i = 0; i < NumAllpassFilters; ++i)
        {
            auto scaledDelay = baseDelayTime * delayScales[i];
            allpassFilters[i].setDelayTime(static_cast<double>(scaledDelay));
            allpassFilters[i].setFeedback(feedback);
        }
    }
};

} // namespace Filters
} // namespace DSP
