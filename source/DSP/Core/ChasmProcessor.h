#pragma once

#include "../Filters/SchroederAllpassChain.h"
#include "../Effects/StereoEnhancer.h"
#include "../Effects/Limiter.h"
#include "../Utils/ParameterSmoother.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace DSP {
namespace Core {

/**
 * Main DSP processor that combines all effects in the Chasm plugin.
 * Handles the complete signal chain: Input Gain -> Allpass Chain -> Stereo Enhancer -> Limiter -> Output Gain
 */
template<typename SampleType>
class ChasmProcessor
{
public:
    ChasmProcessor() = default;
    
    /** Prepares the processor with sample rate and block size. */
    void prepare(double newSampleRate, int newMaxBlockSize)
    {
        this->sampleRate = newSampleRate;
        this->maxBlockSize = newMaxBlockSize;
          // Prepare all DSP components
        schroederChain.prepare(sampleRate);
        stereoEnhancer.prepare(sampleRate);
        limiter.prepare(sampleRate);
        
        // Prepare parameter smoothers with specified smoothing times from your table
        inputGainSmoother.prepare(sampleRate, 5.0);   // 5ms smoothing
        outputGainSmoother.prepare(sampleRate, 5.0);  // 5ms smoothing
        mixSmoother.prepare(sampleRate, 20.0);        // 20ms smoothing
        
        // Set default values
        inputGainSmoother.setTargetValue(SampleType{1.0});   // 0 dB (linear)
        outputGainSmoother.setTargetValue(SampleType{1.0});  // 0 dB (linear)
        mixSmoother.setTargetValue(SampleType{50.0});        // 50% mix
        
        // Snap to avoid initial smoothing
        inputGainSmoother.snapToTargetValue();
        outputGainSmoother.snapToTargetValue();
        mixSmoother.snapToTargetValue();
        
        // Prepare working buffer for dry signal
        dryBuffer.setSize(2, maxBlockSize);
        
        reset();
    }
    
    // Parameter setters based on your specification table
    
    /** Sets input gain in dB (-24 to +24 dB). */
    void setInputGain(SampleType gainDb)
    {
        auto gainLinear = juce::Decibels::decibelsToGain(static_cast<float>(
            juce::jlimit(SampleType{-24}, SampleType{24}, gainDb)
        ));
        inputGainSmoother.setTargetValue(static_cast<SampleType>(gainLinear));
    }
    
    /** Sets output gain in dB (-24 to +24 dB). */
    void setOutputGain(SampleType gainDb)
    {
        auto gainLinear = juce::Decibels::decibelsToGain(static_cast<float>(
            juce::jlimit(SampleType{-24}, SampleType{24}, gainDb)
        ));
        outputGainSmoother.setTargetValue(static_cast<SampleType>(gainLinear));
    }
    
    /** Sets wet/dry mix (0 to 100%). */
    void setMix(SampleType mixPercent)
    {
        mixSmoother.setTargetValue(juce::jlimit(SampleType{0}, SampleType{100}, mixPercent));
    }
    
    /** Sets delay time in ms (1 to 100ms, logarithmic scaling). */
    void setDelay(SampleType delayMs)
    {
        schroederChain.setDelayTime(juce::jlimit(SampleType{1}, SampleType{100}, delayMs));
    }
    
    /** Sets brightness in dB (-12 to +12 dB). */
    void setBrightness(SampleType brightnessDb)
    {
        stereoEnhancer.setBrightness(juce::jlimit(SampleType{-12}, SampleType{12}, brightnessDb));
    }
    
    /** Sets character Q factor (0.1 to 10, logarithmic scaling). */
    void setCharacter(SampleType character)
    {
        schroederChain.setCharacter(juce::jlimit(SampleType{0.1}, SampleType{10}, character));
    }
    
    /** Sets low cut amount (0 to 100%). */
    void setLowCut(SampleType lowCutPercent)
    {
        stereoEnhancer.setLowCut(juce::jlimit(SampleType{0}, SampleType{100}, lowCutPercent));
    }
    
    /** Sets high cut amount (0 to 100%). */
    void setHighCut(SampleType highCutPercent)
    {
        stereoEnhancer.setHighCut(juce::jlimit(SampleType{0}, SampleType{100}, highCutPercent));
    }
    
    /** Sets stereo width (0 to 200%). */
    void setWidth(SampleType widthPercent)
    {
        stereoEnhancer.setWidth(juce::jlimit(SampleType{0}, SampleType{200}, widthPercent));
    }
      /** Sets limiter on/off. */
    void setLimiterEnabled(bool enabled)
    {
        limiterEnabled = enabled;
        limiter.setEnabled(enabled);
    }
    
    /** Processes a stereo audio buffer. */
    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        jassert(buffer.getNumChannels() >= 2);
        
        auto numSamples = buffer.getNumSamples();
        
        // Store dry signal for mix control
        dryBuffer.setSize(2, numSamples, false, false, true);
        for (int channel = 0; channel < 2; ++channel)
        {
            dryBuffer.copyFrom(channel, 0, buffer, channel, 0, numSamples);
        }
        
        // Apply input gain
        applyGain(buffer, inputGainSmoother, numSamples);
        
        // Process each channel through the Schroeder allpass chain
        for (int channel = 0; channel < 2; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            schroederChain.processBlock(channelData, numSamples);
        }
        
        // Apply stereo enhancement
        stereoEnhancer.processBlock(buffer);
        
        // Apply limiter if enabled
        if (limiterEnabled)
        {
            limiter.processBlock(buffer);
        }
        
        // Apply wet/dry mix
        applyMix(buffer, dryBuffer, numSamples);
        
        // Apply output gain
        applyGain(buffer, outputGainSmoother, numSamples);
    }
    
    /** Resets all DSP components. */
    void reset()
    {
        schroederChain.reset();
        stereoEnhancer.reset();
        limiter.reset();
        
        inputGainSmoother.reset(SampleType{1.0});
        outputGainSmoother.reset(SampleType{1.0});
        mixSmoother.reset(SampleType{50.0});
        
        dryBuffer.clear();
    }

private:
    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    bool limiterEnabled = true;
      // DSP Components
    Filters::SchroederAllpassChain<SampleType> schroederChain;
    Effects::StereoEnhancer<SampleType> stereoEnhancer;
    Effects::SmoothLimiter<SampleType> limiter;
    
    // Parameter smoothers
    Utils::ParameterSmoother<SampleType> inputGainSmoother;
    Utils::ParameterSmoother<SampleType> outputGainSmoother;
    Utils::ParameterSmoother<SampleType> mixSmoother;
    
    // Working buffers
    juce::AudioBuffer<SampleType> dryBuffer;
    
    void applyGain(juce::AudioBuffer<SampleType>& buffer, 
                   Utils::ParameterSmoother<SampleType>& gainSmoother, 
                   int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            auto gain = gainSmoother.getNextValue();
            
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                channelData[i] *= gain;
            }
        }
    }
    
    void applyMix(juce::AudioBuffer<SampleType>& wetBuff,
                  const juce::AudioBuffer<SampleType>& dryBuff,
                  int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            auto mixPercent = mixSmoother.getNextValue();
            auto wetGain = mixPercent / SampleType{100.0};
            auto dryGain = SampleType{1.0} - wetGain;
            
            for (int channel = 0; channel < wetBuff.getNumChannels(); ++channel)
            {
                auto* wetData = wetBuff.getWritePointer(channel);
                auto* dryData = dryBuff.getReadPointer(channel);
                
                wetData[i] = wetData[i] * wetGain + dryData[i] * dryGain;
            }
        }
    }
};

} // namespace Core
} // namespace DSP
