
namespace bav
{

SerializableData::SerializableData (juce::Identifier identifier)
    : dataIdentifier (identifier)
{
}

SerializableData::~SerializableData()
{
}


bool SerializableData::operator== (const SerializableData& other) const
{
    return dataIdentifier == other.dataIdentifier;
}


ValueTree SerializableData::serialize()
{
    ValueTree tree {dataIdentifier};

    toValueTree (tree);

    for (auto* child : children)
        child->serialize (tree);

    return tree;
}


ValueTree& SerializableData::serialize (ValueTree& t)
{
    t.appendChild (serialize(), nullptr);
    return t;
}


void SerializableData::deserialize (const ValueTree& t)
{
    if (t.hasType (dataIdentifier))
    {
        setTree (t);
    }
    else
    {
        setTree (t.getChildWithName (dataIdentifier));
    }
}

void SerializableData::addDataChild (SerializableData& child)
{
    jassert (&child != this);
    children.addIfNotAlreadyThere (&child);
}

void SerializableData::addDataChild (SerializableData* child)
{
    if (child != nullptr)
        addDataChild (*child);
}

void SerializableData::setTree (const ValueTree& newTree)
{
    if (! newTree.isValid()) return;

    fromValueTree (newTree);

    for (auto& child : children)
        child->deserialize (newTree);
}

}  // namespace bav