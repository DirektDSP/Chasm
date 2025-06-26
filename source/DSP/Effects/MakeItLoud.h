#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace DSP {
namespace Effects {

/*
    Implementation of MakeItLoud effect chain.
    Consists of:
    - Pre Boost Compressor (Mode Dropdown)
    - Waveshaper
    - Post Boost Compressor (Mode Dropdown)

    Both compressors are set to the same settings.
    Waveshaper implemented as tanh (input * boost * inGain) originally

    Should be adapted to be implemented as follows:
    Input.applyGain(inGain);
    Tanh(Input * Boost);
    to ensure decibel accurate gain


    When implementing, add the following to your apvts:
    - Boost (float, 0.0 to 2.0, default 1.0)
    - Pre Compressor Mode (int, 0 to 2, default 0)
    - Post Compressor Mode (int, 0 to 2, default 0)

    The compressor modes are:
    - Clean: Threshold -20dB, Ratio 4:1, Attack 10ms, Release 80ms
    - Further: Threshold -15dB, Ratio 6:1, Attack 75ms, Release 50ms
    - Crunchy: Threshold -8dB, Ratio 5:1, Attack 100ms, Release 200ms

*/

template<typename SampleType>
class MakeItLoud
{
    public:
    MakeItLoud() = default;
    ~MakeItLoud() = default;

    enum class CompressorMode
    {
        Clean,
        Further,
        Crunchy
    };

    /** Prepares the effect with sample rate and block size. */
    void prepare(double newSampleRate, int blockSize)
    {
        _sampleRate = newSampleRate;
        _blockSize = blockSize;

        // Prepare the compressors
        _preCompressor.prepare({newSampleRate, static_cast<uint32>(blockSize), 2});
        _postCompressor.prepare({newSampleRate, static_cast<uint32>(blockSize), 2});

        reset();
    }

    /** Resets the internal state of the effect. */
    void reset()
    {
        _preCompressor.reset();
        _postCompressor.reset();
    }

    /** Processes a buffer of audio samples. */
    void processBlock(juce::AudioBuffer<SampleType>& buffer)
    {
        if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0)
            return;

        // create processing context
        juce::dsp::AudioBlock<SampleType> block(buffer);
        juce::dsp::ProcessContextReplacing<SampleType> context(block);

        // Apply pre-compression
        _preCompressor.process(context);

        // Apply boost gain
        _boostGain.setGainLinear(_boost);
        _boostGain.process(context);

        // Apply waveshaping
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                channelData[i] = tanh(channelData[i]);
            }
        }
        // Apply post-compression
        _postCompressor.process(context);
    }

    void processSingleSample(SampleType& sample)
    {
        // Apply pre-compression
        _preCompressor.processSample(sample);

        // Apply boost gain
        _boostGain.setGainLinear(_boost);
        sample = _boostGain.processSample(sample);

        // Ensure sample is not NaN or Inf
        if (std::isnan(sample) || std::isinf(sample))
            sample = SampleType{0.0};

        // Apply waveshaping
        sample = tanh(sample);

        // Apply post-compression
        _postCompressor.processSample(sample);
    }


    /** Sets the boost factor. */
    void setBoost(SampleType boost)
    {
        _boost = boost;
    }

    /** Sets the pre-compressor mode. */
    void setCompressorMode(CompressorMode mode){
        switch (mode)
        {
        case CompressorMode::Clean:
            _preCompressor.setThreshold(-20.0f);
            _preCompressor.setRatio(4.0f);
            _preCompressor.setAttack(10.0f);
            _preCompressor.setRelease(80.0f);

            _postCompressor.setThreshold(-20.0f);
            _postCompressor.setRatio(4.0f);
            _postCompressor.setAttack(10.0f);
            _postCompressor.setRelease(80.0f);

            break;
        case CompressorMode::Further:
            _preCompressor.setThreshold(-15.0f);
            _preCompressor.setRatio(6.0f);
            _preCompressor.setAttack(75.0f);
            _preCompressor.setRelease(50.0f);

            _postCompressor.setThreshold(-15.0f);
            _postCompressor.setRatio(6.0f);
            _postCompressor.setAttack(75.0f);
            _postCompressor.setRelease(50.0f);

            break;
        case CompressorMode::Crunchy:
            _preCompressor.setThreshold(-8.0f);
            _preCompressor.setRatio(5.0f);
            _preCompressor.setAttack(100.0f);
            _preCompressor.setRelease(200.0f);

            _postCompressor.setThreshold(-8.0f);
            _postCompressor.setRatio(5.0f);
            _postCompressor.setAttack(100.0f);
            _postCompressor.setRelease(200.0f);

            break;
        }
    }

    void setCompressorMode(int mode){
        switch (mode)
        {
        case 0: //clean
            _preCompressor.setThreshold(-20.0f);
            _preCompressor.setRatio(4.0f);
            _preCompressor.setAttack(10.0f);
            _preCompressor.setRelease(80.0f);

            _postCompressor.setThreshold(-20.0f);
            _postCompressor.setRatio(4.0f);
            _postCompressor.setAttack(10.0f);
            _postCompressor.setRelease(80.0f);

            break;
        case 1: // further
            _preCompressor.setThreshold(-15.0f);
            _preCompressor.setRatio(6.0f);
            _preCompressor.setAttack(75.0f);
            _preCompressor.setRelease(50.0f);

            _postCompressor.setThreshold(-15.0f);
            _postCompressor.setRatio(6.0f);
            _postCompressor.setAttack(75.0f);
            _postCompressor.setRelease(50.0f);

            break;
        case 2: // crunchy
            _preCompressor.setThreshold(-8.0f);
            _preCompressor.setRatio(5.0f);
            _preCompressor.setAttack(100.0f);
            _preCompressor.setRelease(200.0f);

            _postCompressor.setThreshold(-8.0f);
            _postCompressor.setRatio(5.0f);
            _postCompressor.setAttack(100.0f);
            _postCompressor.setRelease(200.0f);

            break;
        }
    }

    private:
    double _sampleRate = 44100.0;
    int _blockSize = 512;

    SampleType _boost = SampleType{1.0};

    juce::dsp::Gain<SampleType> _inputGain;
    juce::dsp::Gain<SampleType> _boostGain;

    juce::dsp::Compressor<SampleType> _preCompressor;
    juce::dsp::Compressor<SampleType> _postCompressor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MakeItLoud<SampleType>)
};

} // namespace Effects
} // namespace DSP
