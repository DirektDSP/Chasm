#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

// DSP Components
#include "../Utils/ParameterSmoother.h"
#include "../Utils/DSPUtils.h"
#include "../Filters/SchroederAllpassChain.h"
#include "../Filters/EQFilters.h"
#include "../Effects/StereoEnhancer.h"
#include "../Effects/Limiter.h"

namespace DSP {
namespace Core {

/**
 * Main DSP Processor for Chasm.
 * Coordinates all DSP components with parameter smoothing.
 */
template<typename SampleType>
class ChasmDSPProcessor
{
public:
    ChasmDSPProcessor() = default;
    
    /** Prepares all DSP components. */
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        samplesPerBlock = static_cast<int>(spec.maximumBlockSize);
        numChannels = static_cast<int>(spec.numChannels);
        
        // Prepare all DSP components
        leftAllpassChain.prepare(sampleRate);  // Max 100ms delay
        rightAllpassChain.prepare(sampleRate);
        
        brightnessEQ.prepare(spec);
        dualCutFilter.prepare(spec);
        stereoEnhancer.setWidth(SampleType{100.0}); // Default 100% width
        limiter.prepare(sampleRate);
        
        // Prepare parameter smoothers with their respective smoothing times
        prepareParameterSmoothers();
        
        // Create working buffers
        wetBuffer.setSize(numChannels, samplesPerBlock);
        dryBuffer.setSize(numChannels, samplesPerBlock);
        
        reset();
    }
    
    /** Updates all parameters with smoothing. */
    void updateParameters(SampleType inputGainDb, SampleType outputGainDb, SampleType mixPercent,
                         SampleType delayMs, SampleType brightnessDb, SampleType characterQ,
                         SampleType lowCutPercent, SampleType highCutPercent, SampleType widthPercent,
                         bool limiterEnabled)
    {
        // Update all parameter smoothers
        inputGainSmoother.setTargetValue(Utils::DSPUtils::dbToGain(inputGainDb));
        outputGainSmoother.setTargetValue(Utils::DSPUtils::dbToGain(outputGainDb));
        mixSmoother.setTargetValue(Utils::DSPUtils::percentageToNormalized(mixPercent));
        delaySmoother.setTargetValue(delayMs);
        brightnessSmoother.setTargetValue(brightnessDb);
        characterSmoother.setTargetValue(characterQ);
        lowCutSmoother.setTargetValue(lowCutPercent);
        highCutSmoother.setTargetValue(highCutPercent);
        widthSmoother.setTargetValue(widthPercent);
        
        // Limiter is not smoothed (binary parameter)
        limiter.setEnabled(limiterEnabled);
    }
    
    /** Processes a block of audio. */
    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        jassert(buffer.getNumChannels() >= 1);
        
        int numSamples = buffer.getNumSamples();
        
        // Ensure working buffers are the right size
        if (wetBuffer.getNumSamples() != numSamples)
        {
            wetBuffer.setSize(numChannels, numSamples, false, false, true);
            dryBuffer.setSize(numChannels, numSamples, false, false, true);
        }
        
        // Store dry signal
        dryBuffer.makeCopyOf(buffer);
        
        // Process each sample with parameter smoothing
        for (int i = 0; i < numSamples; ++i)
        {
            // Get smoothed parameter values for this sample
            SampleType inputGain = inputGainSmoother.getNextValue();
            SampleType outputGain = outputGainSmoother.getNextValue();
            SampleType mix = mixSmoother.getNextValue();
            SampleType delay = delaySmoother.getNextValue();
            SampleType brightness = brightnessSmoother.getNextValue();
            SampleType character = characterSmoother.getNextValue();
            SampleType lowCut = lowCutSmoother.getNextValue();
            SampleType highCut = highCutSmoother.getNextValue();
            SampleType width = widthSmoother.getNextValue();
            
            // Update DSP components with smoothed values
            if (i == 0 || shouldUpdateDSPComponents(i))
            {
                updateDSPComponents(delay, brightness, character, lowCut, highCut, width);
            }
            
            // Process single sample
            processSingleSample(buffer, i, inputGain, outputGain, mix);
        }

        // Apply stereo enhancement ONCE per buffer (if stereo)
        if (buffer.getNumChannels() >= 2)
        {
            stereoEnhancer.processBlock(wetBuffer);
        }

        // After enhancement, mix dry/wet and apply output gain
        for (int i = 0; i < numSamples; ++i)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                SampleType drySample = dryBuffer.getSample(channel, i);
                SampleType wetSample = wetBuffer.getSample(channel, i);
                SampleType mix = mixSmoother.getCurrentValue();
                SampleType outputGain = outputGainSmoother.getCurrentValue();
                SampleType mixedSample = drySample * (SampleType{1.0} - mix) + wetSample * mix;
                channelData[i] = mixedSample * outputGain;
            }
        }

        // Apply final limiter
        limiter.processBlock(buffer);
    }
    
    /** Resets all DSP components. */
    void reset()
    {
        leftAllpassChain.reset();
        rightAllpassChain.reset();
        brightnessEQ.reset();
        dualCutFilter.reset();
        stereoEnhancer.reset();
        limiter.reset();
        
        // Reset parameter smoothers
        inputGainSmoother.reset(SampleType{1.0});
        outputGainSmoother.reset(SampleType{1.0});
        mixSmoother.reset(SampleType{0.5});
        delaySmoother.reset(SampleType{30.0});
        brightnessSmoother.reset(SampleType{0.0});
        characterSmoother.reset(SampleType{1.0});
        lowCutSmoother.reset(SampleType{0.0});
        highCutSmoother.reset(SampleType{0.0});
        widthSmoother.reset(SampleType{100.0});
    }

