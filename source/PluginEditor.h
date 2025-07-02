#pragma once

#include "BinaryData.h"
#include "PluginProcessor.h"
#include "UI/Components/AnimatedBackground.h"
#include "UI/Components/PresetPanel.h"
#include "UI/Components/RasterKnob.h"
#include "UI/Components/SmallerRasterKnob.h"
#include "UI/Components/ModeCB.h"
#include "UI/Components/LedButton.h"
#include "UI/Utils/Timestamp.h"
#include "melatonin_inspector/melatonin_inspector.h"

#include <juce_gui_basics/juce_gui_basics.h>

// Include the Moonbase Activation UI header (adjust path if needed)
#include "moonbase_JUCEClient/moonbase_JUCEClient.h"

class PluginEditor : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override; //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    int lastRight, lastLeft;

private:
    
    AudioProcessorValueTreeState& apvts;

    // Helper functions for UI setup
    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& labelText, const juce::String& suffix);
    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& labelText);
    void setupComboBox(juce::ComboBox& comboBox, juce::Label& label, const juce::String& labelText);
    void setupToggleButton (juce::ToggleButton& button, juce::Label& label, const juce::String& labelText);
    void layoutSliderWithLabel (juce::Slider& slider, juce::Label& label, juce::Rectangle<int> area);
    void layoutToggleWithLabel (juce::ToggleButton& button, juce::Label& label, juce::Rectangle<int> area);
    PluginProcessor& processorRef;

    void sliderValueChanged(juce::Slider* slider) override;

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
    UI::Components::AnimatedBackground bg;

    UI::Components::PresetPanel presetPanel;

    // DSP Parameter Controls
    UI::Components::RasterKnob delaySlider;

    UI::Components::SmallerRasterKnob inputGainSlider, outputGainSlider, mixSlider;
    UI::Components::SmallerRasterKnob brightnessSlider, characterSlider, lowCutSlider, highCutSlider, widthSlider, haasSlider;

    // Labels for sliders
    juce::Label inputGainLabel, outputGainLabel, mixLabel, delayLabel;
    juce::Label brightnessLabel, characterLabel, lowCutLabel, highCutLabel, widthLabel, haasLabel;

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
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> haasAttachment;

    // MIL

    juce::Label mil_modeCBLabel, mil_inGainLabel, mil_enabledLabel, mil_boostLabel;

    UI::Components::ModeCB mil_modeCB;
    UI::Components::SmallerRasterKnob mil_inGain, mil_boost;
    UI::Components::LedButton mil_enabled;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mil_modeCBAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mil_inGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mil_boostAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> mil_enabledAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
