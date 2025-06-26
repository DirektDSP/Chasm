#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "moonbase_JUCEClient/moonbase_JUCEClient.h"
#include "BinaryData.h"
#include "PresetManager.h"
#include "DSP/ChasmDSP.h"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() noexcept override;

    MOONBASE_DECLARE_LICENSING_NAMED(BinaryData, "DirektDSP", "plugintemplate", VERSION)

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


    Service::PresetManager& getPresetManager() { return *presetManager; }

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        
        // Input/Output Gains (-24 to +24 dB, Linear, 5ms smoothing)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"INPUT_GAIN", 1}, "Input Gain", 
            juce::NormalisableRange<float>(-48.0f, 24.0f, 0.1f), 0.0f));
            
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"OUTPUT_GAIN", 1}, "Output Gain", 
            juce::NormalisableRange<float>(-48.0f, 24.0f, 0.1f), 0.0f));
        
        // Mix (0 to 100%, Linear, 20ms smoothing)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"MIX", 1}, "Mix", 
            juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f));
        
        // Delay (1 to 100ms, Logarithmic, 50ms smoothing)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"DELAY", 1}, "Delay", 
            juce::NormalisableRange<float>(1.0f, 100.0f, 0.1f, 0.5f), 30.0f));
        
        // Brightness (-12 to +12 dB, Linear, 10ms smoothing)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"BRIGHTNESS", 1}, "Brightness", 
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
        
        // Character (0.1 to 10 Q, Logarithmic, 10ms smoothing)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"CHARACTER", 1}, "Character", 
            juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f), 1.0f));
        
        // Low Cut Logarithmic (20hz to 20khz, Linear, 20ms smoothing)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"LOW_CUT", 1},
            "Low Cut", 
            juce::NormalisableRange<float>(0.0f, 20000.0f, 0.1f, 0.2f),
            0.0f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {
                if (value <= 0.0f)
                    return juce::String("Off");
                return juce::String(juce::roundToInt(value)) + " Hz";
            },
            [](const juce::String& text) {
                if (text.compareIgnoreCase("off") == 0)
                    return 0.0f;
                return text.getFloatValue();
            }
        ));
        
        // High Cut Logarithmic (20hz to 20khz, Linear, 20ms smoothing)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"HIGH_CUT", 1}, "High Cut",
            juce::NormalisableRange<float>(0.0f, 20000.0f, 0.1f, 0.2f), 20000.0f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {
                if (value >= 20000.0f)
                    return juce::String("Off");
                return juce::String(juce::roundToInt(value)) + " Hz";
            },
            [](const juce::String& text) {
                if (text.compareIgnoreCase("off") == 0)
                    return 20000.0f;
                return text.getFloatValue();
            }
        ));

        // Width (0 to 200%, Linear, 20ms smoothing)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"WIDTH", 1}, "Width", 
            juce::NormalisableRange<float>(0.0f, 200.0f, 0.1f), 100.0f));
        
        // Keep bypass for compatibility
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"BYPASS", 1}, "Bypass", false));

        // MakeItLoud Parameters

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"MIL_ENABLED", 1}, "MakeItLoud Enabled", false));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"MIL_INPUT_GAIN", 1}, "MakeItLoud Input Gain",
            juce::NormalisableRange<float>(-6.0f, 6.0f, 0.1f), 0.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"MIL_BOOST_VALUE", 1}, "MakeItLoud Boost Value",
            juce::NormalisableRange<float>(0.0f, 2.0f, 0.1f), 0.0f));
        
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{"MIL_MODE", 1}, "MakeItLoud Mode",
            juce::StringArray{"Clean", "Further", "Crunchy"}, 0));
        
        return { params.begin(), params.end() };
    }

private:

    std::unique_ptr<Service::PresetManager> presetManager;

    // DSP Processor
    DSP::FloatProcessor dspProcessor;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
