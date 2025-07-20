#include "PresetManager.h"
#include <algorithm>

namespace Service
{
    const File PresetManager::defaultDirectory {
        File::getSpecialLocation (File::SpecialLocationType::commonDocumentsDirectory)
            .getChildFile ("DirektDSP")
            .getChildFile (JucePlugin_Name)
            .getChildFile ("Presets")
    };

    const String PresetManager::extension { "ddsp" };
    const String PresetManager::presetNameProperty { "presetName" };
    const String PresetManager::defaultCategory { "Default" };

    PresetManager::PresetManager (AudioProcessorValueTreeState& apvts)
        : valueTreeState (apvts)
    {
        if (!defaultDirectory.exists())
        {
            const auto result = defaultDirectory.createDirectory();
            if (result.failed())
            {
                DBG ("Could not create preset directory: " + result.getErrorMessage());
                jassertfalse;
            }
        }

        valueTreeState.state.addListener (this);
        currentPreset.referTo (valueTreeState.state.getPropertyAsValue (presetNameProperty, nullptr));
        currentCategory.referTo (valueTreeState.state.getPropertyAsValue ("currentCategory", nullptr));
        
        // Add parameter listeners to clear preset name when parameters are modified
        addParameterListeners();
        
        updatePresetList();
    }

    void PresetManager::buildPresetMenu (PopupMenu& menu, int& menuItemId)
    {
        // Clear previous mappings
        menuItemToPresetMap.clear();
        menuItemToCategoryMap.clear();

        const auto categories = getAllCategories();

        for (const auto& category : categories)
        {
            const auto presetsInCategory = getPresetsInCategory (category);

            if (presetsInCategory.isEmpty())
                continue;

            if (category == defaultCategory)
            {
                // Add default category presets directly to main menu
                menu.addSectionHeader ("Default Presets");

                for (const auto& preset : presetsInCategory)
                {
                    menuItemToPresetMap.add (preset);
                    menuItemToCategoryMap.add (category);

                    // Mark current preset with a tick
                    const bool isCurrentPreset = (preset == getCurrentPreset() && category == getCurrentCategory());
                    menu.addItem (menuItemId++, preset, true, isCurrentPreset);
                }

                menu.addSeparator();
            }
            else
            {
                // Create submenu for this category
                PopupMenu categorySubmenu;
                buildCategorySubmenu (categorySubmenu, category, menuItemId);

                // Add submenu to main menu with folder icon or indicator
                menu.addSubMenu (category + " ▶", categorySubmenu);
            }
        }
    }

    void PresetManager::buildCategorySubmenu (PopupMenu& submenu, const String& category, int& menuItemId)
    {
        const auto presetsInCategory = getPresetsInCategory (category);

        // Add header showing category name
        submenu.addSectionHeader (category);

        for (const auto& preset : presetsInCategory)
        {
            menuItemToPresetMap.add (preset);
            menuItemToCategoryMap.add (category);

            // Mark current preset with a tick
            const bool isCurrentPreset = (preset == getCurrentPreset() && category == getCurrentCategory());
            submenu.addItem (menuItemId++, preset, true, isCurrentPreset);
        }

        // Add separator and category management options
        submenu.addSeparator();
        submenu.addItem (menuItemId++, "Delete Category: " + category, true, false);

        // Store the category management item mapping
        menuItemToPresetMap.add ("DELETE_CATEGORY");
        menuItemToCategoryMap.add (category);
    }

    void PresetManager::handlePresetMenuResult (int result, const StringArray& menuItemIds)
    {
        if (result == 0 || result > menuItemToPresetMap.size())
            return;

        const int index = result - 1; // Convert to 0-based index
        const String presetName = menuItemToPresetMap[index];
        const String category = menuItemToCategoryMap[index];

        if (presetName == "DELETE_CATEGORY")
        {
            MessageBoxOptions options = MessageBoxOptions()
                                            .withTitle ("Delete Category")
                                            .withMessage ("Are you sure you want to delete the category '" + category + "' and all its presets?")
                                            .withButton ("Delete")
                                            .withButton ("Cancel")
                                            .withParentComponent(nullptr); // Use the main component as parent

            NativeMessageBox::showAsync (options, [this, category] (int result) {
                DBG("Button Pressed");
                if (result == 1)
                    deleteCategory (category);
            });
        }
        else
        {
            // Load the selected preset
            loadPreset (presetName, category);
        }
    }

