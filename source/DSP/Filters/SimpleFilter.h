#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace DSP {
namespace Filters {

/**
 * A simple but effective high-pass and low-pass filter implementation.
 * Uses first-order IIR filters for smooth frequency response.
 */
template<typename SampleType>
class SimpleFilter
{
public:
    enum Type
    {
        HighPass,
        LowPass
    };
    
    SimpleFilter() = default;
    
    /** Prepares the filter with sample rate. */
    void prepare(double sampleRate)
    {
        this->sampleRate = sampleRate;
        reset();
    }
    
    /** Sets the filter type. */
    void setType(Type newType)
    {
        type = newType;
        updateCoefficients();
    }
    
    /** Sets the cutoff frequency as a percentage (0-100%). */
    void setCutoffPercentage(SampleType percentage)
    {
        percentage = juce::jlimit(SampleType{0}, SampleType{100}, percentage);
        
        if (type == HighPass)
        {
            // Map 0-100% to 20Hz-2000Hz for high-pass
            auto freq = 20.0 + (percentage / 100.0) * 1980.0;
            setCutoffFrequency(static_cast<SampleType>(freq));
        }
        else
        {
            // Map 0-100% to 2000Hz-20000Hz for low-pass
            auto freq = 2000.0 + (percentage / 100.0) * 18000.0;
            setCutoffFrequency(static_cast<SampleType>(freq));
        }
    }
    
    /** Sets the cutoff frequency in Hz. */
    void setCutoffFrequency(SampleType frequency)
    {
        cutoffFreq = juce::jlimit(SampleType{10}, SampleType{20000}, frequency);
        updateCoefficients();
    }
    
    /** Processes a single sample. */
    SampleType processSample(SampleType input)
    {
        if (type == HighPass)
        {
            // High-pass: y[n] = a * (y[n-1] + x[n] - x[n-1])
            auto output = a * (lastOutput + input - lastInput);
            lastInput = input;
            lastOutput = output;
            return output;
        }
        else
        {
            // Low-pass: y[n] = a * x[n] + (1-a) * y[n-1]
            auto output = a * input + (SampleType{1} - a) * lastOutput;
            lastOutput = output;
            return output;
        }
    }
    
    /** Resets the filter state. */
    void reset()
    {
        lastInput = SampleType{0};
        lastOutput = SampleType{0};
    }

private:
    Type type = HighPass;
    double sampleRate = 44100.0;
    SampleType cutoffFreq = SampleType{1000};
    SampleType a = SampleType{1};
    SampleType lastInput = SampleType{0};
    SampleType lastOutput = SampleType{0};
    
    void updateCoefficients()
    {
        auto omega = juce::MathConstants<double>::twoPi * cutoffFreq / sampleRate;
        
        if (type == HighPass)
        {
            auto rc = 1.0 / (cutoffFreq * juce::MathConstants<double>::twoPi);
            auto dt = 1.0 / sampleRate;
            a = static_cast<SampleType>(rc / (rc + dt));
        }
        else
        {
            auto alpha = std::exp(-omega);
            a = static_cast<SampleType>(1.0 - alpha);
        }
    }
};

} // namespace Filters
} // namespace DSP
