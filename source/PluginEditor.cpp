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
    constrainer.setMinimumSize(400, 300);

    addAndMakeVisible(presetPanel);

    setSize (400, 300);
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

    presetPanel.setBounds(getLocalBounds().removeFromTop(proportionOfHeight(0.1f)));

    // Layout the child components.
    auto area = getLocalBounds();
    inspectButton.setBounds (area.removeFromBottom (50).withSizeKeepingCentre (100, 50));

    // IMPORTANT: Ensure the activation UI is resized as well.
    MOONBASE_RESIZE_ACTIVATION_UI
}
