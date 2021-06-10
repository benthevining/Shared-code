#pragma once

namespace bav
{
class PresetManagerBase
{
public:
    PresetManagerBase (StateBase& stateToUse);
    PresetManagerBase (StateBase& stateToUse, UndoManager& undoManagerToUse);
    virtual ~PresetManagerBase() = default;

    virtual std::string getCompanyName()         = 0;
    virtual std::string getProductName()         = 0;
    virtual std::string getPresetFileExtension() = 0;

    juce::File presetsFolder();

    bool loadPreset (const String& presetName);
    void savePreset (const String& presetName);
    void deletePreset (const String& presetName);
    bool renamePreset (const String& previousName, const String& newName);

    void rescanPresetsFolder();

    const juce::StringArray& presetNames();

    events::Broadcaster& getBroadcaster() { return broadcaster; }

private:
    juce::File presetNameToFilePath (const String& presetName);

    StateBase&          state;
    juce::StringArray   namesOfAvailablePresets;
    events::Broadcaster broadcaster;

    UndoManager* undoManager = nullptr;
};

}  // namespace bav
