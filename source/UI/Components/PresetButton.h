#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
namespace UI
{
    namespace Components
    {
        class PresetButton : public juce::TextButton
        {
        public:
            PresetButton() : juce::TextButton ("")
            {
                setLookAndFeel (&mainButtonLookAndFeel);
            }

            ~PresetButton()
            {
                setLookAndFeel (nullptr);
            }

        private:
            class PresetButtonLookAndFeel : public juce::LookAndFeel_V4
            {
            public:
                PresetButtonLookAndFeel()
                {
                    bgColour = juce::Colour::fromRGB (173, 173, 173);
                }

                void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown) override
                {
                    // draw flat background colour without any rounded borders
                    g.setColour (bgColour);
                    g.fillAll();
                }

            private:
                juce::Colour bgColour;

                JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetButtonLookAndFeel)
            } mainButtonLookAndFeel;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetButton)
        };
    }
}