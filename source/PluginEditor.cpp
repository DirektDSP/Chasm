#include "PluginEditor.h"
#include "UI/Utils/CustomFont.h"

int TEXT_BOX_WIDTH = 80;
int TEXT_BOX_HEIGHT = 20;

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), presetPanel(p.getPresetManager()), apvts(p.getApvts())
{
    // Create the activation UI via the Moonbase client.
    // The activation UI is created using the licensing member from the processor.
    if (processorRef.moonbaseClient != nullptr)
        activationUI.reset(processorRef.moonbaseClient->createActivationUi(*this));


    // Customize the activation UI if it exists.
    if (activationUI)
    {
        // Set welcome text (max 2 lines) for the activation screen.
        activationUI->setWelcomePageText ("Chasm", "Made by Korzana & DirektDSP");

        // Set spinner logo from your BinaryData assets.

        //not using, it looks ugly, might change later
        // activationUI->setSpinnerLogo (juce::Drawable::createFromImageData (BinaryData::direktdsp_svg, BinaryData::direktdsp_svgSize));
        // Optionally set company logo (replace CompanyLogo with your drawable class).
        // activationUI->setCompanyLogo (std::make_unique<CompanyLogo>());
    }


    addAndMakeVisible(bg);

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

    constrainer.setFixedAspectRatio (2000.0f/1200.0f);

    // Now tell the editor to use this constrainer
    setConstrainer (&constrainer);

    // Allow the editor to be resizable
    setResizable(true, true);
    constrainer.setMinimumSize(800, 420);    addAndMakeVisible(presetPanel);

    // Setup sliders and labels
    setupSlider(inputGainSlider, inputGainLabel, "Input", "dB");
    setupSlider(outputGainSlider, outputGainLabel, "Output", "dB");
    setupSlider(mixSlider, mixLabel, "Mix", "%");
    setupSlider(delaySlider, delayLabel, "", "ms");
    setupSlider(brightnessSlider, brightnessLabel, "Brightness", "dB");
    setupSlider(characterSlider, characterLabel, "Character", "");
    setupSlider(widthSlider, widthLabel, "Width", "%");
    setupSlider(haasSlider, haasLabel, "Haas", "%");
    
    // use suffix-less functions since these can be "off"
    setupSlider(lowCutSlider, lowCutLabel, "Low Cut");
    setupSlider(highCutSlider, highCutLabel, "High Cut");

    setupSlider(mil_inGain, mil_inGainLabel, "In Gain", "dB");
    setupSlider(mil_boost, mil_boostLabel, "Boost", "dB");
    mil_inGain.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mil_boost.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    
    setupComboBox(mil_modeCB, mil_modeCBLabel, "Mode");

    mil_modeCB.addItemList(juce::StringArray{
        "Off",
        "Clean",
        "Further",
        "Crunchy"
    }, 1);
    mil_modeCB.setSelectedItemIndex(0);

    
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
    haasAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "HAAS", haasSlider);

    mil_modeCBAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processorRef.apvts, "MIL_MODE", mil_modeCB);

    mil_inGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "MIL_INGAIN", mil_inGain);

    mil_boostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "MIL_BOOST", mil_boost);

    setSize (1000, 600);

    delaySlider.addListener(this);
    characterSlider.addListener(this);
}

PluginEditor::~PluginEditor()
{
    delaySlider.removeListener(this);
    characterSlider.removeListener(this);
}

