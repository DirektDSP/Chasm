#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace DSP {
namespace Filters {

/**
 * High-quality EQ section with high shelf filter for brightness control.
 */
template<typename SampleType>
class BrightnessEQ
{
public:
    BrightnessEQ() = default;
    
    /** Prepares the EQ with sample rate. */
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        highShelfFilter.prepare(spec);
        reset();
    }
    
    /** Sets the brightness amount in dB (-12 to +12). */
    void setBrightness(SampleType brightnessDb)
    {
        brightnessDb = juce::jlimit(SampleType{-12.0}, SampleType{12.0}, brightnessDb);
        
        // High shelf filter at 3kHz
        auto coeffs = juce::dsp::IIR::Coefficients<SampleType>::makeHighShelf(
            sampleRate, 
            SampleType{3000.0}, // 3kHz cutoff
            SampleType{0.707},  // Q factor
            juce::Decibels::decibelsToGain(brightnessDb)
        );
        
        *highShelfFilter.coefficients = *coeffs;
    }
    
    /** Processes a single sample. */
    SampleType processSample(SampleType input)
    {
        return highShelfFilter.processSample(input);
    }
    
    /** Processes a block of samples. */
    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        juce::dsp::AudioBlock<SampleType> block(buffer);
        juce::dsp::ProcessContextReplacing<SampleType> context(block);
        highShelfFilter.process(context);
    }
    
    /** Resets the filter state. */
    void reset()
    {
        highShelfFilter.reset();
    }

private:
    juce::dsp::IIR::Filter<SampleType> highShelfFilter;
    double sampleRate = 44100.0;
};

/**
 * Dual filter section for Low Cut and High Cut controls.
 */
template<typename SampleType>
class DualCutFilter
{
public:
    DualCutFilter() = default;
    
    /** Prepares the filters with sample rate. */
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        lowCutFilter.prepare(spec);
        highCutFilter.prepare(spec);
        reset();
    }
    
    /** Sets the low cut amount (0-100%). */
    void setLowCut(SampleType cutAmount)
    {
        cutAmount = juce::jlimit(SampleType{0.0}, SampleType{100.0}, cutAmount);
        
        if (cutAmount > SampleType{1.0})
        {
            // Map 0-100% to 20Hz-1000Hz
            SampleType frequency = SampleType{20.0} + (cutAmount * 0.01f) * SampleType{980.0};
            
            auto coeffs = juce::dsp::IIR::Coefficients<SampleType>::makeHighPass(
                sampleRate,
                frequency,
                SampleType{0.707} // Butterworth response
            );
            
            *lowCutFilter.coefficients = *coeffs;
            lowCutActive = true;
        }
        else
        {
            lowCutActive = false;
        }
    }
    
    /** Sets the high cut amount (0-100%). */
    void setHighCut(SampleType cutAmount)
    {
        cutAmount = juce::jlimit(SampleType{0.0}, SampleType{100.0}, cutAmount);
        
        if (cutAmount > SampleType{1.0})
        {
            // Map 0-100% to 20kHz-1kHz (inverted)
            SampleType frequency = SampleType{20000.0} - (cutAmount * 0.01f) * SampleType{19000.0};
            
            auto coeffs = juce::dsp::IIR::Coefficients<SampleType>::makeLowPass(
                sampleRate,
                frequency,
                SampleType{0.707} // Butterworth response
            );
            
            *highCutFilter.coefficients = *coeffs;
            highCutActive = true;
        }
        else
        {
            highCutActive = false;
        }
    }
    
    /** Processes a single sample. */
    SampleType processSample(SampleType input)
    {
        SampleType output = input;
        
        if (lowCutActive)
            output = lowCutFilter.processSample(output);
            
        if (highCutActive)
            output = highCutFilter.processSample(output);
            
        return output;
    }
    
    /** Processes a block of samples. */
    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        if (lowCutActive)
        {
            juce::dsp::AudioBlock<SampleType> block(buffer);
            juce::dsp::ProcessContextReplacing<SampleType> context(block);
            lowCutFilter.process(context);
        }
        
        if (highCutActive)
        {
            juce::dsp::AudioBlock<SampleType> block(buffer);
            juce::dsp::ProcessContextReplacing<SampleType> context(block);
            highCutFilter.process(context);
        }
    }
    
    /** Resets the filter states. */
    void reset()
    {
        lowCutFilter.reset();
        highCutFilter.reset();
    }

private:
    juce::dsp::IIR::Filter<SampleType> lowCutFilter;
    juce::dsp::IIR::Filter<SampleType> highCutFilter;
    double sampleRate = 44100.0;
    bool lowCutActive = false;
    bool highCutActive = false;
};

} // namespace Filters
} // namespace DSP
