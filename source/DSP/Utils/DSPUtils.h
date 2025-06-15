#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace DSP {
namespace Utils {

/**
 * Utility functions for DSP processing.
 */
class DSPUtils
{
public:
    /** Converts decibels to linear gain. */
    static inline float dbToGain(float db)
    {
        return std::pow(10.0f, db * 0.05f);
    }
    
    /** Converts linear gain to decibels. */
    static inline float gainToDb(float gain)
    {
        return 20.0f * std::log10(std::max(gain, 1e-6f));
    }
    
    /** Converts percentage (0-100) to normalized value (0-1). */
    static inline float percentageToNormalized(float percentage)
    {
        return juce::jlimit(0.0f, 1.0f, percentage * 0.01f);
    }
    
    /** Converts normalized value (0-1) to percentage (0-100). */
    static inline float normalizedToPercentage(float normalized)
    {
        return juce::jlimit(0.0f, 100.0f, normalized * 100.0f);
    }
    
    /** Logarithmic scaling for delay time (1-100ms range). */
    static inline float normalizedToDelayMs(float normalized)
    {
        // Logarithmic scaling from 1ms to 100ms
        return 1.0f + (99.0f * std::pow(normalized, 2.0f));
    }
    
    /** Inverse logarithmic scaling for delay time. */
    static inline float delayMsToNormalized(float delayMs)
    {
        // Inverse of logarithmic scaling
        return std::sqrt((delayMs - 1.0f) / 99.0f);
    }
    
    /** Logarithmic scaling for Q factor (0.1-10 range). */
    static inline float normalizedToQFactor(float normalized)
    {
        // Logarithmic scaling from 0.1 to 10
        return 0.1f * std::pow(100.0f, normalized);
    }
    
    /** Inverse logarithmic scaling for Q factor. */
    static inline float qFactorToNormalized(float qFactor)
    {
        // Inverse of logarithmic scaling
        return std::log10(qFactor / 0.1f) / 2.0f;
    }
    
    /** Soft clipping for audio signals. */
    static inline float softClip(float input)
    {
        return std::tanh(input);
    }
    
    /** Hard clipping for audio signals. */
    static inline float hardClip(float input, float threshold = 1.0f)
    {
        return juce::jlimit(-threshold, threshold, input);
    }
    
    /** Linear interpolation between two values. */
    static inline float lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }
    
    /** Check if a floating point number is denormal and flush to zero if needed. */
    static inline float flushDenormalToZero(float input)
    {
        return std::abs(input) < 1e-30f ? 0.0f : input;
    }
};

} // namespace Utils
} // namespace DSP
