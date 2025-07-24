#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

// DSP Components
#include "../Effects/MakeItLoud.h"
#include "../Effects/StereoEnhancer.h"
#include "../Effects/StereoEnhancer.h"
#include "../Effects/HaasEffect.h"
#include "../Filters/EQFilters.h"
#include "../Filters/SchroederAllpassChain.h"
#include "../Utils/DSPUtils.h"
#include "../Utils/ParameterSmoother.h"

namespace DSP
{
    namespace Core
    {

        template <typename SampleType>
        class ChasmDSPProcessor
        {
        public:
            ChasmDSPProcessor()
            {
                using FilterType = juce::dsp::StateVariableTPTFilterType;

                lowCutFilter.setType (FilterType::highpass);
                highCutFilter.setType (FilterType::lowpass);

                lowCutFilter.setResonance (static_cast<SampleType> (0.707));
                highCutFilter.setResonance (static_cast<SampleType> (0.707));
            }

            void prepare (const juce::dsp::ProcessSpec& spec,
                          SampleType inputGain = SampleType{1.0},
                          SampleType outputGain = SampleType{1.0},
                          SampleType mix = SampleType{0.5},
                          SampleType delayMs = SampleType{30.0},
                          SampleType brightness = SampleType{0.0},
                          SampleType character = SampleType{1.0},
                          SampleType lowCut = SampleType{0.0},
                          SampleType highCut = SampleType{0.0},
                          SampleType width = SampleType{100.0},
                          SampleType mil_InputGain = SampleType{1.0},
                          SampleType mil_BoostValue = SampleType{0.0},
                          int mil_Mode = 0,
                          SampleType haasAmount = SampleType{0.0})
            {
                sampleRate = spec.sampleRate;
                samplesPerBlock = static_cast<int> (spec.maximumBlockSize);
                numChannels = static_cast<int> (spec.numChannels);

                leftAllpassChain.prepare (sampleRate, delayMs, character);
                rightAllpassChain.prepare (sampleRate, delayMs, character);
                brightnessEQ.prepare (spec);

                stereoEnhancer.setWidth (SampleType { 100.0 });

                haasEffect.prepare(sampleRate, samplesPerBlock);

                prepareParameterSmoothers();

                // Snap all smoothers to current values to avoid ramping artifacts
                inputGainSmoother.setTargetValue(inputGain);
                inputGainSmoother.snapToTargetValue();
                outputGainSmoother.setTargetValue(outputGain);
                outputGainSmoother.snapToTargetValue();
                mixSmoother.setTargetValue(mix);
                mixSmoother.snapToTargetValue();
                delaySmoother.setTargetValue(delayMs);
                delaySmoother.snapToTargetValue();
                brightnessSmoother.setTargetValue(brightness);
                brightnessSmoother.snapToTargetValue();
                characterSmoother.setTargetValue(character);
                characterSmoother.snapToTargetValue();
                lowCutSmoother.setTargetValue(lowCut);
                lowCutSmoother.snapToTargetValue();
                highCutSmoother.setTargetValue(highCut);
                highCutSmoother.snapToTargetValue();
                widthSmoother.setTargetValue(width);
                widthSmoother.snapToTargetValue();
                haasSmoother.setTargetValue(haasAmount);
                haasSmoother.snapToTargetValue();
                mil_InputGainSmoother.setTargetValue(mil_InputGain);
                mil_InputGainSmoother.snapToTargetValue();
                mil_BoostSmoother.setTargetValue(mil_BoostValue);
                mil_BoostSmoother.snapToTargetValue();
                makeItLoud.setCompressorMode(mil_Mode);

                wetBuffer.setSize (numChannels, samplesPerBlock);
                dryBuffer.setSize (numChannels, samplesPerBlock);

                lowCutFilter.prepare (spec);
                lowCutFilter.reset();

                highCutFilter.prepare (spec);
                highCutFilter.reset();

                makeItLoud.prepare (spec);

                reset(inputGain, outputGain, mix, delayMs, brightness, character, lowCut, highCut, width, mil_InputGain, mil_BoostValue, mil_Mode, haasAmount);
            }

