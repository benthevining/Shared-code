
namespace bav::network
{
OscDataSynchronizer::OscDataSynchronizer (SerializableData& dataToUse, juce::OSCSender& s, juce::OSCReceiver& r)
    : DataSynchronizer (dataToUse),
      addressPattern (dataToUse.dataIdentifier.toString()),
      oscSender (s),
      oscReceiver (r)
{
    oscReceiver.addListener (this);
}

OscDataSynchronizer::~OscDataSynchronizer()
{
    oscReceiver.removeListener (this);
}

void OscDataSynchronizer::sendChangeData (const void* data, size_t dataSize)
{
    outgoingData.replaceWith (data, dataSize);
    oscSender.send (addressPattern, outgoingData);
}

void OscDataSynchronizer::oscMessageReceived (const juce::OSCMessage& message)
{
    if (message.getAddressPattern() != addressPattern || message.isEmpty())
        return;

    auto& arg = message[0];

    if (arg.isBlob())
    {
        auto& block = arg.getBlob();
        applyChangeData (block.getData(), block.getSize());
    }
}

}  // namespace bav::network
