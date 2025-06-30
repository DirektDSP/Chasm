#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace UI
{
    namespace Components
    {
        class LedButton : public juce::ToggleButton
        {
        public:
            LedButton() : juce::ToggleButton ("")
            {
                setLookAndFeel (&mainButtonLookAndFeel);
            }

            ~LedButton()
            {
                setLookAndFeel (nullptr);
            }

        private:
            class LedButtonLookAndFeel : public juce::LookAndFeel_V4
            {
            public:
                LedButtonLookAndFeel()
                {
                    image = juce::ImageCache::getFromMemory (BinaryData::Dual_LED_png, BinaryData::Dual_LED_pngSize);
                }

                void drawTickBox (juce::Graphics& g, juce::Component& component, float x, float y, float w, float h, bool ticked, bool isEnabled, bool isMouseOverButton, bool isButtonDown) override
                {
                    const auto frames = 2;
                    const auto frameId = static_cast<int> (ticked);

                    juce::Rectangle<float> Bounds (x, y, w, h);

                    g.drawImage (image,
                        x,
                        y,
                        w,
                        h,
                        0,
                        frameId * image.getHeight() / frames,
                        image.getWidth(),
                        image.getHeight() / frames);
                }

            private:
                juce::Image image;

                JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LedButtonLookAndFeel)
            } mainButtonLookAndFeel;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LedButton)
        };
    }
}