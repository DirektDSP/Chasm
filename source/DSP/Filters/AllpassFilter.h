#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace DSP {
namespace Filters {

/**
 * A single allpass filter with adjustable delay and feedback.
 * Forms the building block for Schroeder reverb networks.
 */
template<typename SampleType>
class AllpassFilter
{
public:
    AllpassFilter() = default;
    
    /** Prepares the filter with sample rate and maximum delay time. */
    void prepare(double newSampleRate, double maxDelayMs)
    {
        _sampleRate = newSampleRate;
        
        auto maxDelaySamples = static_cast<size_t>(maxDelayMs * 0.001 * _sampleRate) + 1;
        delayLine.resize(maxDelaySamples, SampleType{0});
        
        reset();
    }
    
    /** Sets the delay time in milliseconds. */
    void setDelayTime(double delayMs)
    {
        auto newDelaySamples = static_cast<double>(delayMs * 0.001 * _sampleRate);
        this->delaySamples = juce::jlimit(1.0, static_cast<double>(delayLine.size() - 1), newDelaySamples);
    }
    
    /** Sets the feedback coefficient (-1.0 to 1.0). */
    void setFeedback(SampleType newFeedback)
    {
        feedback = juce::jlimit(SampleType{-0.99}, SampleType{0.99}, newFeedback);
    }
    
    /** Processes a single sample. */
    SampleType processSample(SampleType input)
    {
        // Get delayed sample with interpolation
        auto delayedSample = getInterpolatedSample();
        
        // Allpass equation: y[n] = -g*x[n] + x[n-d] + g*y[n-d]
        auto output = -feedback * input + delayedSample;
        
        // Store input + feedback into delay line
        auto feedbackInput = input + feedback * delayedSample;
        delayLine[writeIndex] = feedbackInput;
        
        // Advance write index
        writeIndex = (writeIndex + 1) % delayLine.size();
        
        return output;
    }
    
    /** Resets the filter state. */
    void reset()
    {
        std::fill(delayLine.begin(), delayLine.end(), SampleType{0});
        writeIndex = 0;
        feedback = SampleType{0};
        delaySamples = 1.0;
    }

private:
    std::vector<SampleType> delayLine;
    size_t writeIndex = 0;
    double _sampleRate = 44100.0;
    double delaySamples = 1.0;
    SampleType feedback = SampleType{0};
    
    SampleType getInterpolatedSample() const
    {
        auto readPosition = static_cast<double>(writeIndex) - delaySamples;
        if (readPosition < 0.0)
            readPosition += static_cast<double>(delayLine.size());
            
        auto readIndex1 = static_cast<size_t>(readPosition) % delayLine.size();
        auto readIndex2 = (readIndex1 + 1) % delayLine.size();
        
        auto fraction = readPosition - std::floor(readPosition);
        
        return static_cast<SampleType>((1.0 - fraction) * delayLine[readIndex1] + 
                                       fraction * delayLine[readIndex2]);
    }
};

} // namespace Filters
} // namespace DSP
