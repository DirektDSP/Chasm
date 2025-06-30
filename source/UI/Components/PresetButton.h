// PresetButton.h
#pragma once
#include "UI/Utils/CustomFont.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace UI
{
    namespace Components
    {
        class PresetButton : public juce::TextButton
        {
        public:
            enum class IconType {
                None,
                FloppyDisk,
                Delete,
                Plus
            };

            PresetButton (IconType iconType = IconType::None) : juce::TextButton (""), iconType (iconType)
            {
                mainButtonLookAndFeel.setIconType (iconType);
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
                    bgColour = juce::Colour::fromRGB (160, 196, 126);
                    customFont = UI::Utils::getCustomFont();
                    currentIconType = IconType::None;
                }

                void setIconType (IconType type)
                {
                    currentIconType = type;
                }

                Font getTextButtonFont (TextButton&, int buttonHeight) override
                {
                    return customFont;
                }

                void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown) override
                {
                    auto bounds = button.getLocalBounds().toFloat();
                    auto cornerRadius = 5.0f;

                    // Hover and pressed states
                    if (isButtonDown)
                    {
                        g.setColour (bgColour.darker (0.1f));
                    }
                    else if (isMouseOverButton)
                    {
                        g.setColour (bgColour.brighter (0.1f));
                    }
                    else
                    {
                        g.setColour (bgColour);
                    }

                    g.fillRoundedRectangle (bounds, cornerRadius);
                }

                void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonDown) override
                {
                    // Draw icon instead of text if we have an icon type
                    if (currentIconType != IconType::None)
                    {
                        drawIcon (g, button.getLocalBounds(), currentIconType);
                        return;
                    }

                    // Original text drawing code for buttons without icons
                    g.setFont (customFont);
                    g.setColour (juce::Colours::white);

                    const int yIndent = juce::jmin (4, button.proportionOfHeight (0.3f));
                    const int cornerSize = juce::jmin (button.getHeight(), button.getWidth()) / 2;

                    const int fontHeight = juce::roundToInt (customFont.getHeight() * 0.6f);
                    const int leftIndent = juce::jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
                    const int rightIndent = juce::jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
                    const int textWidth = button.getWidth() - leftIndent - rightIndent;

                    if (textWidth > 0)
                        g.drawFittedText (button.getButtonText(),
                            leftIndent,
                            yIndent,
                            textWidth,
                            button.getHeight() - yIndent * 2,
                            juce::Justification::centred,
                            2);
                }

            private:
                void drawIcon (juce::Graphics& g, juce::Rectangle<int> bounds, IconType iconType)
                {
                    auto iconBounds = bounds.reduced (bounds.getWidth() / 4);

                    switch (iconType)
                    {
                        case IconType::FloppyDisk:
                            drawFloppyIcon (g, iconBounds);
                            break;
                        case IconType::Delete:
                            drawDeleteIcon (g, iconBounds);
                            break;
                        case IconType::Plus:
                            drawPlusIcon (g, iconBounds);
                            break;
                        case IconType::None:
                        default:
                            break;
                    }
                }

                void drawFloppyIcon (juce::Graphics& g, juce::Rectangle<int> bounds)
                {
                    // Main floppy body
                    g.setColour (juce::Colours::transparentBlack.withAlpha (0.9f));
                    g.fillRoundedRectangle (bounds.toFloat(), 2.0f);

                    // Draw border
                    g.setColour (juce::Colours::grey);
                    g.drawRoundedRectangle (bounds.toFloat(), 2.0f, 1.5f);

                    // Metal sliding cover
                    auto coverBounds = bounds.removeFromTop (bounds.getHeight() / 3);
                    g.setColour (juce::Colours::grey.withAlpha (0.7f));
                    g.fillRoundedRectangle (coverBounds.reduced (3).toFloat(), 1.0f);

                    // Label area
                    auto labelBounds = bounds.removeFromBottom (bounds.getHeight() / 2);
                    g.setColour (juce::Colours::white.withAlpha (0.8f));
                    g.fillRoundedRectangle (labelBounds.reduced (6).toFloat(), 1.0f);

                    // Write protect notch
                    g.setColour (juce::Colours::white);
                    auto originalBounds = bounds.getUnion (coverBounds).getUnion (labelBounds);
                    auto notchBounds = juce::Rectangle<int> (originalBounds.getRight() - originalBounds.getWidth() / 8,
                        originalBounds.getY() + originalBounds.getHeight() / 6,
                        originalBounds.getWidth() / 12,
                        originalBounds.getHeight() / 6);
                    g.fillRect (notchBounds);
                }

                void drawDeleteIcon(juce::Graphics& g, juce::Rectangle<int> bounds)
                {
                    g.setColour(juce::Colours::white.withAlpha(0.9f));

                    int midX = bounds.getX() + bounds.getWidth() / 2;
                    int midY = bounds.getY() + bounds.getHeight() / 2;
                    int thickness = 3;
                    int length = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2;

                    juce::Graphics::ScopedSaveState saveState(g);

                    // Rotate 45 degrees (pi/4 radians) around center point
                    g.addTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::pi / 4, (float)midX, (float)midY));

                    // Draw vertical line (which will appear rotated)
                    g.fillRect(midX - thickness / 2, midY - length, thickness, length * 2);

                    // Draw horizontal line (which will appear rotated)
                    g.fillRect(midX - length, midY - thickness / 2, length * 2, thickness);
                }


                void drawPlusIcon (juce::Graphics& g, juce::Rectangle<int> bounds)
                {
                    g.setColour (juce::Colours::white.withAlpha (0.9f));

                    int midX = bounds.getX() + bounds.getWidth() / 2;
                    int midY = bounds.getY() + bounds.getHeight() / 2;
                    int thickness = 3; // thickness of the lines
                    int length = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2; // half length of each line

                    // vertical line
                    g.fillRect (midX - thickness / 2, midY - length, thickness, length * 2);

                    // horizontal line
                    g.fillRect (midX - length, midY - thickness / 2, length * 2, thickness);
                }

                juce::Colour bgColour;
                juce::Font customFont;
                IconType currentIconType;
                JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetButtonLookAndFeel)
            } mainButtonLookAndFeel;

            IconType iconType;
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetButton)
        };
    }
}