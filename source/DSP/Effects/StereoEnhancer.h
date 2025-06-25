#pragma once

#include "../Utils/ParameterSmoother.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace DSP {
namespace Effects {

/**
 * A stereo enhancer that widens the stereo image through phase manipulation,
 * mid-side processing, and Haas effect.
 */
template<typename SampleType>
class StereoEnhancer
{
public:
    StereoEnhancer() = default;
    
    /** Prepares the enhancer with sample rate and block size. */
    void prepare(double newSampleRate)
    {
        this->sampleRate = newSampleRate;

        // Prepare parameter smoothers with specified smoothing times
        widthSmoother.prepare(sampleRate, 20.0);    // 20ms smoothing
        
        // Set default values
        widthSmoother.setTargetValue(SampleType{100.0});  // 100% width
        
        // Snap to avoid initial smoothing
        widthSmoother.snapToTargetValue();
        
        reset();
    }
    
    /** Sets the stereo width (0-200%). */
    void setWidth(SampleType widthPercent)
    {
        widthSmoother.setTargetValue(juce::jlimit(SampleType{0}, SampleType{200}, widthPercent));
    }
    
    /** Processes a stereo buffer. */
    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        jassert(buffer.getNumChannels() >= 2);
        
        auto numSamples = buffer.getNumSamples();
        
        for (int i = 0; i < numSamples; ++i)
        {
            updateParameters();
            
            // Get current sample values
            auto leftSample = buffer.getSample(0, i);
            auto rightSample = buffer.getSample(1, i);
            
            // Calculate mid and side signals
            auto midSignal = (leftSample + rightSample) * SampleType{0.5};
            auto sideSignal = (leftSample - rightSample) * SampleType{0.5};
            
            
            // Apply width control to side signal
            sideSignal *= currentWidthGain;
            
            // Convert back to left/right
            auto newLeft = midSignal + sideSignal;
            auto newRight = midSignal - sideSignal;
            
            buffer.setSample(0, i, newLeft);
            buffer.setSample(1, i, newRight);
        }
    }
    
    /** Resets the stereo enhancer state. */
    void reset()
    {
        widthSmoother.reset(SampleType{100.0});
        currentWidthGain = SampleType{1.0};
    }

private:
    double sampleRate = 44100.0;
    
    // Parameter smoothers
    Utils::ParameterSmoother<SampleType> widthSmoother;
    
    // Current parameter values
    SampleType currentWidthGain = SampleType{1.0};
    
    void updateParameters()
    {
        // Update width (convert percentage to gain)
        auto widthPercent = widthSmoother.getNextValue();
        currentWidthGain = widthPercent / SampleType{100.0};
    }
};

} // namespace Effects
} // namespace DSP
