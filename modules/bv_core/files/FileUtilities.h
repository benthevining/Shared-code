
#pragma once

namespace bav
{
/*
 this function attempts to return the default location your plugin's preset files should be saved to and loaded from.
 if the directory cannot be found for your plugin, calling this function will attempt to create it.
 */
extern juce::File getPresetsFolder (std::string companyName,
                                    std::string pluginName);


//==============================================================================


extern juce::String addFileExtensionIfMissing (const String& string,
                                               const String& extension);

extern juce::String removeFileExtensionIfThere (const String& string,
                                                const String& extension);


//==============================================================================


extern bool renameFile (const juce::File& f, const String& newName);

extern void deleteFile (const juce::File& f);


}  // namespace bav
