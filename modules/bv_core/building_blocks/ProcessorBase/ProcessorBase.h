
#pragma once


namespace bav::dsp
{

class ProcessorBase :   private SystemInitializer,
                        public juce::AudioProcessor
{
public:
    ProcessorBase();
    virtual ~ProcessorBase() override;

    void prepareToPlay (double, int) override;
    void releaseResources() override;

    void processBlock (juce::AudioBuffer< float >&, juce::MidiBuffer&) override;

    double getTailLengthSeconds() const override;

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    int                getNumPrograms() override;
    int                getCurrentProgram() override;
    void               setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void               changeProgramName (int, const juce::String&);

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool supportsMPE() const override;
    bool isMidiEffect() const override;

    juce::String getName() const override;

    bool                        hasEditor() const override;
    juce::AudioProcessorEditor* createEditor() override;
    
    bool isBusesLayoutSupported(const BusesLayout& layout) const override;
    
    virtual void saveEditorSize (int width, int height);
    
    virtual juce::Point<int> getSavedEditorSize() const;

    /*=========================================================================================*/

protected:
    virtual juce::AudioProcessor::BusesProperties createBusProperties() const;
    
    juce::Point<int> savedEditorSize;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DummyAudioProcessor)
};


}  // namespace bav::dsp
