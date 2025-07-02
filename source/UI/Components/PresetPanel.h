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
            PresetPanel (Service::PresetManager& pm) : 
                presetManager (pm),
                saveButton(PresetButton::IconType::Plus),
                deleteButton(PresetButton::IconType::Delete)
            {
                configureButton (saveButton, "");
                configureButton (deleteButton, "");

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
                presetList.removeListener (this);
            }

            void resized() override
            {
                // Simple approach: position elements directly using your specified coordinates
                // scaled to the current component size
                
                // Base resolution for calculations (2000 x 1200)
                const float baseWidth = 2000.0f;
                const float baseHeight = 1200.0f;

                auto* topLevel = getParentComponent();
                
                // Use the component's own dimensions for scaling
                const float currentWidth = topLevel->getWidth();
                const float currentHeight = topLevel->getHeight();
                
                // Calculate scale factors
                const float scaleX = currentWidth / baseWidth;
                const float scaleY = currentHeight / baseHeight;
                
                // Your specified positions, scaled to current size
                const int dropdownX = int(220.0f * scaleX);
                const int dropdownY = int(48.0f * scaleY);
                const int dropdownWidth = int(600.0f * scaleX);
                const int dropdownHeight = int(82.0f * scaleY);
                
                const int saveX = int(840.0f * scaleX);
                const int saveY = int(48.0f * scaleY);
                const int buttonWidth = int(82.0f * scaleX);
                const int buttonHeight = int(82.0f * scaleY);
                
                const int deleteX = int(940.0f * scaleX);
                const int deleteY = int(48.0f * scaleY);
                
                // Set component bounds
                presetList.setBounds(dropdownX, dropdownY, dropdownWidth, dropdownHeight);
                saveButton.setBounds(saveX, saveY, buttonWidth, buttonHeight);
                deleteButton.setBounds(deleteX, deleteY, buttonWidth, buttonHeight);
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
            PresetButton saveButton, deleteButton;
            PresetCB presetList;
            std::unique_ptr<FileChooser> fileChooser;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetPanel)
        };
    }
}