void PluginEditor::paint (juce::Graphics& g)
{
    // g.setOpacity(0.0);
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();
    
    int padding = getWidth() * 0.02; // 20px at 1000x600
    int knobSizeX = getWidth() * 0.08f; // 80x80 at expected res (1000x600)
    int knobSizeY = (getWidth() * 0.08f) + TEXT_BOX_HEIGHT;
    int largeKnobSizeX = getWidth() * 0.36f; // 320x320 at expected res (1000x600)
    int largeKnobSizeY = getWidth() * 0.36f + TEXT_BOX_HEIGHT; // 320x320 at expected res (1000x600)
    
    bg.setBounds(area);
    bg.setLeftFrame(lastLeft);
    bg.setRightFrame(lastRight);
    
    // // Reserve space for preset panel at the top
    // presetPanel.setBounds(area.removeFromTop(proportionOfHeight(0.08f)));
    
    // // Reserve space for inspect button at bottom
    inspectButton.setBounds(area.removeFromBottom(50).withSizeKeepingCentre(100, 50));
    
    // // Main DSP controls area

    // Main Delay Slider
    delaySlider.setBounds(
        (getWidth() / 2) - (largeKnobSizeX/2),
        (getHeight() / 2) - (largeKnobSizeY/2),
        largeKnobSizeX,
        largeKnobSizeY
    );
    
    // In / Out gain & filters
    // Right hand side, 2 on top one on bottom
    // 175 px y
    int topLayerHeight = (getHeight()*0.35f);

    int leftPad = getWidth() *0.1f;
    int rightPad = getWidth() - leftPad - (1.35f*knobSizeX);

    inputGainSlider.setBounds(
        rightPad,
        topLayerHeight,
        knobSizeX,
        knobSizeY
    );

    outputGainSlider.setBounds(
        rightPad-knobSizeX-padding,
        topLayerHeight,
        knobSizeX,
        knobSizeY
    );

    highCutSlider.setBounds(
        rightPad,
        topLayerHeight+knobSizeY+(1.3*padding),
        knobSizeX,
        knobSizeY
    );

    lowCutSlider.setBounds(
        rightPad-knobSizeX-padding,
        topLayerHeight+knobSizeY+(1.3*padding),
        knobSizeX,
        knobSizeY
    );

    mixSlider.setBounds(
        getWidth()-TEXT_BOX_WIDTH-knobSizeX-padding,
        (getHeight()*0.985f)-(knobSizeY)+TEXT_BOX_HEIGHT,
        knobSizeX+TEXT_BOX_WIDTH,
        knobSizeY-TEXT_BOX_HEIGHT
    );


    presetPanel.setBounds(
        0,
        0,
        getWidth()*0.56,
        getHeight()*0.11f
    );

    
    
    // left side
    
    characterSlider.setBounds(
        leftPad*0.75,
        (getHeight()*0.5 - knobSizeX*1.6),
        knobSizeX*1.5,
        knobSizeY*1.5-(TEXT_BOX_HEIGHT/2)
    );
    
    brightnessSlider.setBounds(
        (leftPad*0.75) + (knobSizeX*1.5),
        (getHeight()*0.5 - knobSizeX*1.6),
        knobSizeX*1.5,
        knobSizeY*1.5-(TEXT_BOX_HEIGHT/2)
    );
    
    widthSlider.setBounds(
        leftPad*0.75+TEXT_BOX_WIDTH,
        (getHeight()*0.5) + knobSizeX*0.4,
        knobSizeX+TEXT_BOX_WIDTH,
        knobSizeY-TEXT_BOX_HEIGHT
    );

    haasSlider.setBounds(
        leftPad*0.75+TEXT_BOX_WIDTH,
        (getHeight()*0.5) + knobSizeX*0.4 + (knobSizeX),
        knobSizeX+TEXT_BOX_WIDTH,
        knobSizeY-TEXT_BOX_HEIGHT
    );
    // timestampLabel.setBounds(area.removeFromBottom(20).withSizeKeepingCentre(200, 30));


    // MIL

    auto btmHeight = (getHeight()*0.985f)-(knobSizeY)+TEXT_BOX_HEIGHT;

    mil_boost.setBounds(
        getWidth()/2 + 8*padding,
        btmHeight,
        knobSizeX,
        knobSizeY-TEXT_BOX_HEIGHT
    );

    mil_modeCB.setBounds(
        getWidth()/2.75,
        btmHeight+(knobSizeY/4),
        knobSizeX+TEXT_BOX_WIDTH,
        knobSizeY/4
    );
    

    // IMPORTANT: Ensure the activation UI is resized as well.
    MOONBASE_RESIZE_ACTIVATION_UI
}

//==============================================================================

void PluginEditor::sliderValueChanged(juce::Slider* slider)
{
    float delay = apvts.getRawParameterValue("DELAY")->load();
    float character = apvts.getRawParameterValue("CHARACTER")->load();

    if (slider == &delaySlider) {
        // Normalize delay from [1,100] to [0,1]
        float normalized = (delay - 1.0f) / 99.0f;
        
        // Apply skew with exponent < 1 (e.g. 0.5 = sqrt) to bias towards low half
        float skewed = std::pow(normalized, 1.f);
        
        // Map skewed value to [1, 128]
        int mappedDelay = static_cast<int>(skewed * 127.0f + 1);
        
        bg.setRightFrame(mappedDelay);
        lastRight = mappedDelay;
    }
    
    if (slider == &characterSlider){
        // Linear mapping as before
        int mappedCharacter = static_cast<int>(((character - 0.1f) / 1.9f) * 127 + 1);
        bg.setLeftFrame(mappedCharacter);
        lastLeft = mappedCharacter;
    }
}



//==============================================================================
// Helper functions for UI setup

void PluginEditor::setupSlider(juce::Slider& slider, juce::Label& label, 
                              const juce::String& labelText, const juce::String& suffix)
{
    addAndMakeVisible(slider);
    addAndMakeVisible(label);
    
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, TEXT_BOX_WIDTH, TEXT_BOX_HEIGHT);
    slider.setTextValueSuffix(" " + suffix);
    slider.setColour(juce::Label::textColourId, juce::Colour::fromRGB(47, 47 ,47));
    
    label.setText(labelText, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colour::fromRGB(47, 47 ,47));
    label.setFont(UI::Utils::getCustomFont());
    label.setJustificationType(juce::Justification::centred);
    label.attachToComponent(&slider, false);

    if (
        labelText == "Mix" ||
        labelText == "Width" ||
        labelText == "In Gain" ||
        labelText == "Boost" ||
        labelText == "Haas"
    ){
        label.attachToComponent(&slider, true);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, TEXT_BOX_WIDTH, TEXT_BOX_HEIGHT);
    }
}


void PluginEditor::setupSlider(juce::Slider& slider, juce::Label& label, 
                              const juce::String& labelText)
{
    addAndMakeVisible(slider);
    addAndMakeVisible(label);
    
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    slider.setColour(juce::Label::textColourId, juce::Colour::fromRGB(47, 47 ,47));
    
    label.setText(labelText, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colour::fromRGB(47, 47 ,47));
    label.setFont(UI::Utils::getCustomFont());
    label.setJustificationType(juce::Justification::centred);
    label.attachToComponent(&slider, false);
}

void PluginEditor::setupComboBox(juce::ComboBox& comboBox, juce::Label& label, const juce::String& labelText)
{
    addAndMakeVisible(comboBox);
    addAndMakeVisible(label);

    label.setText(labelText, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colour::fromRGB(47, 47, 47));
    label.setFont(UI::Utils::getCustomFont());
    label.setJustificationType(juce::Justification::centred);
    label.attachToComponent(&comboBox, true);
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