    void PresetManager::savePreset (const String& presetName, const String& artistName, const String& category)
    {
        if (presetName.isEmpty())
            return;

        const String finalCategory = category.isEmpty() ? defaultCategory : category;

        // Create category directory if it doesn't exist
        if (!finalCategory.isEmpty() && finalCategory != defaultCategory)
        {
            createCategory (finalCategory);
        }

        currentPreset.setValue (presetName);
        currentCategory.setValue (finalCategory);

        auto state = valueTreeState.copyState();
        state.setProperty ("artist", artistName, nullptr);
        state.setProperty ("category", finalCategory, nullptr);
        state.setProperty ("dateCreated", Time::getCurrentTime().toISO8601 (true), nullptr);
        state.setProperty ("dateModified", Time::getCurrentTime().toISO8601 (true), nullptr);

        const auto xml = state.createXml();
        const auto presetFile = getPresetFile (presetName, finalCategory);

        if (!xml->writeTo (presetFile))
        {
            DBG ("Could not create preset file: " + presetFile.getFullPathName());
            jassertfalse;
        }

        updatePresetList();
    }

    void PresetManager::deletePreset (const String& presetName, const String& category)
    {
        if (presetName.isEmpty())
            return;

        const String finalCategory = category.isEmpty() ? getCurrentCategory() : category;
        const auto presetFile = getPresetFile (presetName, finalCategory);

        DBG("[PRESET-MANAGER] Deleting preset: " << presetName << " from category: " << finalCategory);
        if (!presetFile.existsAsFile())
        {
            DBG ("Preset file " << presetFile.getFullPathName() << " does not exist");
            jassertfalse;
            return;
        }

        if (!presetFile.moveToTrash())
        {
            DBG ("Preset file " << presetFile.getFullPathName() << " could not be deleted");
            jassertfalse;
            return;
        }

        currentPreset.setValue ("");
        updatePresetList();
        DBG("[PRESET-MANAGER] Preset deleted: " << presetName << " from category: " << finalCategory);
    }

    void PresetManager::loadPreset (const String& presetName, const String& category)
    {
        if (presetName.isEmpty())
            return;

        DBG("[PRESET-MANAGER] Loading preset: " << presetName << " from category: " << category);

        const String finalCategory = category.isEmpty() ? getCurrentCategory() : category;
        const auto presetFile = getPresetFile (presetName, finalCategory);

        if (!presetFile.existsAsFile())
        {
            DBG ("Preset file " + presetFile.getFullPathName() + " does not exist");
            jassertfalse;
            return;
        }

        XmlDocument xmlDocument { presetFile };
        std::unique_ptr<XmlElement> xml (xmlDocument.getDocumentElement());
        if (xml == nullptr)
        {
            DBG ("Invalid XML in preset file");
            jassertfalse;
            return;
        }

        auto valueTreeToLoad = ValueTree::fromXml (*xml);
        valueTreeToLoad.setProperty ("dateModified", Time::getCurrentTime().toISO8601 (true), nullptr);

        // Set flag to prevent clearing preset name during load
        isLoadingPreset = true;
        valueTreeState.replaceState (valueTreeToLoad);
        currentPreset.setValue (presetName);
        currentCategory.setValue (finalCategory);
        isLoadingPreset = false;

        updatePresetList();
    }

    void PresetManager::createCategory (const String& categoryName)
    {
        if (categoryName.isEmpty() || categoryName == defaultCategory)
            return;

        const auto categoryDir = getCategoryDirectory (categoryName);
        if (!categoryDir.exists())
        {
            const auto result = categoryDir.createDirectory();
            if (result.failed())
            {
                DBG ("Could not create category directory: " + result.getErrorMessage());
                jassertfalse;
            }
        }
        updatePresetList();
    }

    void PresetManager::deleteCategory (const String& categoryName)
    {
        if (categoryName.isEmpty() || categoryName == defaultCategory)
            return;

        const auto categoryDir = getCategoryDirectory (categoryName);
        if (categoryDir.exists())
        {
            // Move all presets in category to trash
            const auto files = categoryDir.findChildFiles (File::findFiles, false, "*." + extension);
            for (const auto& file : files)
            {
                file.moveToTrash();
            }

            // Remove the directory
            if (!categoryDir.deleteRecursively()){
                DBG ("Could not delete category directory: " + categoryDir.getFullPathName());
                jassertfalse; // Assert to catch this during development
            }
        }
        updatePresetList();
    }

    bool PresetManager::categoryExists (const String& categoryName) const
    {
        if (categoryName.isEmpty() || categoryName == defaultCategory)
            return true;

        return getCategoryDirectory (categoryName).exists();
    }

