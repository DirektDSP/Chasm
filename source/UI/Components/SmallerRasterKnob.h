#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "UI/Utils/CustomFont.h"

namespace UI
{
    namespace Components
    {
        class SmallerRasterKnob : public juce::Slider
        {
        public:
            SmallerRasterKnob() : juce::Slider (SliderStyle::RotaryHorizontalVerticalDrag, TextEntryBoxPosition::NoTextBox)
            {
                setMouseCursor (juce::MouseCursor::PointingHandCursor);
                setLookAndFeel (&mainSliderLookAndFeel);
            }

            ~SmallerRasterKnob()
            {
                setLookAndFeel (nullptr);
            }

            class SmallerRasterKnobLookAndFeel : public juce::LookAndFeel_V4
            {
            public:
                SmallerRasterKnobLookAndFeel()
                {
                    image = juce::ImageCache::getFromMemory (BinaryData::SmallerKnob1_png, BinaryData::SmallerKnob1_pngSize);
                    customFont = UI::Utils::getCustomFont();
                }

                ~SmallerRasterKnobLookAndFeel() override
                {
                }

                void drawRotarySlider (
                    juce::Graphics& g,
                    int x,
                    int y,
                    int width,
                    int height,
                    float sliderPosProportional,
                    float /*rotaryStartAngle*/,
                    float /*rotaryEndAngle*/,
                    juce::Slider& /*slider*/) override
                {
                    const auto frames = 128;
                    const auto frameId = static_cast<int> (ceil (sliderPosProportional * (static_cast<float> (frames) - 1.0f)));

                    // image is vertical strip, each frame is 1/100th of the height

                    g.drawImage (image,
                        x,
                        y,
                        width,
                        height,
                        0,
                        frameId * image.getHeight() / frames,
                        image.getWidth(),
                        image.getHeight() / frames
                    );
                }

            private:
                juce::Image image;
                juce::Image byImage;

                juce::Font customFont;

                bool bp = false;

                JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmallerRasterKnobLookAndFeel)
            } mainSliderLookAndFeel;

        private:
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmallerRasterKnob)
        };

    }
}