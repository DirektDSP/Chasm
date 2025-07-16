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
            PresetPanel (Service::PresetManager& pm) : presetManager (pm),
                                                       saveButton (PresetButton::IconType::Plus),
                                                       deleteButton (PresetButton::IconType::Delete),
                                                       presetMenuButton ("Select Preset")
            {
                configureButton (saveButton, "");
                configureButton (deleteButton, "");
                configureButton (presetMenuButton, "Select Preset");

                // Configure the preset menu button
                presetMenuButton.setButtonText("Select Preset");
                presetMenuButton.setMouseCursor (MouseCursor::PointingHandCursor);
                addAndMakeVisible (presetMenuButton);
                presetMenuButton.addListener (this);

                // Keep the ComboBox for backward compatibility if needed
                presetList.setTextWhenNothingSelected ("No Preset Selected");
                presetList.setMouseCursor (MouseCursor::PointingHandCursor);
                addAndMakeVisible (presetList);
                presetList.addListener (this);

                loadPresetList();
                updatePresetMenuButton();
            }

            ~PresetPanel() override
            {
                saveButton.removeListener (this);
                deleteButton.removeListener (this);
                presetList.removeListener (this);
                presetMenuButton.removeListener (this);
            }

            void resized() override
            {
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
                const int dropdownX = int (220.0f * scaleX);
                const int dropdownY = int (48.0f * scaleY);
                const int dropdownWidth = int (600.0f * scaleX);
                const int dropdownHeight = int (82.0f * scaleY);

                const int saveX = int (840.0f * scaleX);
                const int saveY = int (48.0f * scaleY);
                const int buttonWidth = int (82.0f * scaleX);
                const int buttonHeight = int (82.0f * scaleY);

                const int deleteX = int (940.0f * scaleX);
                const int deleteY = int (48.0f * scaleY);

                // Set component bounds - use presetMenuButton instead of presetList
                presetMenuButton.setBounds (dropdownX, dropdownY, dropdownWidth, dropdownHeight);
                presetList.setBounds (dropdownX, dropdownY, dropdownWidth, dropdownHeight);
                presetList.setVisible(false); // Hide ComboBox, use button instead
                
                saveButton.setBounds (saveX, saveY, buttonWidth, buttonHeight);
                deleteButton.setBounds (deleteX, deleteY, buttonWidth, buttonHeight);
            }

            void presetManagerChanged()
            {
                loadPresetList();
                updatePresetMenuButton();
            }

        private:
            void buttonClicked (Button* button) override
            {
                if (button == &presetMenuButton)
                {
                    showPresetMenu();
                }
                else if (button == &saveButton)
                {
                    // Create a popup menu for save options
                    PopupMenu saveMenu;
                    saveMenu.addItem (1, "Save to Current Category");
                    saveMenu.addItem (2, "Save to New Category");
                    saveMenu.addSeparator();

                    // Add existing categories
                    const auto categories = presetManager.getAllCategories();
                    for (int i = 0; i < categories.size(); ++i)
                    {
                        saveMenu.addItem (10 + i, "Save to " + categories[i]);
                    }

                    saveMenu.showMenuAsync (PopupMenu::Options().withTargetComponent (&saveButton),
                        [this, categories] (int result) {
                            if (result == 1)
                            {
                                // Save to current category
                                showSaveDialog (presetManager.getCurrentCategory());
                            }
                            else if (result == 2)
                            {
                                // Save to new category
                                showNewCategoryDialog();
                            }
                            else if (result >= 10)
                            {
                                // Save to specific category
                                const int categoryIndex = result - 10;
                                if (categoryIndex < categories.size())
                                {
                                    showSaveDialog (categories[categoryIndex]);
                                }
                            }
                        });
                }
                else if (button == &deleteButton)
                {
                    const String currentPreset = presetManager.getCurrentPreset();
                    const String currentCategory = presetManager.getCurrentCategory();

                    DBG("[PRESET-PANEL] 1 Attempting to delete preset: " << currentPreset << " from category: " << currentCategory);

                    if (currentPreset.isNotEmpty())
                    {
                        DBG("[PRESET-PANEL] 2 Attempting to delete preset: " << currentPreset << " from category: " << currentCategory);
                        // Show non-blocking confirmation dialog
                        auto* deleteDialog = new AlertWindow(
                            "Delete Preset",
                            "Are you sure you want to delete the preset \"" + currentPreset + "\"?",
                            AlertWindow::QuestionIcon);

                        deleteDialog->addButton("Delete", 1, KeyPress(KeyPress::returnKey));
                        deleteDialog->addButton("Don't Delete", 2, KeyPress(KeyPress::escapeKey));
                        deleteDialog->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

                        deleteDialog->enterModalState(true,
                            ModalCallbackFunction::create([this, deleteDialog, currentPreset, currentCategory](int result) {
                                if (result == 1) {
                                    DBG("[PRESET-PANEL] Deleting preset: " << currentPreset << " from category: " << currentCategory);
                                    presetManager.deletePreset(currentPreset, currentCategory);
                                    loadPresetList();
                                    updatePresetMenuButton();
                                } else {
                                    DBG("[PRESET-PANEL] Preset deletion cancelled. Result was " << result);
                                }
                                delete deleteDialog;
                            }),
                            false);
                    }
                    else
                    {
                        DBG("[PRESET-PANEL] No preset selected to delete.");
                    }
                }
            }

            void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
            {
                if (comboBoxThatHasChanged == &presetList)
                {
                    const auto selectedPresetInfo = presetList.getSelectedPresetInfo();
                    if (selectedPresetInfo.isValid)
                    {
                        presetManager.loadPreset (selectedPresetInfo.presetName, selectedPresetInfo.category);
                        updatePresetMenuButton();
                    }
                }
            }

            void showPresetMenu()
            {
                PopupMenu menu;
                int menuItemId = 1;
                
                // Clear previous mappings
                menuItemToPresetMap.clear();
                menuItemToCategoryMap.clear();
                
                buildPresetMenu(menu, menuItemId);
                
                // Add menu management options
                menu.addSeparator();
                menu.addItem(menuItemId++, "Create New Category...");
                
                // Show the menu
                menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&presetMenuButton),
                    [this, menuItemId] (int result) {
                        if (result > 0)
                        {
                            // Handle special menu items first
                            if (result == menuItemId - 1) // Create category
                            {
                                showNewCategoryDialog();
                            }
                            else
                            {
                                // Handle preset selection
                                handlePresetMenuResult(result);
                            }
                        }
                    });
            }

            void buildPresetMenu(PopupMenu& menu, int& menuItemId)
            {
                const auto categories = presetManager.getAllCategories();
                
                for (const auto& category : categories)
                {
                    const auto presetsInCategory = presetManager.getPresetsInCategory(category);
                    
                    if (presetsInCategory.isEmpty())
                        continue;
                        
                    if (category == Service::PresetManager::defaultCategory)
                    {
                        // Add default category presets directly to main menu
                        menu.addSectionHeader("Default Presets");
                        
                        for (const auto& preset : presetsInCategory)
                        {
                            menuItemToPresetMap.add(preset);
                            menuItemToCategoryMap.add(category);
                            
                            // Mark current preset with a tick
                            const bool isCurrentPreset = (preset == presetManager.getCurrentPreset() && 
                                                        category == presetManager.getCurrentCategory());
                            menu.addItem(menuItemId++, preset, true, isCurrentPreset);
                        }
                        
                        menu.addSeparator();
                    }
                    else
                    {
                        // Create submenu for this category
                        PopupMenu categorySubmenu;
                        buildCategorySubmenu(categorySubmenu, category, menuItemId);
                        
                        // Add submenu to main menu with folder icon or indicator
                        menu.addSubMenu(category + " ▶", categorySubmenu);
                    }
                }
            }

            void buildCategorySubmenu(PopupMenu& submenu, const String& category, int& menuItemId)
            {
                const auto presetsInCategory = presetManager.getPresetsInCategory(category);
                
                // Add header showing category name
                submenu.addSectionHeader(category);
                
                for (const auto& preset : presetsInCategory)
                {
                    menuItemToPresetMap.add(preset);
                    menuItemToCategoryMap.add(category);
                    
                    // Mark current preset with a tick
                    const bool isCurrentPreset = (preset == presetManager.getCurrentPreset() && 
                                                category == presetManager.getCurrentCategory());
                    submenu.addItem(menuItemId++, preset, true, isCurrentPreset);
                }
                
                // Add separator and category management options
                submenu.addSeparator();
                submenu.addItem(menuItemId++, "Delete Category: " + category, true, false);
                
                // Store the category management item mapping
                menuItemToPresetMap.add("DELETE_CATEGORY");
                menuItemToCategoryMap.add(category);
            }

            void handlePresetMenuResult(int result)
            {
                if (result == 0 || result > menuItemToPresetMap.size())
                    return;
                    
                const int index = result - 1; // Convert to 0-based index
                const String presetName = menuItemToPresetMap[index];
                const String category = menuItemToCategoryMap[index];
                
                if (presetName == "DELETE_CATEGORY")
                {
                    DBG("[PRESET-PANEL] 1 Attempting to delete category: " << category);
                    auto* deleteCatDialog = new AlertWindow(
                        "Delete Category",
                        "Are you sure you want to delete the category '" + category + "' and all its presets?",
                        AlertWindow::WarningIcon);

                    deleteCatDialog->addButton("Delete", 1, KeyPress(KeyPress::returnKey));
                    deleteCatDialog->addButton("Don't Delete", 2, KeyPress(KeyPress::escapeKey));
                    deleteCatDialog->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

                    deleteCatDialog->enterModalState(true,
                        ModalCallbackFunction::create([this, deleteCatDialog, category](int result) {
                            if (result == 1) {
                                DBG("[PRESET-PANEL] Deleting category: " << category);
                                presetManager.deleteCategory(category);
                                loadPresetList();
                                updatePresetMenuButton();
                            } else {
                                DBG("[PRESET-PANEL] Category deletion cancelled. Result was " << result);
                            }
                            delete deleteCatDialog;
                        }),
                        false);
                }
                else
                {
                    // Load the selected preset
                    presetManager.loadPreset(presetName, category);
                    updatePresetMenuButton();
                }
            }

            void updatePresetMenuButton()
            {
                const String currentPreset = presetManager.getCurrentPreset();
                const String currentCategory = presetManager.getCurrentCategory();
                
                if (currentPreset.isNotEmpty())
                {
                    // Show current preset and category
                    String buttonText = currentPreset;
                    if (currentCategory != Service::PresetManager::defaultCategory)
                    {
                        buttonText = currentCategory + " / " + currentPreset;
                    }
                    presetMenuButton.setButtonText(buttonText);
                }
                else
                {
                    presetMenuButton.setButtonText("Select Preset");
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
                presetList.populateFromPresetManager (presetManager);
            }

            void showSaveDialog (const String& category)
            {
                fileChooser = std::make_unique<FileChooser> (
                    "Please enter the name of the preset to save to category: " + category,
                    Service::PresetManager::defaultDirectory,
                    "*." + Service::PresetManager::extension);

                fileChooser->launchAsync (FileBrowserComponent::saveMode,
                    [this, category] (const FileChooser& chooser) {
                        const auto resultFile = chooser.getResult();
                        if (resultFile != File {})
                        {
                            String presetName = resultFile.getFileNameWithoutExtension();
                            if (presetName.isNotEmpty())
                            {
                                // Show dialog for artist name (optional)
                                showArtistNameDialog (presetName, category);
                            }
                        }
                    });
            }

            void showArtistNameDialog (const String& presetName, const String& category)
            {
                auto* artistDialog = new AlertWindow ("Save Preset",
                    "Enter artist name (optional):",
                    AlertWindow::NoIcon);

                artistDialog->addTextEditor ("artist", "", "Artist:");
                artistDialog->addButton ("Save", 1, KeyPress (KeyPress::returnKey));
                artistDialog->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));

                artistDialog->enterModalState (true,
                    ModalCallbackFunction::create ([this, artistDialog, presetName, category] (int result) {
                        if (result == 1) // Save button
                        {
                            String artistName = artistDialog->getTextEditorContents ("artist");
                            presetManager.savePreset (presetName, artistName, category);
                            loadPresetList();
                            updatePresetMenuButton();
                        }

                        // Don't forget to delete the dialog!
                        delete artistDialog;
                    }),
                    false);
            }

            void showNewCategoryDialog()
            {
                auto* categoryDialog = new AlertWindow ("New Category",
                    "Enter the name for the new category:",
                    AlertWindow::NoIcon);

                categoryDialog->addTextEditor ("category", "", "Category Name:");
                categoryDialog->addButton ("Create", 1, KeyPress (KeyPress::returnKey));
                categoryDialog->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));

                categoryDialog->enterModalState (true,
                    ModalCallbackFunction::create ([this, categoryDialog] (int result) {
                        if (result == 1) // Create button
                        {
                            String categoryName = categoryDialog->getTextEditorContents ("category");
                            if (categoryName.isNotEmpty())
                            {
                                presetManager.createCategory (categoryName);
                                loadPresetList();
                                updatePresetMenuButton();
                            }
                        }

                        delete categoryDialog;
                    }),
                    false);
            }

            Service::PresetManager& presetManager;
            PresetButton saveButton, deleteButton;
            PresetCB presetList;
            TextButton presetMenuButton; // New hierarchical menu button
            std::unique_ptr<FileChooser> fileChooser;
            
            // Menu item mappings for hierarchical menu
            StringArray menuItemToPresetMap;
            StringArray menuItemToCategoryMap;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetPanel)
        };
    }
}