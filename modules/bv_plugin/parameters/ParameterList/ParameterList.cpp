
namespace bav
{
ParameterList::ParameterList (juce::Identifier listName)
    : SerializableData (listName)
{
}

void ParameterList::add (ParamHolderBase& param)
{
    addParameter (param, false);
}

void ParameterList::addInternal (ParamHolderBase& param)
{
    addParameter (param, true);
}

void ParameterList::addParameter (ParamHolderBase& param, bool isInternal)
{
#if JUCE_DEBUG
    for (auto meta : params)
        jassert (meta.holder.getParam()->dataIdentifier != param.getParam()->dataIdentifier);
#endif

    params.add ({param, isInternal});
}

void ParameterList::addParametersTo (juce::AudioProcessor& processor)
{
    for (auto meta : params)
    {
        if (meta.holder.isInternal)
            meta.holder.addTo (processor);
        else
            meta.holder.addTo (dummyProcessor);

        meta.holder.getParam()->sendListenerSyncCallback();
    }
}

void ParameterList::addAllParametersAsInternal()
{
    for (auto meta : params)
        meta.holder.addTo (dummyProcessor);

    sendCallbackToAllListeners();
}

void ParameterList::sendCallbackToAllListeners()
{
    for (auto meta : params)
        meta.holder.getParam()->sendListenerSyncCallback();
}

int ParameterList::getNumParameters() const
{
    return params.size();
}

void ParameterList::refreshAllDefaults()
{
    for (auto meta : params)
        meta.holder.getParam()->refreshDefault();
}

void ParameterList::resetAllToDefault()
{
    for (auto meta : params)
        meta.holder.getParam()->resetToDefault();
}

void ParameterList::doAllActions (bool force)
{
    for (auto meta : params)
        meta.holder.getParam()->doAction (force);
}

void ParameterList::toValueTree (ValueTree& tree)
{
    for (auto meta : params)
        meta.holder.getParam()->serialize (tree);
}

void ParameterList::fromValueTree (const ValueTree& tree)
{
    for (auto meta : params)
        meta.holder.getParam()->deserialize (tree);
}

void ParameterList::setUndoManager (juce::UndoManager& um)
{
    for (auto meta : params)
        meta.holder.getParam()->setUndoManager (um);
}

void ParameterList::processMidi (const juce::MidiBuffer& midiMessages, bool triggerActions)
{
    for (auto meta : midiMessages)
        processMidiMessage (meta.getMessage(), triggerActions);
}

void ParameterList::processMidiMessage (const juce::MidiMessage& message, bool triggerAction)
{
    if (message.isController())
        processNewControllerMessage (message.getControllerNumber(), message.getControllerValue(), triggerAction);
}

void ParameterList::processNewControllerMessage (int controllerNumber, int controllerValue, bool triggerAction)
{
    for (auto meta : params)
        meta.holder.getParam()->processNewControllerMessage (controllerNumber, controllerValue, triggerAction);
}


/*-------------------------------------------------------------*/


ParameterList::ParamHolderMetadata::ParamHolderMetadata (ParamHolderBase& h, bool internal)
    : holder (h), isInternal (internal)
{
}


}  // namespace bav