    StringArray PresetManager::getAllCategories() const
    {
        StringArray categories;
        categories.add (defaultCategory);

        const auto subdirs = defaultDirectory.findChildFiles (File::findDirectories, false);
        StringArray otherCategories;
        for (const auto& dir : subdirs)
        {
            otherCategories.add (dir.getFileName());
        }

        // Sort categories with custom comparator
        otherCategories.sort (false); // First do a basic sort
        
        // Custom sort: numbers 0-9, 10-19, ..., 90-99, then A-Z
        std::sort (otherCategories.begin(), otherCategories.end(), 
            [] (const String& a, const String& b) -> bool
            {
                // Helper lambda to get sort priority
                auto getSortPriority = [] (const String& str) -> int
                {
                    if (str.isEmpty()) return 1000;
                    
                    const auto firstChar = str[0];
                    
                    // Check if it starts with a digit
                    if (firstChar >= '0' && firstChar <= '9')
                    {
                        // Extract the leading number
                        int number = 0;
                        int pos = 0;
                        while (pos < str.length() && str[pos] >= '0' && str[pos] <= '9')
                        {
                            number = number * 10 + (str[pos] - '0');
                            pos++;
                        }
                        
                        // Return priority based on tens digit: 0-9 = 0, 10-19 = 1, etc.
                        return number / 10;
                    }
                    else
                    {
                        // Letters come after all numbers (priority 100+)
                        return 100;
                    }
                };
                
                int priorityA = getSortPriority(a);
                int priorityB = getSortPriority(b);
                
                if (priorityA != priorityB)
                {
                    return priorityA < priorityB;
                }
                
                // Same priority group - use natural string comparison
                return a.compareNatural(b) < 0;
            });

        for (const auto& category : otherCategories)
        {
            categories.add (category);
        }

        return categories;
    }

    StringArray PresetManager::getAllPresets() const
    {
        StringArray presets;

        // Get presets from root directory (default category)
        const auto rootFiles = defaultDirectory.findChildFiles (File::findFiles, false, "*." + extension);
        for (const auto& file : rootFiles)
            presets.add (file.getFileNameWithoutExtension());

        // Get presets from subdirectories
        const auto categories = getAllCategories();
        for (const auto& category : categories)
        {
            if (category == defaultCategory)
                continue;

            const auto categoryPresets = getPresetsInCategory (category);
            for (const auto& preset : categoryPresets)
                presets.add (category + "/" + preset);
        }

        return presets;
    }

    StringArray PresetManager::getPresetsInCategory (const String& category) const
    {
        StringArray presets;

        File searchDir = (category.isEmpty() || category == defaultCategory) ? defaultDirectory : getCategoryDirectory (category);

        if (!searchDir.exists())
            return presets;

        const auto fileArray = searchDir.findChildFiles (File::findFiles, false, "*." + extension);
        for (const auto& file : fileArray)
            presets.add (file.getFileNameWithoutExtension());

        return presets;
    }

    Array<PresetMetadata> PresetManager::getAllPresetMetadata() const
    {
        Array<PresetMetadata> result;

        // Get metadata from root directory (default category)
        const auto rootFiles = defaultDirectory.findChildFiles (File::findFiles, false, "*." + extension);
        for (const auto& file : rootFiles)
        {
            XmlDocument xmlDocument (file);
            std::unique_ptr<XmlElement> xml (xmlDocument.getDocumentElement());

            if (xml != nullptr)
            {
                auto tree = ValueTree::fromXml (*xml);
                PresetMetadata meta;
                meta.name = file.getFileNameWithoutExtension();
                meta.artist = tree.getProperty ("artist", "Unknown").toString();
                meta.category = defaultCategory;
                meta.dateCreated = tree.getProperty ("dateCreated", "").toString();
                meta.dateModified = tree.getProperty ("dateModified", "").toString();
                result.add (meta);
            }
        }

        // Get metadata from subdirectories
        const auto categories = getAllCategories();
        for (const auto& category : categories)
        {
            if (category == defaultCategory)
                continue;

            const auto categoryMetadata = getPresetMetadataInCategory (category);
            result.addArray (categoryMetadata);
        }

        return result;
    }

    Array<PresetMetadata> PresetManager::getPresetMetadataInCategory (const String& category) const
    {
        Array<PresetMetadata> result;

        File searchDir = (category.isEmpty() || category == defaultCategory) ? defaultDirectory : getCategoryDirectory (category);

        if (!searchDir.exists())
            return result;

        const auto fileArray = searchDir.findChildFiles (File::findFiles, false, "*." + extension);
        for (const auto& file : fileArray)
        {
            XmlDocument xmlDocument (file);
            std::unique_ptr<XmlElement> xml (xmlDocument.getDocumentElement());

            if (xml != nullptr)
            {
                auto tree = ValueTree::fromXml (*xml);
                PresetMetadata meta;
                meta.name = file.getFileNameWithoutExtension();
                meta.artist = tree.getProperty ("artist", "Unknown").toString();
                meta.category = category;
                meta.dateCreated = tree.getProperty ("dateCreated", "").toString();
                meta.dateModified = tree.getProperty ("dateModified", "").toString();
                result.add (meta);
            }
        }

        return result;
    }

    int PresetManager::loadNextPreset()
    {
        return loadNextPresetInCategory (getCurrentCategory());
    }

