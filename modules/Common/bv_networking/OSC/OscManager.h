
#pragma once

namespace bav::network
{
struct OscManager : SerializableData
{
public:
    OscManager (const String& dataName = "DataSync");
    virtual ~OscManager() override;

    void connect (const String& targetHostName, int portNumber = 53100);
    void disconnect();

    juce::OSCSender   sender;
    juce::OSCReceiver receiver;

private:
    void serialize (TreeReflector& ref) final;

    int    portNum {53100};
    String hostName;
};

}  // namespace bav::network
