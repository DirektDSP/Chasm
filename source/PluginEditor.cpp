#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), presetPanel(p.getPresetManager())
{
    // Create the activation UI via the Moonbase client.
    // The activation UI is created using the licensing member from the processor.
    if (processorRef.moonbaseClient != nullptr)
        activationUI.reset(processorRef.moonbaseClient->createActivationUi(*this));


    // Customize the activation UI if it exists.
    if (activationUI)
    {
        // Set welcome text (max 2 lines) for the activation screen.
        activationUI->setWelcomePageText ("MiniDist", "Made by DirektDSP");

        // Set spinner logo from your BinaryData assets.

        //not using, it looks ugly, might change later
        // activationUI->setSpinnerLogo (juce::Drawable::createFromImageData (BinaryData::direktdsp_svg, BinaryData::direktdsp_svgSize));
        // Optionally set company logo (replace CompanyLogo with your drawable class).
        // activationUI->setCompanyLogo (std::make_unique<CompanyLogo>());
    }

    addAndMakeVisible (timestampLabel);
    timestampLabel.setText ("DirektDSP - " + String(__DATE__) + " " + String(__TIME__), juce::dontSendNotification);

    // Add a button to show an inspector (example usage of another module).
    addAndMakeVisible (inspectButton);
    inspectButton.onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }
        inspector->setVisible (true);
    };

    constrainer.setFixedAspectRatio (4.0f/3.0f); // 4:3 aspect ratio

    // Now tell the editor to use this constrainer
    setConstrainer (&constrainer);

    // Allow the editor to be resizable
    setResizable(true, true);
    constrainer.setMinimumSize(400, 300);    addAndMakeVisible(presetPanel);

    // Setup sliders and labels
    setupSlider(inputGainSlider, inputGainLabel, "Input Gain", "dB");
    setupSlider(outputGainSlider, outputGainLabel, "Output Gain", "dB");
    setupSlider(mixSlider, mixLabel, "Mix", "%");
    setupSlider(delaySlider, delayLabel, "Delay", "ms");
    setupSlider(brightnessSlider, brightnessLabel, "Brightness", "dB");
    setupSlider(characterSlider, characterLabel, "Character", "");
    setupSlider(lowCutSlider, lowCutLabel, "Low Cut", "%");
    setupSlider(highCutSlider, highCutLabel, "High Cut", "%");
    setupSlider(widthSlider, widthLabel, "Width", "%");

    // Setup toggle buttons
    setupToggleButton(limiterToggle, limiterLabel, "Limiter");
    setupToggleButton(bypassToggle, bypassLabel, "Bypass");

    // Create APVTS attachments
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "INPUT_GAIN", inputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "OUTPUT_GAIN", outputGainSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "MIX", mixSlider);
    delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "DELAY", delaySlider);
    brightnessAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "BRIGHTNESS", brightnessSlider);
    characterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "CHARACTER", characterSlider);
    lowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "LOW_CUT", lowCutSlider);
    highCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "HIGH_CUT", highCutSlider);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "WIDTH", widthSlider);
    limiterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, "LIMITER", limiterToggle);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.apvts, "BYPASS", bypassToggle);

    setSize (800, 600);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds();
    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION +
                      " running in " + CMAKE_BUILD_TYPE;
    g.drawText (helloWorld, area.removeFromTop (150), juce::Justification::centred, false);
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();

    // Reserve space for preset panel at the top
    presetPanel.setBounds(area.removeFromTop(proportionOfHeight(0.08f)));

    // Reserve space for inspect button at bottom
    inspectButton.setBounds(area.removeFromBottom(50).withSizeKeepingCentre(100, 50));

    // Main DSP controls area
    auto controlsArea = area.reduced(10);

    // Create a 3x4 grid for controls (3 rows, 4 columns)
    auto row1 = controlsArea.removeFromTop(controlsArea.getHeight() / 3);
    auto row2 = controlsArea.removeFromTop(controlsArea.getHeight() / 2);
    auto row3 = controlsArea;

    // Row 1: Gain and Mix controls
    layoutSliderWithLabel(inputGainSlider, inputGainLabel, row1.removeFromLeft(row1.getWidth() / 4));
    layoutSliderWithLabel(outputGainSlider, outputGainLabel, row1.removeFromLeft(row1.getWidth() / 3));
    layoutSliderWithLabel(mixSlider, mixLabel, row1.removeFromLeft(row1.getWidth() / 2));
    layoutSliderWithLabel(delaySlider, delayLabel, row1);

    // Row 2: Character and EQ controls  
    layoutSliderWithLabel(brightnessSlider, brightnessLabel, row2.removeFromLeft(row2.getWidth() / 4));
    layoutSliderWithLabel(characterSlider, characterLabel, row2.removeFromLeft(row2.getWidth() / 3));
    layoutSliderWithLabel(lowCutSlider, lowCutLabel, row2.removeFromLeft(row2.getWidth() / 2));
    layoutSliderWithLabel(highCutSlider, highCutLabel, row2);

    // Row 3: Width and toggles
    layoutSliderWithLabel(widthSlider, widthLabel, row3.removeFromLeft(row3.getWidth() / 4));
    layoutToggleWithLabel(limiterToggle, limiterLabel, row3.removeFromLeft(row3.getWidth() / 3));
    layoutToggleWithLabel(bypassToggle, bypassLabel, row3.removeFromLeft(row3.getWidth() / 2));

    timestampLabel.setBounds(area.removeFromBottom(20).withSizeKeepingCentre(200, 30));

    // IMPORTANT: Ensure the activation UI is resized as well.
    MOONBASE_RESIZE_ACTIVATION_UI
}

//==============================================================================
// Helper functions for UI setup

void PluginEditor::setupSlider(juce::Slider& slider, juce::Label& label, 
                              const juce::String& labelText, const juce::String& suffix)
{
    addAndMakeVisible(slider);
    addAndMakeVisible(label);
    
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    slider.setTextValueSuffix(" " + suffix);
    
    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.attachToComponent(&slider, false);
}

void PluginEditor::setupToggleButton(juce::ToggleButton& button, juce::Label& label, 
                                   const juce::String& labelText)
{
    addAndMakeVisible(button);
    addAndMakeVisible(label);
    
    button.setButtonText(labelText);
    
    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.attachToComponent(&button, false);
}

void PluginEditor::layoutSliderWithLabel(juce::Slider& slider, juce::Label& label, 
                                        juce::Rectangle<int> area)
{
    auto labelHeight = 20;
    label.setBounds(area.removeFromTop(labelHeight));
    slider.setBounds(area.reduced(5));
}

void PluginEditor::layoutToggleWithLabel(juce::ToggleButton& button, juce::Label& label, 
                                        juce::Rectangle<int> area)
{
    auto labelHeight = 20;
    label.setBounds(area.removeFromTop(labelHeight));
    button.setBounds(area.reduced(5));
}