    int PresetManager::loadPreviousPreset()
    {
        return loadPreviousPresetInCategory (getCurrentCategory());
    }

    int PresetManager::loadNextPresetInCategory (const String& category)
    {
        const auto categoryPresets = getPresetsInCategory (category);
        if (categoryPresets.isEmpty())
            return -1;

        const auto currentIndex = categoryPresets.indexOf (getCurrentPreset());
        const auto nextIndex = currentIndex + 1 > (categoryPresets.size() - 1) ? 0 : currentIndex + 1;
        loadPreset (categoryPresets[nextIndex], category);
        return nextIndex;
    }

    int PresetManager::loadPreviousPresetInCategory (const String& category)
    {
        const auto categoryPresets = getPresetsInCategory (category);
        if (categoryPresets.isEmpty())
            return -1;

        const auto currentIndex = categoryPresets.indexOf (getCurrentPreset());
        const auto previousIndex = currentIndex - 1 < 0 ? categoryPresets.size() - 1 : currentIndex - 1;
        loadPreset (categoryPresets[previousIndex], category);
        return previousIndex;
    }

    String PresetManager::getCurrentPreset() const
    {
        return currentPreset.toString();
    }

    String PresetManager::getCurrentCategory() const
    {
        const String category = currentCategory.toString();
        return category.isEmpty() ? defaultCategory : category;
    }

    void PresetManager::movePresetToCategory (const String& presetName, const String& fromCategory, const String& toCategory)
    {
        if (presetName.isEmpty() || fromCategory == toCategory)
            return;

        const auto fromFile = getPresetFile (presetName, fromCategory);
        if (!fromFile.existsAsFile())
        {
            DBG ("Source preset file does not exist: " + fromFile.getFullPathName());
            return;
        }

        // Create destination category if needed
        if (!toCategory.isEmpty() && toCategory != defaultCategory)
        {
            createCategory (toCategory);
        }

        const auto toFile = getPresetFile (presetName, toCategory);

        // Read the preset and update its category metadata
        XmlDocument xmlDocument { fromFile };
        std::unique_ptr<XmlElement> xml (xmlDocument.getDocumentElement());
        if (xml != nullptr)
        {
            auto tree = ValueTree::fromXml (*xml);
            tree.setProperty ("category", toCategory, nullptr);
            tree.setProperty ("dateModified", Time::getCurrentTime().toISO8601 (true), nullptr);

            const auto updatedXml = tree.createXml();
            if (updatedXml->writeTo (toFile))
            {
                fromFile.moveToTrash();
                updatePresetList();
            }
        }
    }

    File PresetManager::getCategoryDirectory (const String& category) const
    {
        if (category.isEmpty() || category == defaultCategory)
            return defaultDirectory;

        return defaultDirectory.getChildFile (category);
    }

    void PresetManager::valueTreeRedirected (ValueTree& treeWhichHasBeenChanged)
    {
        currentPreset.referTo (treeWhichHasBeenChanged.getPropertyAsValue (presetNameProperty, nullptr));
        currentCategory.referTo (treeWhichHasBeenChanged.getPropertyAsValue ("currentCategory", nullptr));
    }

    void PresetManager::updatePresetList()
    {
        availablePresets = getAllPresets();
        availableCategories = getAllCategories();
    }

    File PresetManager::getPresetFile (const String& presetName, const String& category) const
    {
        const String finalCategory = category.isEmpty() ? defaultCategory : category;
        const File dir = (finalCategory == defaultCategory) ? defaultDirectory : getCategoryDirectory (finalCategory);
        return dir.getChildFile (presetName + "." + extension);
    }

    void PresetManager::addParameterListeners()
    {
        // Get all parameters and add listeners to detect changes
        for (auto* param : valueTreeState.processor.getParameters())
        {
            if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
            {
                rangedParam->addListener(this);
            }
        }
    }

    void PresetManager::removeParameterListeners()
    {
        // Remove listeners from all parameters
        for (auto* param : valueTreeState.processor.getParameters())
        {
            if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
            {
                rangedParam->removeListener(this);
            }
        }
    }

    void PresetManager::parameterValueChanged(int parameterIndex, float newValue)
    {
        // Clear the current preset name when any parameter is changed
        // but only if there's currently a preset selected and we're not loading a preset
        if (!isLoadingPreset && getCurrentPreset().isNotEmpty())
        {
            DBG("[PRESET-MANAGER] Parameter " << parameterIndex << " changed to " << newValue << ", clearing preset name");
            currentPreset.setValue("");
        }
    }

    void PresetManager::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
    {
        // We don't need to do anything for gesture changes
    }

    PresetManager::~PresetManager()
    {
        removeParameterListeners();
        valueTreeState.state.removeListener(this);
    }
}