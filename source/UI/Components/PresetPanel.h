#pragma once

#include "Service/PresetManager.h"
#include "UI/Components/PresetButton.h"
#include "UI/Components/PresetCB.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

using namespace juce;

namespace UI
{
    namespace Components
    {
        class PresetPanel : public Component, Button::Listener, ComboBox::Listener
        {
        public:
            PresetPanel (Service::PresetManager& pm) : presetManager (pm)
            {
                configureButton (saveButton, "Save");
                configureButton (deleteButton, "Delete");
                configureButton (previousPresetButton, "< ");
                configureButton (nextPresetButton, " >");

                presetList.setTextWhenNothingSelected ("No Preset Selected");
                presetList.setMouseCursor (MouseCursor::PointingHandCursor);
                addAndMakeVisible (presetList);
                presetList.addListener (this);

                loadPresetList();
            }

            ~PresetPanel() override
            {
                saveButton.removeListener (this);
                deleteButton.removeListener (this);
                previousPresetButton.removeListener (this);
                nextPresetButton.removeListener (this);
                presetList.removeListener (this);
            }

            void resized() override
            {
                auto area = getLocalBounds(); // Add padding
                const int spacing = 0;
                const int buttonWidth = 80;
                const int dropdownWidth = 200;
                const int height = 24;

                // Define widths
                presetList.setBounds(area.removeFromLeft(dropdownWidth));

                area.removeFromLeft(spacing); // Spacer

                previousPresetButton.setBounds(area.removeFromLeft(buttonWidth));
                area.removeFromLeft(spacing);

                nextPresetButton.setBounds(area.removeFromLeft(buttonWidth));
                area.removeFromLeft(spacing);

                saveButton.setBounds(area.removeFromLeft(buttonWidth));
                area.removeFromLeft(spacing);

                deleteButton.setBounds(area.removeFromLeft(buttonWidth));
            }


        private:
            void buttonClicked (Button* button) override
            {
                if (button == &saveButton)
                {
                    fileChooser = std::make_unique<FileChooser> (
                        "Please enter the name of the preset to save",
                        Service::PresetManager::defaultDirectory,
                        "*." + Service::PresetManager::extension);
                    fileChooser->launchAsync (FileBrowserComponent::saveMode, [&] (const FileChooser& chooser) {
                        const auto resultFile = chooser.getResult();
                        presetManager.savePreset (resultFile.getFileNameWithoutExtension());
                        loadPresetList();
                    });
                }
                if (button == &previousPresetButton)
                {
                    const auto index = presetManager.loadPreviousPreset();
                    presetList.setSelectedItemIndex (index, dontSendNotification);
                }
                if (button == &nextPresetButton)
                {
                    const auto index = presetManager.loadNextPreset();
                    presetList.setSelectedItemIndex (index, dontSendNotification);
                }
                if (button == &deleteButton)
                {
                    presetManager.deletePreset (presetManager.getCurrentPreset());
                    loadPresetList();
                }
            }
            void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
            {
                if (comboBoxThatHasChanged == &presetList)
                {
                    presetManager.loadPreset (presetList.getItemText (presetList.getSelectedItemIndex()));
                }
            }

            void configureButton (Button& button, const String& buttonText)
            {
                button.setButtonText (buttonText);
                button.setMouseCursor (MouseCursor::PointingHandCursor);
                addAndMakeVisible (button);
                button.addListener (this);
            }

            void loadPresetList()
            {
                presetList.clear (dontSendNotification);
                const auto allPresets = presetManager.getAllPresets();
                const auto currentPreset = presetManager.getCurrentPreset();
                presetList.addItemList (allPresets, 1);
                presetList.setSelectedItemIndex (allPresets.indexOf (currentPreset), dontSendNotification);
            }

            Service::PresetManager& presetManager;
            PresetButton saveButton, deleteButton, previousPresetButton, nextPresetButton;
            PresetCB presetList;
            std::unique_ptr<FileChooser> fileChooser;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetPanel)
        };
    }
}