#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"
#include "PresetPanel.h"
#include "UI/Utils/Timestamp.h"

// Include the Moonbase Activation UI header (adjust path if needed)
#include "moonbase_JUCEClient/moonbase_JUCEClient.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Helper functions for UI setup
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText, const juce::String& suffix);
    void setupToggleButton(juce::ToggleButton& button, juce::Label& label, const juce::String& labelText);
    void layoutSliderWithLabel(juce::Slider& slider, juce::Label& label, juce::Rectangle<int> area);
    void layoutToggleWithLabel(juce::ToggleButton& button, juce::Label& label, juce::Rectangle<int> area);
    PluginProcessor& processorRef;

    // A button to show a sample inspector (if needed)
    juce::TextButton inspectButton { "Inspect the UI" };

    TimestampLabel timestampLabel;
    
    // keep aspect ratio when resizing :)
    juce::ComponentBoundsConstrainer constrainer;

    // Licensing activation UI
    std::unique_ptr<Moonbase::JUCEClient::ActivationUI> activationUI;

    // Optional: a sample inspector from the melatonin module
    std::unique_ptr<melatonin::Inspector> inspector;
    // Actual Plugin UI
    Gui::PresetPanel presetPanel;

    // DSP Parameter Controls
    juce::Slider inputGainSlider, outputGainSlider, mixSlider, delaySlider;
    juce::Slider brightnessSlider, characterSlider, lowCutSlider, highCutSlider, widthSlider;
    juce::ToggleButton limiterToggle, bypassToggle;
    
    // Labels for sliders
    juce::Label inputGainLabel, outputGainLabel, mixLabel, delayLabel;
    juce::Label brightnessLabel, characterLabel, lowCutLabel, highCutLabel, widthLabel;
    juce::Label limiterLabel, bypassLabel;

    // APVTS Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> brightnessAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> characterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limiterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
