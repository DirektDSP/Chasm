#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace DSP {
namespace Utils {

/**
 * A parameter smoother with configurable smoothing time.
 * Uses exponential smoothing for natural parameter transitions.
 */
template<typename SampleType>
class ParameterSmoother
{
public:
    ParameterSmoother() = default;
    
    /** Prepares the smoother with sample rate and smoothing time. */
    void prepare(double newSampleRate, double newSmoothingTimeMs)
    {
        jassert(newSampleRate > 0.0 && newSmoothingTimeMs >= 0.0);

        _sampleRate = newSampleRate;
        _smoothingTimeMs = newSmoothingTimeMs;
        
        // Calculate smoothing coefficient
        if (_smoothingTimeMs > 0.0)
        {
            auto samplesForSmoothingTime = _smoothingTimeMs * 0.001 * _sampleRate;
            smoothingCoeff = static_cast<SampleType>(1.0 - std::exp(-1.0 / samplesForSmoothingTime));
        }
        else
        {
            smoothingCoeff = static_cast<SampleType>(1.0);
        }
    }
    
    /** Sets the target value to smooth towards. */
    void setTargetValue(SampleType newTargetValue)
    {
        targetValue = newTargetValue;
    }
    
    /** Gets the next smoothed sample. */
    SampleType getNextValue()
    {
        currentValue += smoothingCoeff * (targetValue - currentValue);
        return currentValue;
    }
    
    /** Processes a block of samples with the same target value. */
    void processBlock(SampleType* samples, int numSamples, SampleType newTargetValue)
    {
        setTargetValue(newTargetValue);
        
        for (int i = 0; i < numSamples; ++i)
        {
            samples[i] = getNextValue();
        }
    }
    
    /** Skips to the target value immediately (useful for initialization). */
    void snapToTargetValue()
    {
        currentValue = targetValue;
    }
    
    /** Gets the current smoothed value without advancing. */
    SampleType getCurrentValue() const { return currentValue; }
    
    /** Gets the target value. */
    SampleType getTargetValue() const { return targetValue; }
    
    /** Resets the smoother to a specific value. */
    void reset(SampleType initialValue = SampleType{0})
    {
        currentValue = targetValue = initialValue;
    }

private:
    double _sampleRate = 44100.0;
    double _smoothingTimeMs = 0.0;
    SampleType smoothingCoeff = SampleType{1};
    SampleType currentValue = SampleType{0};
    SampleType targetValue = SampleType{0};
};

} // namespace Utils
} // namespace DSP