            void updateParameters (SampleType inputGainDb, SampleType outputGainDb, SampleType mixPercent, SampleType delayMs, SampleType brightnessDb, SampleType characterQ, SampleType lowCutPercent, SampleType highCutPercent, SampleType widthPercent, SampleType mil_InputGain, SampleType mil_BoostValue, int mil_Mode, SampleType haasAmount)
            {
                inputGainSmoother.setTargetValue (Utils::DSPUtils::dbToGain (inputGainDb));
                outputGainSmoother.setTargetValue (Utils::DSPUtils::dbToGain (outputGainDb));
                mixSmoother.setTargetValue (Utils::DSPUtils::percentageToNormalized (mixPercent));
                delaySmoother.setTargetValue (delayMs);
                brightnessSmoother.setTargetValue (brightnessDb);
                characterSmoother.setTargetValue (characterQ);
                lowCutSmoother.setTargetValue (lowCutPercent);
                highCutSmoother.setTargetValue (highCutPercent);
                widthSmoother.setTargetValue (widthPercent);

                haasSmoother.setTargetValue(haasAmount);

                mil_BoostSmoother.setTargetValue (Utils::DSPUtils::dbToGain (mil_BoostValue));
                mil_InputGainSmoother.setTargetValue (Utils::DSPUtils::dbToGain (mil_InputGain));
                makeItLoud.setCompressorMode (mil_Mode);
            }

            void processBlock (juce::AudioBuffer<SampleType>& buffer)
            {
                jassert (buffer.getNumChannels() >= 1);

                const int numSamples = buffer.getNumSamples();

                if (wetBuffer.getNumSamples() != numSamples)
                {
                    wetBuffer.setSize (numChannels, numSamples, false, false, true);
                    dryBuffer.setSize (numChannels, numSamples, false, false, true);
                }

                dryBuffer.makeCopyOf (buffer);

                for (int i = 0; i < numSamples; ++i)
                {
                    const auto inputGain = inputGainSmoother.getNextValue();
                    const auto delay = delaySmoother.getNextValue();
                    const auto brightness = brightnessSmoother.getNextValue();
                    const auto character = characterSmoother.getNextValue();
                    const auto width = widthSmoother.getNextValue();
                    const auto haasAmount = haasSmoother.getNextValue();

                    auto lowCutFreq = lowCutSmoother.getNextValue();
                    if (!juce::approximatelyEqual (lowCutFreq, lastLowCut))
                    {
                        lowCutFreq = juce::jlimit(lowCutMin, lowCutMax, lowCutFreq);
                        lowCutFilter.setCutoffFrequency (lowCutFreq);
                        lowCutActive = lowCutFreq > SampleType { 1.0 };
                        lastLowCut = lowCutFreq;
                    }

                    auto highCutFreq = highCutSmoother.getNextValue();
                    if (!juce::approximatelyEqual (highCutFreq, lastHighCut))
                    {
                        highCutFreq = juce::jlimit(highCutMin, highCutMax, highCutFreq);
                        highCutFilter.setCutoffFrequency (highCutFreq);
                        highCutActive = highCutFreq < SampleType { 19999.0 };
                        lastHighCut = highCutFreq;
                    }

                    if (i == 0 || shouldUpdateDSPComponents (i))
                        updateDSPComponents (delay, brightness, character, width, haasAmount);

                    if (buffer.getNumChannels() == 1)
                    {
                        // mono
                        processSingleSampleMono (buffer, i, inputGain);
                    }
                    else
                    {
                        // something other than mono, assume stereo
                        processSingleSample (buffer, i, inputGain);
                    }
                }

                // After allpasschains to tr regain some high end.
                brightnessEQ.processBlock (wetBuffer);

                // Apply stereo enhancement
                haasEffect.processBlock(wetBuffer);
                stereoEnhancer.processBlock (wetBuffer);

                // Apply MakeItLoud effect
                makeItLoud.setInputGain (mil_InputGainSmoother.getNextValue());
                makeItLoud.setBoost (mil_BoostSmoother.getNextValue());
                makeItLoud.processBlock (wetBuffer);

                for (int i = 0; i < numSamples; ++i)
                {
                    const auto mix = mixSmoother.getNextValue();
                    const auto outputGain = outputGainSmoother.getNextValue();
                    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                    {
                        auto* channelData = buffer.getWritePointer (channel);
                        const auto drySample = dryBuffer.getSample (channel, i);
                        const auto wetSample = wetBuffer.getSample (channel, i);
                        channelData[i] = (drySample * (SampleType { 1.0 } - mix) + wetSample * mix) * outputGain;
                    }
                }
            }

