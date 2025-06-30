#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace DSP {
namespace Effects {

/**
 * @brief MakeItLoud is a dynamic waveshaping effect chain consisting of:
 * - Pre-compressor (with selectable mode)
 * - Input gain + boost gain
 * - Waveshaper using tanh
 * - Post-compressor (same settings as pre)
 * 
 * Parameters:
 * - Input gain (linear, 0.0 to 2.0)
 * - Boost gain (linear, 0.0 to 2.0)
 * - Compressor mode (Clean, Further, Crunchy)
 * - Bypass mode / Enabled state
 *
 * Designed to be loud and punchy, with tone control via compressor modes.
 *
 * @tparam SampleType float or double, depending on processing precision
 */
template<typename SampleType>
class MakeItLoud
{
public:
    MakeItLoud() = default;
    ~MakeItLoud() = default;

    /**
     * @brief Compressor mode presets.
     */
    enum class CompressorMode
    {
        Clean,   ///< -20dB threshold, 4:1 ratio, fast attack
        Further, ///< -15dB threshold, 6:1 ratio, slower attack
        Crunchy  ///< -8dB threshold, 5:1 ratio, strong saturation
    };

    /**
     * @brief Prepare the processor for playback.
     * @param spec The DSP processing spec (sample rate, block size, channels)
     */
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        _sampleRate = spec.sampleRate;
        _blockSize = static_cast<int>(spec.maximumBlockSize);

        _inputGain.prepare(spec);
        _boostGain.prepare(spec);
        _preCompressor.prepare(spec);
        _postCompressor.prepare(spec);

        reset();
    }

    /**
     * @brief Reset internal state (used during playback start/stop).
     */
    void reset()
    {
        _inputGain.reset();
        _boostGain.reset();
        _preCompressor.reset();
        _postCompressor.reset();
    }

    /**
     * @brief Process a full audio block.
     * @param buffer Audio buffer to process.
     */
    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        if (!_enabled)
            return;

        if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0)
            return;

        // Wrap buffer into DSP context
        juce::dsp::AudioBlock<SampleType> block(buffer);
        juce::dsp::ProcessContextReplacing<SampleType> context(block);

        // Pre-compression
        _preCompressor.process(context);

        // Input gain and boost gain
        _inputGain.setGainLinear(_inputGainValue);
        _inputGain.process(context);

        _boostGain.setGainLinear(_boostValue);
        _boostGain.process(context);

        // Apply tanh waveshaping to all samples
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                SampleType sample = channelData[i];
                sample = std::tanh(sample * _boostValue);

                if (std::isnan(sample) || std::isinf(sample))
                    sample = SampleType{ 0.0 };

                channelData[i] = sample;
            }
        }

        // Post-compression
        _postCompressor.process(context);
    }

    /**
     * @brief Process a single audio sample (useful for modulation or test code).
     * @param sample Input/output sample
     */
    void processSingleSample(SampleType& sample)
    {
        if (!_enabled)
            return;

        sample = _preCompressor.processSample(sample);
        sample = _inputGain.processSample(sample * _inputGainValue);
        sample = _boostGain.processSample(sample * _boostValue);
        sample = std::tanh(sample);

        if (std::isnan(sample) || std::isinf(sample))
            sample = SampleType{ 0.0 };

        sample = _postCompressor.processSample(sample);
    }

    /**
     * @brief Set the linear boost gain applied before tanh.
     * @param boost Gain in linear space (0.0 to 2.0 typically)
     */
    void setBoost(SampleType boost)
    {
        _boostValue = boost;
    }

    /**
     * @brief Set the linear input gain applied before boost.
     * @param gain Gain in linear space
     */
    void setInputGain(SampleType gain)
    {
        _inputGainValue = gain;
    }

    void setEnabled(bool enabled)
    {
        _enabled = enabled;
    }

    /**
     * @brief Set compressor mode using enum.
     * @param mode CompressorMode (Clean, Further, Crunchy)
     */
    void setCompressorMode(CompressorMode mode)
    {
        switch (mode)
        {
            case CompressorMode::Clean:
                applyCompressorSettings(-20.0f, 4.0f, 10.0f, 80.0f);
                break;
            case CompressorMode::Further:
                applyCompressorSettings(-15.0f, 6.0f, 75.0f, 50.0f);
                break;
            case CompressorMode::Crunchy:
                applyCompressorSettings(-8.0f, 5.0f, 100.0f, 200.0f);
                break;
        }
    }

    /**
     * @brief Set compressor mode using integer (for UI dropdowns).
     * @param mode Integer: 0 = Clean, 1 = Further, 2 = Crunchy
     */
    void setCompressorMode(int mode)
    {
        if (mode == 0){
            _enabled = false;
        } else {
            _enabled = true;
            setCompressorMode(static_cast<CompressorMode>(juce::jlimit(0, 2, mode+1))); // +1 offset from 0->disabled
        }
    }

private:
    /** Applies the same compressor settings to both pre and post compressors. */
    void applyCompressorSettings(float threshold, float ratio, float attack, float release)
    {
        auto apply = [&](juce::dsp::Compressor<SampleType>& comp)
        {
            comp.setThreshold(threshold);
            comp.setRatio(ratio);
            comp.setAttack(attack);
            comp.setRelease(release);
        };

        apply(_preCompressor);
        apply(_postCompressor);
    }

    // DSP modules
    juce::dsp::Gain<SampleType> _inputGain;
    juce::dsp::Gain<SampleType> _boostGain;
    juce::dsp::Compressor<SampleType> _preCompressor;
    juce::dsp::Compressor<SampleType> _postCompressor;

    // Parameters
    SampleType _boostValue { 1.0 };
    SampleType _inputGainValue { 1.0 };
    double _sampleRate { 44100.0 };
    int _blockSize { 512 };
    bool _enabled { true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MakeItLoud<SampleType>)
};

} // namespace Effects
} // namespace DSP
