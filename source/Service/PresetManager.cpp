#include "PresetManager.h"

namespace Service
{
    const File PresetManager::defaultDirectory{
        File::getSpecialLocation(File::SpecialLocationType::commonDocumentsDirectory)
            .getChildFile("DirektDSP")
            .getChildFile(JucePlugin_Name)
            .getChildFile("Presets")
    };

    const String PresetManager::extension{ "ddsp" };
    const String PresetManager::presetNameProperty{ "presetName" };

    PresetManager::PresetManager(AudioProcessorValueTreeState& apvts)
        : valueTreeState(apvts)
    {
        if (!defaultDirectory.exists())
        {
            const auto result = defaultDirectory.createDirectory();
            if (result.failed())
            {
                DBG("Could not create preset directory: " + result.getErrorMessage());
                jassertfalse;
            }
        }

        valueTreeState.state.addListener(this);
        currentPreset.referTo(valueTreeState.state.getPropertyAsValue(presetNameProperty, nullptr));
        updatePresetList();
    }

    void PresetManager::savePreset(const String& presetName, const String& artistName)
    {
        if (presetName.isEmpty())
            return;

        currentPreset.setValue(presetName);

        auto state = valueTreeState.copyState();
        state.setProperty("artist", artistName, nullptr);
        state.setProperty("dateCreated", Time::getCurrentTime().toISO8601(true), nullptr);
        state.setProperty("dateModified", Time::getCurrentTime().toISO8601(true), nullptr);

        const auto xml = state.createXml();
        const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);

        if (!xml->writeTo(presetFile))
        {
            DBG("Could not create preset file: " + presetFile.getFullPathName());
            jassertfalse;
        }

        updatePresetList();
    }

    void PresetManager::deletePreset(const String& presetName)
    {
        if (presetName.isEmpty())
            return;

        const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);

        if (!presetFile.existsAsFile())
        {
            DBG("Preset file " + presetFile.getFullPathName() + " does not exist");
            jassertfalse;
            return;
        }

        
        if (!presetFile.moveToTrash())
        {
            DBG("Preset file " + presetFile.getFullPathName() + " could not be deleted");
            jassertfalse;
            return;
        }

        currentPreset.setValue("");
        updatePresetList();
    
    }

    void PresetManager::loadPreset(const String& presetName)
    {
        if (presetName.isEmpty())
            return;

        const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);
        if (!presetFile.existsAsFile())
        {
            DBG("Preset file " + presetFile.getFullPathName() + " does not exist");
            jassertfalse;
            return;
        }

        XmlDocument xmlDocument{ presetFile };
        std::unique_ptr<XmlElement> xml(xmlDocument.getDocumentElement());
        if (xml == nullptr)
        {
            DBG("Invalid XML in preset file");
            jassertfalse;
            return;
        }

        auto valueTreeToLoad = ValueTree::fromXml(*xml);
        valueTreeToLoad.setProperty("dateModified", Time::getCurrentTime().toISO8601(true), nullptr);

        valueTreeState.replaceState(valueTreeToLoad);
        currentPreset.setValue(presetName);

        updatePresetList();
    }

    int PresetManager::loadNextPreset()
    {
        if (availablePresets.isEmpty())
            return -1;

        const auto currentIndex = availablePresets.indexOf(currentPreset.toString());
        const auto nextIndex = currentIndex + 1 > (availablePresets.size() - 1) ? 0 : currentIndex + 1;
        loadPreset(availablePresets[nextIndex]);
        return nextIndex;
    }

    int PresetManager::loadPreviousPreset()
    {
        if (availablePresets.isEmpty())
            return -1;

        const auto currentIndex = availablePresets.indexOf(currentPreset.toString());
        const auto previousIndex = currentIndex - 1 < 0 ? availablePresets.size() - 1 : currentIndex - 1;
        loadPreset(availablePresets[previousIndex]);
        return previousIndex;
    }

    StringArray PresetManager::getAllPresets() const
    {
        StringArray presets;
        const auto fileArray = defaultDirectory.findChildFiles(File::findFiles, false, "*." + extension);
        for (const auto& file : fileArray)
            presets.add(file.getFileNameWithoutExtension());
        return presets;
    }

    Array<PresetMetadata> PresetManager::getAllPresetMetadata() const
    {
        Array<PresetMetadata> result;
        const auto fileArray = defaultDirectory.findChildFiles(File::findFiles, false, "*." + extension);

        for (const auto& file : fileArray)
        {
            XmlDocument xmlDocument(file);
            std::unique_ptr<XmlElement> xml(xmlDocument.getDocumentElement());

            if (xml != nullptr)
            {
                auto tree = ValueTree::fromXml(*xml);
                PresetMetadata meta;
                meta.name = file.getFileNameWithoutExtension();
                meta.artist = tree.getProperty("artist", "Unknown").toString();
                meta.dateCreated = tree.getProperty("dateCreated", "").toString();
                meta.dateModified = tree.getProperty("dateModified", "").toString();
                result.add(meta);
            }
        }

        return result;
    }

    String PresetManager::getCurrentPreset() const
    {
        return currentPreset.toString();
    }

    void PresetManager::valueTreeRedirected(ValueTree& treeWhichHasBeenChanged)
    {
        currentPreset.referTo(treeWhichHasBeenChanged.getPropertyAsValue(presetNameProperty, nullptr));
    }

    void PresetManager::updatePresetList()
    {
        availablePresets = getAllPresets();
    }
}