            void reset(SampleType inputGain = SampleType{1.0},
                       SampleType outputGain = SampleType{1.0},
                       SampleType mix = SampleType{0.5},
                       SampleType delayMs = SampleType{30.0},
                       SampleType brightness = SampleType{0.0},
                       SampleType character = SampleType{1.0},
                       SampleType lowCut = SampleType{0.0},
                       SampleType highCut = SampleType{0.0},
                       SampleType width = SampleType{100.0},
                       SampleType mil_InputGain = SampleType{1.0},
                       SampleType mil_BoostValue = SampleType{0.0},
                       int mil_Mode = 0,
                       SampleType haasAmount = SampleType{0.0})
            {
                leftAllpassChain.reset(delayMs, character);
                rightAllpassChain.reset(delayMs, character);
                brightnessEQ.reset();
                stereoEnhancer.reset();

                inputGainSmoother.reset(inputGain);
                inputGainSmoother.setTargetValue(inputGain);
                inputGainSmoother.snapToTargetValue();
                outputGainSmoother.reset(outputGain);
                outputGainSmoother.setTargetValue(outputGain);
                outputGainSmoother.snapToTargetValue();
                mixSmoother.reset(mix);
                mixSmoother.setTargetValue(mix);
                mixSmoother.snapToTargetValue();
                delaySmoother.reset(delayMs);
                delaySmoother.setTargetValue(delayMs);
                delaySmoother.snapToTargetValue();
                brightnessSmoother.reset(brightness);
                brightnessSmoother.setTargetValue(brightness);
                brightnessSmoother.snapToTargetValue();
                characterSmoother.reset(character);
                characterSmoother.setTargetValue(character);
                characterSmoother.snapToTargetValue();
                lowCutSmoother.reset(lowCut);
                lowCutSmoother.setTargetValue(lowCut);
                lowCutSmoother.snapToTargetValue();
                highCutSmoother.reset(highCut);
                highCutSmoother.setTargetValue(highCut);
                highCutSmoother.snapToTargetValue();
                widthSmoother.reset(width);
                widthSmoother.setTargetValue(width);
                widthSmoother.snapToTargetValue();
                haasSmoother.reset(haasAmount);
                haasSmoother.setTargetValue(haasAmount);
                haasSmoother.snapToTargetValue();
                mil_BoostSmoother.reset(mil_BoostValue);
                mil_BoostSmoother.setTargetValue(mil_BoostValue);
                mil_BoostSmoother.snapToTargetValue();
                mil_InputGainSmoother.reset(mil_InputGain);
                mil_InputGainSmoother.setTargetValue(mil_InputGain);
                mil_InputGainSmoother.snapToTargetValue();
                makeItLoud.setCompressorMode(mil_Mode);
            }

        private:
            void prepareParameterSmoothers()
            {
                inputGainSmoother.prepare (sampleRate, 1.0);
                outputGainSmoother.prepare (sampleRate, 1.0);
                mixSmoother.prepare (sampleRate, 5.0);
                delaySmoother.prepare (sampleRate, 5.0);
                brightnessSmoother.prepare (sampleRate, 5.0);
                characterSmoother.prepare (sampleRate, 5.0);
                lowCutSmoother.prepare (sampleRate, 5.0);
                highCutSmoother.prepare (sampleRate, 5.0);
                widthSmoother.prepare (sampleRate, 5.0);
                haasSmoother.prepare (sampleRate, 1.0);

                mil_BoostSmoother.prepare (sampleRate, 1.0);
                mil_InputGainSmoother.prepare (sampleRate, 1.0);
            }

            bool shouldUpdateDSPComponents (int sampleIndex) { return (sampleIndex % 32) == 0; }

            void updateDSPComponents (SampleType delay, SampleType brightness, SampleType character, SampleType width, SampleType haasAmount)
            {
                leftAllpassChain.setDelayTime (delay);
                rightAllpassChain.setDelayTime (delay);
                leftAllpassChain.setCharacter (character);
                rightAllpassChain.setCharacter (character);
                brightnessEQ.setBrightness (brightness);
                stereoEnhancer.setWidth (width);
                haasEffect.setDelayMs(haasAmount);
            }

