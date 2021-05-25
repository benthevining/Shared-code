
namespace bav
{

ParameterList::ParameterList (juce::Identifier listName)
: SerializableData (listName)
{ }

void ParameterList::add (ParamHolderBase& param)
{
    addParameter (param, true);
}

void ParameterList::addInternal (ParamHolderBase& param)
{
    addParameter (param, false);
}

void ParameterList::addParameter (ParamHolderBase& param, bool isInternal)
{
    params.add ({ param, isInternal });
}

void ParameterList::addParametersTo (juce::AudioProcessor& processor)
{
    for (auto meta : params)
    {
        if (meta.holder.isInternal)
            meta.holder.addTo (processor);
        else
            meta.holder.addTo (dummyProcessor);
    }
}

void ParameterList::addAllParametersAsInternal()
{
    for (auto meta : params)
        meta.holder.addTo (dummyProcessor);
}

Parameter* ParameterList::getParameter (juce::String parameterNameVerbose) const
{
    for (auto meta : params)
    {
        auto* param = meta.holder.getParam();
        if (param->parameterNameVerbose == parameterNameVerbose)
            return param;
    }

    return nullptr;
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

void ParameterList::doAllActions()
{
    for (auto meta : params)
        meta.holder.getParam()->doAction();
}

juce::ValueTree ParameterList::toValueTree() const
{
    juce::ValueTree tree {"Parameters"};
    
    for (auto meta : params)
        meta.serialize (tree);
    
    return tree;
}

void ParameterList::restoreFromValueTree (const juce::ValueTree& tree)
{
    for (auto meta : params)
        meta.deserialize (tree);
}

void ParameterList::toValueTree (ValueTree& tree) const
{
    for (auto meta : params)
        meta.serialize (tree);
}

void ParameterList::fromValueTree (const ValueTree& tree)
{
    for (auto meta : params)
        meta.deserialize (tree);
}

ParameterList::ParamHolderMetadata::ParamHolderMetadata (ParamHolderBase& h, bool internal)
: SerializableData (h.getParam()->parameterNameVerbose + "_Metadata"),
holder (h), isInternal(internal)
{ }

void ParameterList::ParamHolderMetadata::toValueTree (ValueTree& tree) const
{
    holder.getParam()->serialize (tree);
}

void ParameterList::ParamHolderMetadata::fromValueTree (const ValueTree& tree) 
{
    holder.getParam()->deserialize (tree);
}

}  // namespace bav
