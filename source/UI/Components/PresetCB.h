#pragma once

#include "UI/Utils/CustomFont.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace UI
{
    namespace Components
    {
        class PresetCB : public juce::ComboBox
        {
        public:
            PresetCB() : juce::ComboBox ("")
            {
                setLookAndFeel (&mainCBLookAndFeel);
            }

            ~PresetCB()
            {
                setLookAndFeel (nullptr);
            }

        private:
            class PresetCBLookAndFeel : public juce::LookAndFeel_V4
            {
            public:
                PresetCBLookAndFeel()
                {
                    bgColour = juce::Colour::fromRGB (160, 196, 126);
                    bgStroke = juce::Colour::fromRGB (135, 166, 106);
                    customFont = UI::Utils::getCustomFont();
                }

                Font getComboBoxFont (ComboBox& /*box*/) override
                {
                    return getCommonMenuFont();
                }
                Font getPopupMenuFont() override
                {
                    return customFont;
                }

                void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box) override
                {
                    g.setFont (customFont);

                    // Draw rounded background with 5px corner radius
                    g.setColour (bgColour);
                    juce::Rectangle<float> backgroundRect (0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height));
                    g.fillRoundedRectangle (backgroundRect, 5.0f);

                    // Draw the dropdown arrow as white
                    g.setColour (juce::Colours::white);
                    juce::Path path;
                    path.startNewSubPath (0.0f, 0.0f);
                    path.lineTo (8.0f, 0.0f);
                    path.lineTo (4.0f, 8.0f);
                    path.closeSubPath();
                    g.fillPath (path, juce::AffineTransform::translation (width - 20.0f, (height - 8.0f) * 0.5f));
                }

                void drawComboBoxTextWhenNothingSelected (juce::Graphics& g, ComboBox& box, juce::Label& label) override
                {
                    g.setColour (juce::Colours::white.withAlpha (0.7f));
                    g.setFont (customFont);
                    
                    auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

                    g.drawFittedText(
                        box.getTextWhenNothingSelected(),
                        textArea,
                        juce::Justification::centred, // center horizontally and vertically
                        juce::jmax(1, (int)( (float)textArea.getHeight() / customFont.getHeight() )),
                        label.getMinimumHorizontalScale()
                    );
                }

                void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
                {
                    g.setColour (bgColour);
                    g.fillRect (0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height));
                }

                void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* const textColourToUse) override
                {
                    if (isSeparator)
                    {
                        juce::Rectangle<float> r (area.toFloat().reduced (0, 7));
                        r.removeFromTop (r.getHeight() * 0.5f - 0.5f);

                        g.setColour (bgColour);
                        g.fillRect (r.removeFromTop (1));

                        juce::ColourGradient cg (bgColour, 0.0f, 0.0f, bgColour.brighter (0.1f), 0.0f, (float) area.getHeight(), false);

                        g.setGradientFill (cg);
                        g.fillRect (r.removeFromTop (1));
                    }
                    else
                    {
                        juce::Rectangle<int> r (area);

                        if (isHighlighted)
                        {
                            g.setColour (bgColour.brighter (0.1f));
                            g.fillRect (r.toFloat());

                            g.setColour (bgColour.brighter (0.2f));
                            g.drawRect (r.toFloat(), 8.0f);
                        }

                        g.setColour (bgColour);
                        g.setFont (customFont);

                        juce::Rectangle<float> iconArea ((float) r.removeFromLeft ((r.getHeight() * 5) / 4).reduced (3).getX(),
                            (float) r.getY(),
                            (float) r.getHeight(),
                            (float) r.getHeight());

                        if (icon != nullptr)
                        {
                            icon->drawWithin (g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
                        }
                        else if (isTicked)
                        {
                            const juce::Path tick (getTickShape (1.0f));
                            g.setColour (juce::Colours::white);
                            g.fillPath (tick, tick.getTransformToScaleToFit (iconArea.reduced (iconArea.getWidth() / 5, 0).toFloat(), true));
                        }

                        if (hasSubMenu)
                        {
                            const auto arrowH = 0.6f * customFont.getHeight();

                            const auto x = (float) r.removeFromRight ((int) arrowH).getX();
                            const auto halfH = (float) r.getCentreY();

                            juce::Path path;
                            path.startNewSubPath (x, halfH - arrowH * 0.5f);
                            path.lineTo (x + arrowH * 0.6f, halfH);
                            path.lineTo (x, halfH + arrowH * 0.5f);

                            g.strokePath (path, juce::PathStrokeType (2.0f));
                        }
                        g.setColour (isHighlighted ? juce::Colours::white : juce::Colours::grey);
                        g.setFont (customFont);

                        g.setColour (juce::Colours::white);
                        g.drawText (text, r, juce::Justification::centredLeft, true);
                    }
                }

            private:
                Font getCommonMenuFont()
                {
                    return customFont;
                }

                juce::Colour bgColour;
                juce::Colour bgStroke;
                juce::Font customFont;

                JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetCBLookAndFeel)
            } mainCBLookAndFeel;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetCB)
        };
    }
}