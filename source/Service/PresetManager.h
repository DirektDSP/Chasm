#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
using namespace juce;

namespace Service
{
    struct PresetMetadata
    {
        String name;
        String artist;
        String category;
        String dateCreated;
        String dateModified;
        String getFullPath() const { return category.isEmpty() ? name : category + "/" + name; }
    };

    class PresetManager : private ValueTree::Listener, private AudioProcessorParameter::Listener
    {
    public:
        static const File defaultDirectory;
        static const String extension;
        static const String presetNameProperty;
        static const String defaultCategory;

        PresetManager(AudioProcessorValueTreeState&);
        ~PresetManager();
       
        void buildPresetMenu(PopupMenu& menu, int& menuItemId);
        void handlePresetMenuResult(int result, const StringArray& menuItemIds);

        // Enhanced save/load methods with category support
        void savePreset(const String& presetName, const String& artistName = "Unknown", const String& category = "");
        void deletePreset(const String& presetName, const String& category = "");
        void loadPreset(const String& presetName, const String& category = "");
        
        // Category management
        void createCategory(const String& categoryName);
        void deleteCategory(const String& categoryName);
        bool categoryExists(const String& categoryName) const;
        StringArray getAllCategories() const;
        
        // Enhanced preset retrieval
        StringArray getAllPresets() const;
        StringArray getPresetsInCategory(const String& category) const;
        Array<PresetMetadata> getAllPresetMetadata() const;
        Array<PresetMetadata> getPresetMetadataInCategory(const String& category) const;
        
        // Navigation methods
        int loadNextPreset();
        int loadPreviousPreset();
        int loadNextPresetInCategory(const String& category);
        int loadPreviousPresetInCategory(const String& category);
        
        // Current preset info
        String getCurrentPreset() const;
        String getCurrentCategory() const;
        
        // Utility methods
        void movePresetToCategory(const String& presetName, const String& fromCategory, const String& toCategory);
        File getCategoryDirectory(const String& category) const;
        
    private:
        void buildCategorySubmenu(PopupMenu& submenu, const String& category, int& menuItemId);
        StringArray menuItemToPresetMap;
        StringArray menuItemToCategoryMap;

        void valueTreeRedirected(ValueTree& treeWhichHasBeenChanged) override;
        void updatePresetList();
        File getPresetFile(const String& presetName, const String& category) const;
        
        // AudioProcessorParameter::Listener overrides
        void parameterValueChanged(int parameterIndex, float newValue) override;
        void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;
        
        // Helper methods for parameter listening
        void addParameterListeners();
        void removeParameterListeners();
        
        // Check if current state matches loaded preset
        bool currentStateMatchesPreset() const;
        
        AudioProcessorValueTreeState& valueTreeState;
        Value currentPreset;
        Value currentCategory;
        StringArray availablePresets;
        StringArray availableCategories;
        
        // Flag to prevent clearing preset name during preset loading
        bool isLoadingPreset = false;
    };
}