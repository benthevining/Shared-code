
namespace bav
{
juce::File getPresetsFolder (std::string companyName,
                             std::string pluginName)
{
    juce::File rootFolder;

#if JUCE_WINDOWS
    rootFolder = juce::File::getSpecialLocation (
        juce::File::SpecialLocationType::userDocumentsDirectory);
#else
    rootFolder = juce::File::getSpecialLocation (
        juce::File::SpecialLocationType::userApplicationDataDirectory);

#    if JUCE_MAC
    rootFolder = rootFolder.getChildFile ("Audio").getChildFile ("Presets");
#    endif
#endif

    rootFolder = rootFolder.getChildFile (companyName).getChildFile (pluginName);

    if (! rootFolder.isDirectory())
        rootFolder.createDirectory();

    return rootFolder;
}


//==============================================================================


juce::String addFileExtensionIfMissing (const String& string,
                                        const String& extension)
{
    return juce::File::createLegalFileName (
        string.endsWith (extension) ? string.trim() : (string + extension).trim());
}


juce::String removeFileExtensionIfThere (const String& string,
                                         const String& extension)
{
    return (string.endsWith (extension))
             ? string.dropLastCharacters (extension.length()).trim()
             : string.trim();
}


//==============================================================================

bool renameFile (const juce::File& f, const String& newName)
{
    if (! f.existsAsFile())
        return false;

    return f.moveFileTo (f.getSiblingFile (newName));
}

void deleteFile (const juce::File& f)
{
    if (f.existsAsFile())
        if (! f.moveToTrash())
            f.deleteFile();
}

}  // namespace bav
