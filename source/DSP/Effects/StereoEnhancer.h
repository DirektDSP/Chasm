#pragma once

#include "../Utils/ParameterSmoother.h"
#include "../Filters/SimpleFilter.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace DSP {
namespace Effects {

/**
 * A stereo enhancer that widens the stereo image through phase manipulation,
 * mid-side processing, and frequency-dependent stereo enhancement.
 */
template<typename SampleType>
class StereoEnhancer
{
public:
    StereoEnhancer() = default;
    
    /** Prepares the enhancer with sample rate and block size. */
    void prepare(double sampleRate, int maxBlockSize)
    {
        this->sampleRate = sampleRate;
        
        // Prepare filters for frequency-dependent processing
        lowCutFilter.prepare(sampleRate);
        lowCutFilter.setType(Filters::SimpleFilter<SampleType>::HighPass);
        
        highCutFilter.prepare(sampleRate);
        highCutFilter.setType(Filters::SimpleFilter<SampleType>::LowPass);
        
        brightnessFilter.prepare(sampleRate);
        brightnessFilter.setType(Filters::SimpleFilter<SampleType>::HighPass);
        
        // Prepare parameter smoothers with specified smoothing times
        widthSmoother.prepare(sampleRate, 20.0);    // 20ms smoothing
        brightnessSmoother.prepare(sampleRate, 10.0); // 10ms smoothing
        lowCutSmoother.prepare(sampleRate, 20.0);   // 20ms smoothing
        highCutSmoother.prepare(sampleRate, 20.0);  // 20ms smoothing
        
        // Set default values
        widthSmoother.setTargetValue(SampleType{100.0});  // 100% width
        brightnessSmoother.setTargetValue(SampleType{0.0}); // 0 dB brightness
        lowCutSmoother.setTargetValue(SampleType{0.0});   // 0% low cut
        highCutSmoother.setTargetValue(SampleType{0.0});  // 0% high cut
        
        // Snap to avoid initial smoothing
        widthSmoother.snapToTargetValue();
        brightnessSmoother.snapToTargetValue();
        lowCutSmoother.snapToTargetValue();
        highCutSmoother.snapToTargetValue();
        
        reset();
    }
    
    /** Sets the stereo width (0-200%). */
    void setWidth(SampleType widthPercent)
    {
        widthSmoother.setTargetValue(juce::jlimit(SampleType{0}, SampleType{200}, widthPercent));
    }
    
    /** Sets the brightness in dB (-12 to +12 dB). */
    void setBrightness(SampleType brightnessDb)
    {
        brightnessSmoother.setTargetValue(juce::jlimit(SampleType{-12}, SampleType{12}, brightnessDb));
    }
    
    /** Sets the low cut amount (0-100%). */
    void setLowCut(SampleType lowCutPercent)
    {
        lowCutSmoother.setTargetValue(juce::jlimit(SampleType{0}, SampleType{100}, lowCutPercent));
    }
    
    /** Sets the high cut amount (0-100%). */
    void setHighCut(SampleType highCutPercent)
    {
        highCutSmoother.setTargetValue(juce::jlimit(SampleType{0}, SampleType{100}, highCutPercent));
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
            
            // Apply low and high cut filters to side signal
            sideSignal = lowCutFilter.processSample(sideSignal);
            sideSignal = highCutFilter.processSample(sideSignal);
            
            // Apply width control to side signal
            sideSignal *= currentWidthGain;
            
            // Apply brightness enhancement to side signal
            if (currentBrightness != SampleType{0.0})
            {
                auto brightnessFactor = juce::Decibels::decibelsToGain(static_cast<float>(currentBrightness));
                auto highFreqContent = brightnessFilter.processSample(sideSignal);
                sideSignal += highFreqContent * (brightnessFactor - SampleType{1.0});
            }
            
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
        lowCutFilter.reset();
        highCutFilter.reset();
        brightnessFilter.reset();
        
        widthSmoother.reset(SampleType{100.0});
        brightnessSmoother.reset(SampleType{0.0});
        lowCutSmoother.reset(SampleType{0.0});
        highCutSmoother.reset(SampleType{0.0});
        
        currentWidthGain = SampleType{1.0};
        currentBrightness = SampleType{0.0};
        currentLowCut = SampleType{0.0};
        currentHighCut = SampleType{0.0};
    }

private:
    double sampleRate = 44100.0;
    
    // Filters
    Filters::SimpleFilter<SampleType> lowCutFilter;
    Filters::SimpleFilter<SampleType> highCutFilter;
    Filters::SimpleFilter<SampleType> brightnessFilter;
    
    // Parameter smoothers
    Utils::ParameterSmoother<SampleType> widthSmoother;
    Utils::ParameterSmoother<SampleType> brightnessSmoother;
    Utils::ParameterSmoother<SampleType> lowCutSmoother;
    Utils::ParameterSmoother<SampleType> highCutSmoother;
    
    // Current parameter values
    SampleType currentWidthGain = SampleType{1.0};
    SampleType currentBrightness = SampleType{0.0};
    SampleType currentLowCut = SampleType{0.0};
    SampleType currentHighCut = SampleType{0.0};
    
    void updateParameters()
    {
        // Update width (convert percentage to gain)
        auto widthPercent = widthSmoother.getNextValue();
        currentWidthGain = widthPercent / SampleType{100.0};
        
        // Update brightness
        currentBrightness = brightnessSmoother.getNextValue();
        
        // Update filter cutoffs
        currentLowCut = lowCutSmoother.getNextValue();
        currentHighCut = highCutSmoother.getNextValue();
        
        // Update filter parameters
        lowCutFilter.setCutoffPercentage(currentLowCut);
        highCutFilter.setCutoffPercentage(currentHighCut);
        
        // Set brightness filter to a high frequency for enhancement
        brightnessFilter.setCutoffFrequency(SampleType{3000.0});
    }
};

} // namespace Effects
} // namespace DSP
