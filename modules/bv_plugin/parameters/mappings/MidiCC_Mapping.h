
#pragma once

namespace bav
{
/*
        A simple listener class that allows you to map a specified parameter to a new MIDI controller number.
    */

class MidiCC_Listener
{
public:
    MidiCC_Listener (Parameter& param, int controller)
        : parameter (param), controllerNum (controller), lastControllerValue (defaultLastControllerVal)
    {
    }

    ~MidiCC_Listener() = default;

    void processMidiMessage (const juce::MidiMessage& msg)
    {
        if (msg.isController())
            processNewControllerMessage (msg.getControllerNumber(),
                                         msg.getControllerValue());
    }

    void processNewControllerMessage (int controllerNumber, int controllerValue)
    {
        constexpr auto inv127 = 1.0f / 127.0f;

        if (controllerNumber == controllerNum)
        {
            parameter.rap.setValueNotifyingHost (controllerValue * inv127);
            lastControllerValue = controllerValue;
        }
    }

    void changeControllerNumber (int newControllerNumber) noexcept
    {
        controllerNum       = newControllerNumber;
        lastControllerValue = defaultLastControllerVal;
    }

    int getControllerNumber() const noexcept { return controllerNum; }

    int getLastControllerValue() const noexcept { return lastControllerValue; }


    Parameter& parameter;


private:
    int controllerNum;
    int lastControllerValue;

    static constexpr int defaultLastControllerVal = 64;
};


/*
        A manager class that owns a set of MidiCCmapper objects and iterates through them to process all the CC changes in a midi buffer at a time
    */

class MidiCC_MappingManager
{
public:
    MidiCC_MappingManager()
        : lastMovedController (0), lastControllerValue (0)
    {
    }

    ~MidiCC_MappingManager() { mappings.clear(); }

    /* adds a new MidiCC_Listener linked to the specified parameter and listening for the specified MIDI controller. */
    void addParameterMapping (Parameter& parameter, int controllerNumber)
    {
        mappings.add (new MidiCC_Listener (parameter, controllerNumber));
    }

    /* clears any previous mappings this object held and loads the new specified set of mappings */
    void loadMappingSet (MidiCC_Listener* parameterMappings, int numMappings)
    {
        mappings.clear();
        mappings.ensureStorageAllocated (numMappings);

        for (int i = 0; i < numMappings; ++i)
        {
            auto* mapping = parameterMappings + i;
            mappings.add (new MidiCC_Listener (mapping->parameter,
                                               mapping->getControllerNumber()));
        }
    }

    /* if a mapping exists for the current parameter, it will be changed to now listen for the new CC number. If no mapping exists for the parameter, returns false. */
    bool changeParameterMapping (const Parameter& parameter, int newControllerNumber)
    {
        for (auto* mapping : mappings)
        {
            if (mapping->parameter == parameter)
            {
                mapping->changeControllerNumber (newControllerNumber);
                return true;
            }
        }
        return false;
    }

    /* removes all mappings to the passed parameter */
    void removeAllParameterMappingsFor (const Parameter& parameter)
    {
        for (auto* mapping : mappings)
            if (mapping->parameter == parameter)
                mappings.removeObject (mapping);
    }

    /* removes all mappings to the passed CC number */
    void removeAllParameterMappingsFor (int controllerNumber)
    {
        for (auto* mapping : mappings)
            if (mapping->getControllerNumber() == controllerNumber)
                mappings.removeObject (mapping);
    }

    /* removes only mappings that map the passed parameter to the passed CC nnumber */
    void removeParameterMapping (const Parameter& parameter, int controllerNumber)
    {
        for (auto* mapping : mappings)
            if (mapping->parameter == parameter
                && mapping->getControllerNumber() == controllerNumber)
                mappings.removeObject (mapping);
    }

    /* clears all active parameter mappings */
    void clearAllMappings() { mappings.clear(); }


    /* processes all the MIDI CC events in the passed midi buffer */
    void processMidiBuffer (const juce::MidiBuffer& midiMessages)
    {
        std::for_each (midiMessages.cbegin(),
                       midiMessages.cend(),
                       [&] (const juce::MidiMessageMetadata& meta)
                       { processMidiEvent (meta.getMessage()); });
    }

    /* processes a single MIDI event at a time */
    void processMidiEvent (const juce::MidiMessage& msg)
    {
        if (msg.isController())
            processNewControllerMessage (msg.getControllerNumber(),
                                         msg.getControllerValue());
    }

    /* processes raw MIDI CC data. You can call this function manually to simulate the effect of recieving a MIDI CC message. */
    void processNewControllerMessage (int controllerNumber, int controllerValue)
    {
        for (auto* mapping : mappings)
            mapping->processNewControllerMessage (controllerNumber, controllerValue);

        lastMovedController.store (controllerNumber);
        lastControllerValue.store (controllerValue);
    }

    /* if the parameter is currently mapped, returns a pointer to its MidiCC_Listener; else returns a nullptr */
    MidiCC_Listener*
        getMappingForParameter (const Parameter& parameter) const noexcept
    {
        for (auto* mapping : mappings)
            if (mapping->parameter == parameter) return mapping;

        return nullptr;
    }

    /* if the passed MIDI controller number is mapped, returns a pointer to its MidiCC_Listener; else returns a nullptr */
    MidiCC_Listener*
        getMappingForController (int midiControllerNumber) const noexcept
    {
        for (auto* mapping : mappings)
            if (mapping->getControllerNumber() == midiControllerNumber)
                return mapping;

        return nullptr;
    }


    void getLastMovedController (int* controllerNumber,
                                 int* controllerValue = nullptr)
    {
        if (controllerNumber != nullptr)
            *controllerNumber = lastMovedController.load();

        if (controllerValue != nullptr)
            *controllerValue = lastControllerValue.load();
    }


private:
    juce::OwnedArray< MidiCC_Listener > mappings;
    std::atomic< int >                  lastMovedController, lastControllerValue;
};

}  // namespace bav