            void processSingleSample (juce::AudioBuffer<SampleType>& buffer, int sampleIndex, SampleType inputGain)
            {
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                    buffer.getWritePointer (channel)[sampleIndex] *= inputGain;

                for (int channel = 0; channel < juce::jmin (wetBuffer.getNumChannels(), buffer.getNumChannels()); ++channel)
                    wetBuffer.setSample (channel, sampleIndex, buffer.getSample (channel, sampleIndex));

                auto leftSample = wetBuffer.getSample (0, sampleIndex);
                auto rightSample = wetBuffer.getSample (1, sampleIndex);

                leftSample = leftAllpassChain.processSample (leftSample);
                rightSample = rightAllpassChain.processSample (rightSample);

                if (lowCutActive)
                {
                    leftSample = lowCutFilter.processSample (0, leftSample);
                    rightSample = lowCutFilter.processSample (1, rightSample);
                }

                if (highCutActive)
                {
                    leftSample = highCutFilter.processSample (0, leftSample);
                    rightSample = highCutFilter.processSample (1, rightSample);
                }

                wetBuffer.setSample (0, sampleIndex, leftSample);
                wetBuffer.setSample (1, sampleIndex, rightSample);
            }

            void processSingleSampleMono (juce::AudioBuffer<SampleType>& buffer, int sampleIndex, SampleType inputGain)
            {
                buffer.getWritePointer (0)[sampleIndex] *= inputGain;
                // since we setup the wetbuffer using numChannels, we dont need to worry about having more than one in mono
                wetBuffer.setSample (0, sampleIndex, buffer.getSample (0, sampleIndex));

                SampleType workSample = wetBuffer.getSample (0, sampleIndex);
                workSample = leftAllpassChain.processSample (workSample); // default to left since in mono does it even matter

                if (lowCutActive)
                {
                    workSample = lowCutFilter.processSample (0, workSample);
                }
                if (highCutActive)
                {
                    workSample = highCutFilter.processSample (0, workSample);
                }

                wetBuffer.setSample (0, sampleIndex, workSample);
            }

            Filters::SchroederAllpassChain<SampleType> leftAllpassChain;
            Filters::SchroederAllpassChain<SampleType> rightAllpassChain;
            Filters::BrightnessEQ<SampleType> brightnessEQ;
            Effects::StereoEnhancer<SampleType> stereoEnhancer;
            DSP::Effects::HaasEffect<SampleType> haasEffect;

            juce::dsp::StateVariableTPTFilter<SampleType> lowCutFilter;
            juce::dsp::StateVariableTPTFilter<SampleType> highCutFilter;

            Utils::ParameterSmoother<SampleType> inputGainSmoother;
            Utils::ParameterSmoother<SampleType> outputGainSmoother;
            Utils::ParameterSmoother<SampleType> mixSmoother;
            Utils::ParameterSmoother<SampleType> delaySmoother;
            Utils::ParameterSmoother<SampleType> brightnessSmoother;
            Utils::ParameterSmoother<SampleType> characterSmoother;
            Utils::ParameterSmoother<SampleType> lowCutSmoother;
            Utils::ParameterSmoother<SampleType> highCutSmoother;
            Utils::ParameterSmoother<SampleType> widthSmoother;
            Utils::ParameterSmoother<SampleType> haasSmoother;

            // MakeItLoud

            Effects::MakeItLoud<SampleType> makeItLoud;

            Utils::ParameterSmoother<SampleType> mil_InputGainSmoother;
            Utils::ParameterSmoother<SampleType> mil_BoostSmoother;

            juce::AudioBuffer<SampleType> wetBuffer;
            juce::AudioBuffer<SampleType> dryBuffer;

            double sampleRate = 44100.0;
            int samplesPerBlock = 512;
            int numChannels = 2;

            SampleType lastLowCut = SampleType { 0.0 };
            SampleType lastHighCut = SampleType { 0.0 };

            SampleType lowCutMin = 0;
            SampleType highCutMin = 0;

            // subtract one to ensure that the value is always
            // LOWER than the upper limit
            SampleType lowCutMax = (sampleRate*0.5) - 1;
            SampleType highCutMax = (sampleRate*0.5) - 1;


            bool lowCutActive = false;
            bool highCutActive = false;
        };

    } // namespace Core
} // namespace DSP
