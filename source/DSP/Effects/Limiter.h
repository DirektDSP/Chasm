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
        void prepare(double newSampleRate)
        {
            _sampleRate = newSampleRate;
            
            // Prepare the compressor for the limiting stage
            _compressor.prepare({_sampleRate, 256, 2});
            
            // Configure compressor for limiting
            _compressor.setAttack(static_cast<SampleType>(0.1));   // 0.1ms attack
            _compressor.setRelease(static_cast<SampleType>(10.0)); // 10ms release
            _compressor.setThreshold(static_cast<SampleType>(-0.0)); // -0.0dB threshold
            _compressor.setRatio(static_cast<SampleType>(20.0));   // High ratio for limiting
    
            reset();
        }
    
    /** Enables or disables the limiter. */
    void setEnabled(bool shouldBeEnabled)
    {
        _enabled = shouldBeEnabled;
    }
    
    /** Sets the output ceiling level in dB. */
    void setCeiling(SampleType ceilingDb)
    {
        _ceiling = juce::Decibels::decibelsToGain(ceilingDb);
    }
    
    /** Processes a single sample. */
    SampleType processSample(SampleType input)
    {
        if (!_enabled)
            return input;
            
        // First stage: soft clipping for harmonic coloration
        SampleType softClipped = softClip(input);
        
        // Second stage: dynamic limiting
        SampleType limited = dynamicLimit(softClipped);
        
        // Final stage: hard ceiling
        return juce::jlimit(-_ceiling, _ceiling, limited);
    }
    
    /** Processes a stereo pair of samples. */
    void processStereoSample(SampleType& left, SampleType& right)
    {
        if (!_enabled)
            return;
            
        left = processSample(left);
        right = processSample(right);
    }
    
    /** Processes a buffer. */
    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        if (!_enabled)
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
            _compressor.process(context);
        }
    }
    
    /** Resets the limiter state. */
    void reset()
    {
        _compressor.reset();
        _envelopeFollower = SampleType{0};
    }
    
    /** Gets the current gain reduction in dB. */
    SampleType getGainReduction() const
    {
        // This would typically come from the compressor
        // For now, estimate based on envelope follower
        return juce::Decibels::gainToDecibels(std::max(_envelopeFollower, SampleType{0.001}));
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
        if (inputLevel > _envelopeFollower)
        {
            // Fast attack
            _envelopeFollower += (inputLevel - _envelopeFollower) * _attackCoeff;
        }
        else
        {
            // Slower release
            _envelopeFollower += (inputLevel - _envelopeFollower) * _releaseCoeff;
        }
        
        // Calculate gain reduction
        SampleType gainReduction = SampleType{1.0};
        if (_envelopeFollower > _threshold)
        {
            gainReduction = static_cast<SampleType>(_threshold / (_envelopeFollower + static_cast<SampleType>(1e-6)));
        }
        
        return input * gainReduction;
    }
    
    void updateCoefficients()
    {
        if (_sampleRate > 0.0)
        {
            // Calculate smoothing coefficients
            SampleType attackTimeMs = SampleType{0.1};  // 0.1ms attack
            SampleType releaseTimeMs = SampleType{10.0}; // 10ms release
            _attackCoeff = SampleType{1.0} - std::exp(-SampleType{1.0} / (attackTimeMs * SampleType{0.001} * _sampleRate));
            _releaseCoeff = SampleType{1.0} - std::exp(-SampleType{1.0} / (releaseTimeMs * SampleType{0.001} * _sampleRate));
        }
    }

    double _sampleRate = 44100.0;
    bool _enabled = true;
    SampleType _ceiling = SampleType{1.0};
    SampleType _threshold = static_cast<SampleType>(0.8);
    SampleType _envelopeFollower = SampleType{0};
    SampleType _attackCoeff = SampleType{static_cast<SampleType>(0.9)};
    SampleType _releaseCoeff = SampleType{static_cast<SampleType>(0.01)};

    juce::dsp::Compressor<SampleType> _compressor;
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
