#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace DSP {
namespace Effects {

/**
 * Smooth Limiter for preventing clipping and adding character.
 * Uses a combination of soft clipping and dynamic range compression.
 */
template<typename SampleType>
class SmoothLimiter
{
public:
    SmoothLimiter() = default;
    
    /** Prepares the limiter with sample rate. */
    void prepare(double sampleRate)
    {
        this->sampleRate = sampleRate;
        
        // Prepare the compressor for the limiting stage
        compressor.prepare({sampleRate, 256, 2});
        
        // Configure compressor for limiting
        compressor.setAttack(SampleType{0.1});   // 0.1ms attack
        compressor.setRelease(SampleType{10.0}); // 10ms release
        compressor.setThreshold(SampleType{-0.5}); // -0.5dB threshold
        compressor.setRatio(SampleType{20.0});   // High ratio for limiting
        
        reset();
    }
    
    /** Enables or disables the limiter. */
    void setEnabled(bool shouldBeEnabled)
    {
        enabled = shouldBeEnabled;
    }
    
    /** Sets the output ceiling level in dB. */
    void setCeiling(SampleType ceilingDb)
    {
        ceiling = juce::Decibels::decibelsToGain(ceilingDb);
    }
    
    /** Processes a single sample. */
    SampleType processSample(SampleType input)
    {
        if (!enabled)
            return input;
            
        // First stage: soft clipping for harmonic coloration
        SampleType softClipped = softClip(input);
        
        // Second stage: dynamic limiting
        SampleType limited = dynamicLimit(softClipped);
        
        // Final stage: hard ceiling
        return juce::jlimit(-ceiling, ceiling, limited);
    }
    
    /** Processes a stereo pair of samples. */
    void processStereoSample(SampleType& left, SampleType& right)
    {
        if (!enabled)
            return;
            
        left = processSample(left);
        right = processSample(right);
    }
    
    /** Processes a buffer. */
    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        if (!enabled)
            return;
            
        int numChannels = buffer.getNumChannels();
        int numSamples = buffer.getNumSamples();
        
        // Process each channel
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            
            for (int i = 0; i < numSamples; ++i)
            {
                channelData[i] = processSample(channelData[i]);
            }
        }
        
        // Apply compressor for final limiting stage
        if (numChannels > 0)
        {
            juce::dsp::AudioBlock<SampleType> block(buffer);
            juce::dsp::ProcessContextReplacing<SampleType> context(block);
            compressor.process(context);
        }
    }
    
    /** Resets the limiter state. */
    void reset()
    {
        compressor.reset();
        envelopeFollower = SampleType{0};
    }
    
    /** Gets the current gain reduction in dB. */
    SampleType getGainReduction() const
    {
        // This would typically come from the compressor
        // For now, estimate based on envelope follower
        return juce::Decibels::gainToDecibels(std::max(envelopeFollower, SampleType{0.001}));
    }

private:
    SampleType softClip(SampleType input)
    {
        // Smooth soft clipping using tanh
        return std::tanh(input * SampleType{2.0}) * SampleType{0.5};
    }
    
    SampleType dynamicLimit(SampleType input)
    {
        // Simple envelope follower for dynamic limiting
        SampleType inputLevel = std::abs(input);
        
        // Update envelope
        if (inputLevel > envelopeFollower)
        {
            // Fast attack
            envelopeFollower += (inputLevel - envelopeFollower) * attackCoeff;
        }
        else
        {
            // Slower release
            envelopeFollower += (inputLevel - envelopeFollower) * releaseCoeff;
        }
        
        // Calculate gain reduction
        SampleType gainReduction = SampleType{1.0};
        if (envelopeFollower > threshold)
        {
            gainReduction = threshold / (envelopeFollower + SampleType{1e-6});
        }
        
        return input * gainReduction;
    }
    
    void updateCoefficients()
    {
        if (sampleRate > 0.0)
        {
            // Calculate smoothing coefficients
            SampleType attackTimeMs = SampleType{0.1};  // 0.1ms attack
            SampleType releaseTimeMs = SampleType{10.0}; // 10ms release
            
            attackCoeff = SampleType{1.0} - std::exp(-SampleType{1.0} / (attackTimeMs * SampleType{0.001} * SampleType{sampleRate}));
            releaseCoeff = SampleType{1.0} - std::exp(-SampleType{1.0} / (releaseTimeMs * SampleType{0.001} * SampleType{sampleRate}));
        }
    }
    
    double sampleRate = 44100.0;
    bool enabled = true;
    SampleType ceiling = SampleType{1.0};
    SampleType threshold = SampleType{0.8};
    SampleType envelopeFollower = SampleType{0};
    SampleType attackCoeff = SampleType{0.9};
    SampleType releaseCoeff = SampleType{0.01};
    
    juce::dsp::Compressor<SampleType> compressor;
};

/**
 * Simple Brick Wall Limiter.
 * Hard limiting at a specified ceiling.
 */
template<typename SampleType>
class BrickWallLimiter
{
public:
    BrickWallLimiter() = default;
    
    /** Sets the ceiling level (0-1). */
    void setCeiling(SampleType ceilingLevel)
    {
        ceiling = juce::jlimit(SampleType{0.1}, SampleType{1.0}, ceilingLevel);
    }
    
    /** Enables or disables the limiter. */
    void setEnabled(bool shouldBeEnabled)
    {
        enabled = shouldBeEnabled;
    }
    
    /** Processes a single sample. */
    SampleType processSample(SampleType input)
    {
        if (!enabled)
            return input;
            
        return juce::jlimit(-ceiling, ceiling, input);
    }
    
    /** Processes a buffer. */
    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        if (!enabled)
            return;
            
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                channelData[i] = processSample(channelData[i]);
            }
        }
    }

private:
    bool enabled = true;
    SampleType ceiling = SampleType{1.0};
};

} // namespace Effects
} // namespace DSP
