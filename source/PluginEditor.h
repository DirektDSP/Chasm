#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"

// Include the Moonbase Activation UI header (adjust path if needed)
#include "moonbase_JUCEClient/moonbase_JUCEClient.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& processorRef;

    // A button to show a sample inspector (if needed)
    juce::TextButton inspectButton { "Inspect the UI" };

    // Licensing activation UI
    std::unique_ptr<Moonbase::JUCEClient::ActivationUI> activationUI;

    // Optional: a sample inspector from the melatonin module
    std::unique_ptr<melatonin::Inspector> inspector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
