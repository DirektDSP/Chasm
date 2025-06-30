#pragma once

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
                    bgColour = juce::Colour::fromRGB (173, 173, 173);
                }

                void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box) override
                {
                    // Define corner radius

                    // draw rounded background colour
                    g.setColour (bgColour);
                    juce::Rectangle<float> backgroundRect (0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height));
                    g.fillAll();

                    // draw the icon as white
                    g.setColour (juce::Colours::white);
                    juce::Path path;
                    path.startNewSubPath (0.0f, 0.0f);
                    path.lineTo (8.0f, 0.0f);
                    path.lineTo (4.0f, 8.0f);
                    path.closeSubPath();
                    g.fillPath (path, juce::AffineTransform::translation (width - 12.0f, (height - 8.0f) * 0.5f));
                }

                // do the same for the dropdown menu

                void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
                {
                    g.setColour (bgColour);
                    g.fillAll();
                }

                void // change the highlight colour
                    drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* const textColourToUse) override
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
                            g.fillRect (r);

                            g.setColour (bgColour.brighter (0.2f));
                            g.drawRect (r, 1);
                        }

                        g.setColour (bgColour);
                        g.setFont (juce::jmin (15.0f, area.getHeight() * 0.85f));

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
                            const auto arrowH = 0.6f * getPopupMenuFont().getHeight();

                            const auto x = (float) r.removeFromRight ((int) arrowH).getX();
                            const auto halfH = (float) r.getCentreY();

                            juce::Path path;
                            path.startNewSubPath (x, halfH - arrowH * 0.5f);
                            path.lineTo (x + arrowH * 0.6f, halfH);
                            path.lineTo (x, halfH + arrowH * 0.5f);

                            g.strokePath (path, juce::PathStrokeType (2.0f));
                        }
                        g.setColour (isHighlighted ? juce::Colours::white : juce::Colours::grey);
                        g.setFont (getPopupMenuFont());

                        g.setColour (juce::Colours::white);
                        g.drawText (text, r, juce::Justification::centredLeft, true);
                    }
                }

            private:
                juce::Colour bgColour;

                JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetCBLookAndFeel)
            } mainCBLookAndFeel;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetCB)
        };
    }
}