private:
    void prepareParameterSmoothers()
    {
        // Prepare smoothers with their specified smoothing times
        inputGainSmoother.prepare(sampleRate, 5.0);   // 5ms
        outputGainSmoother.prepare(sampleRate, 5.0);  // 5ms
        mixSmoother.prepare(sampleRate, 20.0);        // 20ms
        delaySmoother.prepare(sampleRate, 50.0);      // 50ms
        brightnessSmoother.prepare(sampleRate, 10.0); // 10ms
        characterSmoother.prepare(sampleRate, 10.0);  // 10ms
        lowCutSmoother.prepare(sampleRate, 20.0);     // 20ms
        highCutSmoother.prepare(sampleRate, 20.0);    // 20ms
        widthSmoother.prepare(sampleRate, 20.0);      // 20ms
    }
    
    bool shouldUpdateDSPComponents(int sampleIndex)
    {
        // Update DSP components every 32 samples to balance quality vs performance
        return (sampleIndex % 32) == 0;
    }
    
    void updateDSPComponents(SampleType delay, SampleType brightness, SampleType character,
                           SampleType lowCut, SampleType highCut, SampleType width)
    {
        // Update allpass chains
        leftAllpassChain.setDelayTime(delay);
        rightAllpassChain.setDelayTime(delay);
        leftAllpassChain.setCharacter(character);
        rightAllpassChain.setCharacter(character);
        
        // Update EQ and filters
        brightnessEQ.setBrightness(brightness);
        dualCutFilter.setLowCut(lowCut);
        dualCutFilter.setHighCut(highCut);
        
        // Update stereo enhancer
        stereoEnhancer.setWidth(width);
    }
    
    void processSingleSample(juce::AudioBuffer<SampleType>& buffer, int sampleIndex,
                           SampleType inputGain, SampleType outputGain, SampleType mix)
    {
        // Apply input gain
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            channelData[sampleIndex] *= inputGain;
        }
        
        // Store wet signal for processing
        for (int channel = 0; channel < juce::jmin(wetBuffer.getNumChannels(), buffer.getNumChannels()); ++channel)
        {
            wetBuffer.setSample(channel, sampleIndex, buffer.getSample(channel, sampleIndex));
        }
        
        // Process through DSP chain
        if (buffer.getNumChannels() >= 2)
        {
            // Stereo processing
            SampleType leftSample = wetBuffer.getSample(0, sampleIndex);
            SampleType rightSample = wetBuffer.getSample(1, sampleIndex);
            
            // Allpass filtering
            leftSample = leftAllpassChain.processSample(leftSample);
            rightSample = rightAllpassChain.processSample(rightSample);
            
            // EQ and filtering
            leftSample = brightnessEQ.processSample(leftSample);
            rightSample = brightnessEQ.processSample(rightSample);
            
            leftSample = dualCutFilter.processSample(leftSample);
            rightSample = dualCutFilter.processSample(rightSample);
            
            wetBuffer.setSample(0, sampleIndex, leftSample);
            wetBuffer.setSample(1, sampleIndex, rightSample);
            
            // Stereo enhancement (removed from per-sample processing)
            // stereoEnhancer.processStereoSample(leftSample, rightSample);
            
        }
        else if (buffer.getNumChannels() == 1)
        {
            // Mono processing
            SampleType sample = wetBuffer.getSample(0, sampleIndex);
            
            // Process through left chain only for mono
            sample = leftAllpassChain.processSample(sample);
            sample = brightnessEQ.processSample(sample);
            sample = dualCutFilter.processSample(sample);
            
            wetBuffer.setSample(0, sampleIndex, sample);
        }
        
        // Mix dry and wet signals with output gain
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            SampleType drySample = dryBuffer.getSample(channel, sampleIndex);
            SampleType wetSample = wetBuffer.getSample(channel, sampleIndex);
            
            SampleType mixedSample = drySample * (SampleType{1.0} - mix) + wetSample * mix;
            channelData[sampleIndex] = mixedSample * outputGain;
        }
    }
    
    // DSP Components
    Filters::SchroederAllpassChain<SampleType> leftAllpassChain;
    Filters::SchroederAllpassChain<SampleType> rightAllpassChain;
    Filters::BrightnessEQ<SampleType> brightnessEQ;
    Filters::DualCutFilter<SampleType> dualCutFilter;
    Effects::StereoEnhancer<SampleType> stereoEnhancer;
    Effects::SmoothLimiter<SampleType> limiter;
    
    // Parameter Smoothers
    Utils::ParameterSmoother<SampleType> inputGainSmoother;
    Utils::ParameterSmoother<SampleType> outputGainSmoother;
    Utils::ParameterSmoother<SampleType> mixSmoother;
    Utils::ParameterSmoother<SampleType> delaySmoother;
    Utils::ParameterSmoother<SampleType> brightnessSmoother;
    Utils::ParameterSmoother<SampleType> characterSmoother;
    Utils::ParameterSmoother<SampleType> lowCutSmoother;
    Utils::ParameterSmoother<SampleType> highCutSmoother;
    Utils::ParameterSmoother<SampleType> widthSmoother;
    
    // Working buffers
    juce::AudioBuffer<SampleType> wetBuffer;
    juce::AudioBuffer<SampleType> dryBuffer;
    
    // Audio settings
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    int numChannels = 2;
};

} // namespace Core
} // namespace DSP
