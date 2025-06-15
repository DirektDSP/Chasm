#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <memory>

namespace DSP {
namespace Filters {

/**
 * Single Allpass Filter implementation.
 * Forms the building block of the Schroeder reverb algorithm.
 */
template<typename SampleType>
class AllpassFilter
{
public:
    AllpassFilter() = default;
    
    /** Prepares the filter with maximum delay time in samples. */
    void prepare(int maxDelaySamples)
    {
        delayLine.clear();
        delayLine.resize(maxDelaySamples + 1, SampleType{0});
        writeIndex = 0;
        maxDelayInSamples = maxDelaySamples;
        reset();
    }
    
    /** Sets the delay time in samples. */
    void setDelay(int delaySamples)
    {
        currentDelay = juce::jlimit(1, maxDelayInSamples, delaySamples);
    }
    
    /** Sets the feedback gain (-1.0 to 1.0). */
    void setGain(SampleType newGain)
    {
        gain = juce::jlimit(SampleType{-0.99}, SampleType{0.99}, newGain);
    }
    
    /** Processes a single sample through the allpass filter. */
    SampleType processSample(SampleType input)
    {
        // Calculate read index
        int readIndex = writeIndex - currentDelay;
        if (readIndex < 0)
            readIndex += static_cast<int>(delayLine.size());
            
        // Get delayed sample
        SampleType delayedSample = delayLine[readIndex];
        
        // Calculate output: output = -gain * input + delayed_input + gain * delayed_output
        SampleType output = -gain * input + delayedSample;
        
        // Calculate feedback signal
        SampleType feedbackSignal = input + gain * output;
        
        // Store in delay line
        delayLine[writeIndex] = feedbackSignal;
        
        // Advance write index
        writeIndex = (writeIndex + 1) % static_cast<int>(delayLine.size());
        
        return output;
    }
    
    /** Resets the filter state. */
    void reset()
    {
        std::fill(delayLine.begin(), delayLine.end(), SampleType{0});
        writeIndex = 0;
    }

private:
    std::vector<SampleType> delayLine;
    int writeIndex = 0;
    int currentDelay = 1;
    int maxDelayInSamples = 1;
    SampleType gain = SampleType{0.5};
};

/**
 * Schroeder Allpass Filter Chain.
 * Implements a series of allpass filters for creating reverb effects.
 */
template<typename SampleType>
class SchroederAllpassChain
{
public:
    SchroederAllpassChain() = default;
    
    /** Prepares the chain with sample rate and maximum delay time. */
    void prepare(double sampleRate, double maxDelayMs = 100.0)
    {
        this->sampleRate = sampleRate;
        this->maxDelayMs = maxDelayMs;
        
        int maxDelaySamples = static_cast<int>(maxDelayMs * 0.001 * sampleRate);
        
        // Create 4 allpass filters with different delay times for rich modulation
        allpassFilters.clear();
        allpassFilters.resize(4);
        
        for (auto& filter : allpassFilters)
        {
            filter = std::make_unique<AllpassFilter<SampleType>>();
            filter->prepare(maxDelaySamples);
        }
        
        // Set different delay ratios for each filter
        updateDelayTimes(30.0); // Default 30ms
        
        // Set gain values for each filter
        if (allpassFilters.size() >= 4)
        {
            allpassFilters[0]->setGain(SampleType{0.7});
            allpassFilters[1]->setGain(SampleType{0.5});
            allpassFilters[2]->setGain(SampleType{0.6});
            allpassFilters[3]->setGain(SampleType{0.4});
        }
    }
    
    /** Sets the base delay time in milliseconds. */
    void setDelayTime(double delayMs)
    {
        currentDelayMs = juce::jlimit(1.0, maxDelayMs, delayMs);
        updateDelayTimes(currentDelayMs);
    }
    
    /** Sets the character (affects gain distribution). */
    void setCharacter(SampleType character)
    {
        // Character affects the gain values of the filters
        SampleType normalizedCharacter = juce::jlimit(SampleType{0.1}, SampleType{1.0}, character / SampleType{10.0});
        
        if (allpassFilters.size() >= 4)
        {
            allpassFilters[0]->setGain(SampleType{0.7} * normalizedCharacter);
            allpassFilters[1]->setGain(SampleType{0.5} * normalizedCharacter);
            allpassFilters[2]->setGain(SampleType{0.6} * normalizedCharacter);
            allpassFilters[3]->setGain(SampleType{0.4} * normalizedCharacter);
        }
    }
    
    /** Processes a single sample through the entire chain. */
    SampleType processSample(SampleType input)
    {
        SampleType output = input;
        
        for (auto& filter : allpassFilters)
        {
            if (filter)
                output = filter->processSample(output);
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
    
    /** Resets all filters in the chain. */
    void reset()
    {
        for (auto& filter : allpassFilters)
        {
            if (filter)
                filter->reset();
        }
    }

private:
    void updateDelayTimes(double baseDelayMs)
    {
        if (allpassFilters.size() >= 4 && sampleRate > 0.0)
        {
            // Use prime number ratios for delay times to avoid comb filtering
            std::vector<double> delayRatios = {1.0, 1.3, 1.7, 2.1};
            
            for (size_t i = 0; i < allpassFilters.size() && i < delayRatios.size(); ++i)
            {
                double delayMs = baseDelayMs * delayRatios[i];
                int delaySamples = static_cast<int>(delayMs * 0.001 * sampleRate);
                allpassFilters[i]->setDelay(delaySamples);
            }
        }
    }
    
    std::vector<std::unique_ptr<AllpassFilter<SampleType>>> allpassFilters;
    double sampleRate = 44100.0;
    double maxDelayMs = 100.0;
    double currentDelayMs = 30.0;
};

} // namespace Filters
} // namespace DSP
