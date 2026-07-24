#pragma once

#include "Service/PresetManager.h"
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

            void populateFromPresetManager (const Service::PresetManager& presetManager)
            {
                clear();
                categoryStartIndices.clear();
                presetItemData.clear();

                int itemId = 1;

                const auto categories = presetManager.getAllCategories();
                for (const auto& category : categories)
                {
                    const auto presetsInCategory = presetManager.getPresetsInCategory (category);
                    if (presetsInCategory.isEmpty())
                        continue;

                    // Add category header (always show categories for better organization)
                    if (categories.size() > 1 || category != Service::PresetManager::defaultCategory)
                    {
                        addSeparator();
                        categoryStartIndices.add (itemId);
                        addItem (getCategoryDisplayName (category), itemId);

                        PresetItemData categoryData;
                        categoryData.category = category;
                        categoryData.isCategory = true;
                        presetItemData[itemId] = categoryData;
                        itemId++;

                        addSeparator();
                    }

                    // Add presets in this category
                    for (const auto& preset : presetsInCategory)
                    {
                        String displayName = preset;
                        if (categories.size() > 1 || category != Service::PresetManager::defaultCategory)
                        {
                            displayName = "  " + preset; // Indent preset names under categories
                        }

                        addItem (displayName, itemId);

                        PresetItemData data;
                        data.presetName = preset;
                        data.category = category;
                        data.isCategory = false;
                        data.isSubfolder = false;
                        presetItemData[itemId] = data;

                        itemId++;
                    }
                }

                // Set current selection based on current preset
                updateCurrentSelection (presetManager.getCurrentPreset(), presetManager.getCurrentCategory());
            }

            void updateCurrentSelection (const String& currentPreset, const String& currentCategory)
            {
                if (currentPreset.isEmpty())
                {
                    setSelectedId (0);
                    return;
                }

                for (auto& pair : presetItemData)
                {
                    const auto& data = pair.second;
                    if (!data.isCategory && !data.isSubfolder && data.presetName == currentPreset && data.category == currentCategory)
                    {
                        setSelectedId (pair.first);
                        return;
                    }
                }

                setSelectedId (0);
            }

            // Get preset info from selected item
            struct SelectedPresetInfo
            {
                String presetName;
                String category;
                bool isValid = false;
                bool isCategory = false;
            };

            SelectedPresetInfo getSelectedPresetInfo() const
            {
                SelectedPresetInfo info;
                const int selectedId = getSelectedId();

                if (presetItemData.count (selectedId) > 0)
                {
                    const auto& data = presetItemData.at (selectedId);
                    info.presetName = data.presetName;
                    info.category = data.category;
                    info.isCategory = data.isCategory;
                    info.isValid = !data.isCategory; // Only valid if it's not a category header
                }

                return info;
            }

            // Override to prevent selection of category headers
            void setSelectedId (int newItemId, NotificationType notification = sendNotificationAsync)
            {
                if (presetItemData.count (newItemId) > 0)
                {
                    const auto& data = presetItemData.at (newItemId);
                    if (data.isCategory)
                    {
                        // Don't allow selection of category headers
                        return;
                    }
                }

                ComboBox::setSelectedId (newItemId, notification);
            }

        private:
            struct PresetItemData
            {
                String presetName;
                String category;
                bool isCategory = false;
                bool isSubfolder = false;
            };

            std::map<int, PresetItemData> presetItemData;
            juce::Array<int> categoryStartIndices;

            String getCategoryDisplayName (const String& category) const
            {
                if (category == Service::PresetManager::defaultCategory)
                    return "Default Presets";

                return category;
            }

            class PresetCBLookAndFeel : public juce::LookAndFeel_V4
            {
            public:
                PresetCBLookAndFeel()
                {
                    bgColour = juce::Colour::fromRGB (160, 196, 126);
                    bgStroke = juce::Colour::fromRGB (135, 166, 106);
                    categoryColour = bgColour.brighter (0.3f);
                    customFont = UI::Utils::getCustomFont();
                }

                Font getComboBoxFont (ComboBox& /*box*/) override
                {
                    return customFont;
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

                    auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

                    g.drawFittedText (
                        box.getTextWhenNothingSelected(),
                        textArea,
                        juce::Justification::centred,
                        juce::jmax (1, (int) ((float) textArea.getHeight() / customFont.getHeight())),
                        label.getMinimumHorizontalScale());
                }

                void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
                {
                    g.setColour (bgColour);
                    g.fillRect (0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height));
                }

                void drawPopupMenuItem (
                    juce::Graphics& g,
                    const juce::Rectangle<int>& area,
                    bool isSeparator,
                    bool isActive,
                    bool isHighlighted,
                    bool isTicked,
                    bool hasSubMenu,
                    const juce::String& text,
                    const juce::String& shortcutKeyText,
                    const juce::Drawable* icon,
                    const juce::Colour* const textColourToUse) override
                {
                    if (isSeparator)
                    {
                        juce::Rectangle<float> r (area.toFloat().reduced (5, 2));
                        r.removeFromTop (r.getHeight() * 0.5f - 0.5f);

                        g.setColour (bgColour.darker (0.2f));
                        g.fillRect (r.removeFromTop (1));
                    }
                    else
                    {
                        juce::Rectangle<int> r (area);

                        // Check if this is a category header
                        bool isCategory = text.startsWith ("📁");
                        bool isIndented = text.startsWith ("  ");

                        if (isCategory)
                        {
                            // Draw category header background
                            g.setColour (categoryColour);
                            g.fillRect (r.toFloat());

                            g.setColour (bgColour.darker (0.3f));
                            g.drawRect (r.toFloat(), 1.0f);

                            // Category text styling
                            g.setColour (juce::Colours::white);
                            g.setFont (customFont.boldened());
                            g.drawText (text, r.reduced (8, 0), juce::Justification::centredLeft, true);
                        }
                        else
                        {
                            // Regular preset item
                            if (isHighlighted && isActive)
                            {
                                g.setColour (bgColour.brighter (0.1f));
                                g.fillRect (r.toFloat());

                                g.setColour (bgColour.brighter (0.2f));
                                g.drawRect (r.toFloat(), 1.0f);
                            }

                            // Draw preset text
                            g.setColour (isHighlighted ? juce::Colours::white : juce::Colours::white.withAlpha (0.9f));
                            g.setFont (customFont);
                            g.drawText (text, r.reduced (8, 0), juce::Justification::centredLeft, true);

                            // Draw tick for selected item
                            if (isTicked)
                            {
                                const juce::Path tick (getTickShape (1.0f));
                                g.setColour (juce::Colours::white);
                                juce::Rectangle<float> iconArea (r.removeFromRight (r.getHeight()).reduced (4).toFloat());
                                g.fillPath (tick, tick.getTransformToScaleToFit (iconArea, true));
                            }
                        }
                    }
                }

                void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int standardMenuItemHeight, int& idealWidth, int& idealHeight) override
                {
                    if (isSeparator)
                    {
                        idealWidth = 50;
                        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight / 10 : 10;
                    }
                    else
                    {
                        auto font = getPopupMenuFont();

                        if (text.startsWith ("📁"))
                            font = font.boldened();

                        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : roundToInt (font.getHeight() * 1.3f);
                        idealWidth = font.getStringWidth (text) + idealHeight * 2;
                    }
                }

                // Override to make category headers non-selectable
                void drawPopupMenuItemWithOptions (
                    juce::Graphics& g,
                    const juce::Rectangle<int>& area,
                    bool isHighlighted,
                    const PopupMenu::Item& item,
                    const PopupMenu::Options& options) override
                {
                    bool isCategory = item.text.startsWith ("📁");

                    if (isCategory)
                    {
                        // Don't highlight category headers
                        drawPopupMenuItem (g,
                            area,
                            false,
                            false,
                            false,
                            false,
                            false,
                            item.text,
                            item.shortcutKeyDescription,
                            item.image.get(),
                            nullptr);
                    }
                    else
                    {
                        drawPopupMenuItem (
                            g,
                            area,
                            item.isSeparator,
                            item.isEnabled,
                            isHighlighted,
                            item.isTicked,
                            (item.subMenu != nullptr),
                            item.text,
                            item.shortcutKeyDescription,
                            item.image.get(),
                            nullptr);
                    }
                }

            private:
                juce::Colour bgColour;
                juce::Colour bgStroke;
                juce::Colour categoryColour;
                juce::Font customFont;

                JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetCBLookAndFeel)
            } mainCBLookAndFeel;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetCB)
        };
    }
}