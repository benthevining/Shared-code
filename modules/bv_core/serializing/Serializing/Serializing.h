#pragma once

namespace bav::serializing
{
extern void toBinary (SerializableData& data, juce::File file);
extern void toBinary (SerializableData& data, juce::MemoryBlock& dest);


extern void fromBinary (juce::File file, SerializableData& dest);
extern void fromBinary (juce::MemoryBlock& data, SerializableData& dest);
extern void fromBinary (const void* data, size_t dataSizeInBytes, SerializableData& dest);
template < typename IntegerType >
void fromBinary (const void* data, IntegerType dataSizeInBytes, SerializableData& dest)
{
    fromBinary (data, static_cast< size_t > (dataSizeInBytes), dest);
}

extern std::unique_ptr< juce::XmlElement > toXML (SerializableData& source);
extern void                                toXML (SerializableData& source, const juce::File& file);

extern void fromXML (const juce::XmlElement& xml, SerializableData& dest);
extern void fromXML (std::unique_ptr< juce::XmlElement > xml, SerializableData& dest);
extern void fromXML (const juce::File& xmlFile, SerializableData& dest);

extern String toJSON (SerializableData& source);
extern void   fromJSON (const String& jsonText, SerializableData& dest);

}  // namespace bav::serializing