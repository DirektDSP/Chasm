#pragma once

#include <juce_graphics/juce_graphics.h>

namespace UI
{
    namespace Utils
    {
        static const juce::Font getCustomFont()
        {
            static auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::Rovaney_ttf, BinaryData::Rovaney_ttfSize);
            juce::Font font (typeface);
            font.setHeight(20.0f);
            
            return font;
        }

        // class CustomFontLookAndFeel : public juce::LookAndFeel_V4
        // {
        // public:
        //     CustomFontLookAndFeel()
        //     {
        //         customFont = UI::Utils::getCustomFont();
        //     }

        //     ~CustomFontLookAndFeel() = default;

        // private:
        //     juce::Font customFont;

        //     JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomFontLookAndFeel)
        // } customFontLookAndFeel;
    }
}
