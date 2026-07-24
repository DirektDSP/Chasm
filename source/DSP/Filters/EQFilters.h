#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace DSP
{
    namespace Filters
    {

        /**
 * High shelf filter for brightness.
 */
        template <typename SampleType>
        class BrightnessEQ
        {
        public:
            BrightnessEQ() = default;

            /** Prepares the EQ with sample rate. */
            void prepare (const juce::dsp::ProcessSpec& spec)
            {
                sampleRate = spec.sampleRate;
                highShelfFilterDuplicator.prepare (spec);
                reset();
            }

            /** Sets the brightness amount in dB (-12 to +12). */
            void setBrightness (SampleType brightnessDb)
            {
                brightnessDb = juce::jlimit (SampleType { -12.0 }, SampleType { 12.0 }, brightnessDb);

                // High shelf filter at 3kHz
                auto coeffs = juce::dsp::IIR::Coefficients<SampleType>::makeHighShelf (
                    sampleRate,
                    SampleType { 3000.0 }, // 3kHz cutoff
                    static_cast<SampleType> (1.2), // Q factor
                    static_cast<SampleType> (juce::Decibels::decibelsToGain (brightnessDb)));

                *highShelfFilterDuplicator.state = *coeffs;
            }

            /** Processes a block of samples. */
            void processBlock (juce::AudioBuffer<SampleType>& buffer)
            {
                juce::dsp::AudioBlock<SampleType> block (buffer);
                juce::dsp::ProcessContextReplacing<SampleType> context (block);
                highShelfFilterDuplicator.process (context);
            }

            /** Resets the filter state. */
            void reset()
            {
                highShelfFilterDuplicator.reset();
            }

        private:
            juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<SampleType>,
                juce::dsp::IIR::Coefficients<SampleType>>
                highShelfFilterDuplicator;
            double sampleRate = 44100.0;
        };

    } // namespace Filters
} // namespace DSP
