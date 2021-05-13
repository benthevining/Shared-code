
namespace bav::dsp
{

template<typename SampleType>
FIFOWrappedEngine<SampleType>::FIFOWrappedEngine()
    : internalBlocksize(0)
{ }

template<typename SampleType>
FIFOWrappedEngine<SampleType>::FIFOWrappedEngine (int consistentInternalBlocksize, double samplerate)
    : internalBlocksize(0)
{
    initialize (samplerate, consistentInternalBlocksize);
}


template<typename SampleType>
void FIFOWrappedEngine<SampleType>::process (AudioBuffer& inplaceInAndOut,
                                             MidiBuffer& midiMessages,
                                             const bool isBypassed)
{
    process (inplaceInAndOut, inplaceInAndOut, midiMessages, isBypassed);
}


template<typename SampleType>
void FIFOWrappedEngine<SampleType>::process (AudioBuffer& input, AudioBuffer& output,
                                             MidiBuffer& midiMessages,
                                             const bool isBypassed)
{
    jassert (! resourcesReleased);
    jassert (isInitialized);
    jassert (sampleRate > 0);
    
    const auto totalNumSamples = input.getNumSamples();
    
    if (totalNumSamples == 0)
        return;
    
    jassert (totalNumSamples == output.getNumSamples());
    
    const auto numInChannels  = std::min (2, input.getNumChannels());
    const auto numOutChannels = std::min (2, output.getNumChannels());
    
    if (numInChannels == 0 || numOutChannels == 0)
        return;
    
    bool processingBypassedThisFrame, applyFadeIn, applyFadeOut;
    
    if (isBypassed)
    {
        processingBypassedThisFrame = wasBypassedLastCallback;
        applyFadeIn = false;
        applyFadeOut = ! wasBypassedLastCallback;
    }
    else
    {
        processingBypassedThisFrame = false;
        applyFadeIn = wasBypassedLastCallback;
        applyFadeOut = false;
    }
    
    wasBypassedLastCallback = isBypassed;
    
    if (totalNumSamples <= internalBlocksize)
    {
        processWrapped (input, output, midiMessages, applyFadeIn, applyFadeOut, processingBypassedThisFrame);
        return;
    }
    
    int samplesLeft = totalNumSamples;
    int startSample = 0;
    
    do {
        const auto chunkNumSamples = std::min (internalBlocksize, samplesLeft);
        
        AudioBuffer inputProxy  (input.getArrayOfWritePointers(),  numInChannels,  startSample, chunkNumSamples);
        AudioBuffer outputProxy (output.getArrayOfWritePointers(), numOutChannels, startSample, chunkNumSamples);
        
        // put just the midi messages for this time segment into the midiChoppingBuffer, starting at timestamp 0
        midiChoppingBuffer.clear();
        bav::midi::copyRangeOfMidiBuffer (midiMessages, midiChoppingBuffer, startSample, 0, chunkNumSamples);
        
        processWrapped (inputProxy, outputProxy, midiChoppingBuffer, applyFadeIn, applyFadeOut, processingBypassedThisFrame);
        
        // copy the midi output back to midiMessages (I/O), at the original startSample
        bav::midi::copyRangeOfMidiBuffer (midiChoppingBuffer, midiMessages, 0, startSample, chunkNumSamples);
        
        startSample += chunkNumSamples;
        samplesLeft -= chunkNumSamples;
        
        applyFadeIn  = false;
        applyFadeOut = false;
        processingBypassedThisFrame = isBypassed;
    } while (samplesLeft > 0);
}


template<typename SampleType>
void FIFOWrappedEngine<SampleType>::processWrapped (AudioBuffer& input, AudioBuffer& output,
                                                    MidiBuffer& midiMessages,
                                                    const bool applyFadeIn, const bool applyFadeOut,
                                                    const bool isBypassed)
{
    const auto numNewSamples = input.getNumSamples();
    jassert (numNewSamples <= internalBlocksize && numNewSamples > 0);
    
    const auto numInChannels  = input.getNumChannels();
    const auto numOutChannels = output.getNumChannels();
    
    jassert (numInChannels <= 2 && numOutChannels <= 2);
    
    for (int chan = 0; chan < numInChannels; ++chan)
        inputBuffer.pushSamples (input, chan, 0, numNewSamples, chan);
    
    midiInputCollection.pushEvents (midiMessages, numNewSamples);
    
    if (inputBuffer.numStoredSamples() >= internalBlocksize)  // we have enough samples, render the new chunk
    {
        inBuffer.clear();
        
        for (int chan = 0; chan < numInChannels; ++chan)
            inputBuffer.popSamples (inBuffer, chan, 0, internalBlocksize, chan);
        
        chunkMidiBuffer.clear();
        midiInputCollection.popEvents (chunkMidiBuffer,  internalBlocksize);
        
        if (isBypassed)
        {
            for (int chan = 0; chan < numOutChannels; ++chan)
                outputBuffer.pushSamples (inBuffer, chan, 0, internalBlocksize, chan);
            
            bypassedBlock (inBuffer, chunkMidiBuffer);
        }
        else
        {
            outBuffer.clear();
            
            renderBlock (inBuffer, outBuffer, chunkMidiBuffer);
            
            for (int chan = 0; chan < numOutChannels; ++chan)
                outputBuffer.pushSamples (outBuffer, chan, 0, internalBlocksize, chan);
        }
        
        midiOutputCollection.pushEvents (chunkMidiBuffer, internalBlocksize);
    }
    
    for (int chan = 0; chan < numOutChannels; ++chan)
        outputBuffer.popSamples (output, chan, 0, numNewSamples, chan);
    
    midiOutputCollection.popEvents (midiMessages, numNewSamples);
    
    if (applyFadeIn)
        output.applyGainRamp (0, numNewSamples, 0.0f, 1.0f);
    
    if (applyFadeOut)
        output.applyGainRamp (0, numNewSamples, 1.0f, 0.0f);
}


template<typename SampleType>
void FIFOWrappedEngine<SampleType>::changeLatency (int newInternalBlocksize)
{
    jassert (newInternalBlocksize > 0);
    
    internalBlocksize = newInternalBlocksize;
    
    inBuffer.setSize  (2, internalBlocksize, true, true, true);
    outBuffer.setSize (2, internalBlocksize, true, true, true);
    
    const auto doubleBlocksize = internalBlocksize * 2;
    
    inputBuffer.changeSize (2, doubleBlocksize);
    outputBuffer.changeSize(2, doubleBlocksize);
    
    const auto doubleBlocksizeT = size_t(doubleBlocksize);
    
    midiInputCollection.setSize(doubleBlocksize);
    midiOutputCollection.setSize(doubleBlocksize);
    chunkMidiBuffer.ensureSize (doubleBlocksizeT);
    midiChoppingBuffer.ensureSize (doubleBlocksizeT);
    
    isInitialized = true;
    
    latencyChanged (newInternalBlocksize);
}


template<typename SampleType>
void FIFOWrappedEngine<SampleType>::initialize (double samplerate, int newInternalBlocksize)
{
    jassert (samplerate > 0);
    jassert (newInternalBlocksize > 0);
    
    wasBypassedLastCallback = true;
    isInitialized = true;
    resourcesReleased = false;
    
    initialized (newInternalBlocksize, samplerate);
    
    changeLatency (newInternalBlocksize);
}


template<typename SampleType>
void FIFOWrappedEngine<SampleType>::prepare (double samplerate)
{
    jassert (samplerate > 0);
    
    sampleRate = samplerate;
    
    if (! isInitialized)
    {
        jassert (internalBlocksize > 0); // if you hit this assertion, you probably made a call to prepare() before calling initialize().
        initialize (samplerate, internalBlocksize);
    }
    
    resourcesReleased = false;
    
    prepareToPlay (samplerate);
}


template<typename SampleType>
void FIFOWrappedEngine<SampleType>::reset()
{
    midiChoppingBuffer.clear();
    chunkMidiBuffer.clear();
    inBuffer.clear();
    outBuffer.clear();
    inputBuffer.clear();
    outputBuffer.clear();
    midiInputCollection.clear();
    midiOutputCollection.clear();
    
    resetTriggered();
}


template<typename SampleType>
void FIFOWrappedEngine<SampleType>::releaseResources()
{
    inBuffer.setSize (0, 0, false, false, false);
    outBuffer.setSize (0, 0, false, false, false);
    
    inputBuffer.releaseResources();
    outputBuffer.releaseResources();
    
    midiChoppingBuffer.clear();
    midiInputCollection.clear();
    midiOutputCollection.clear();
    chunkMidiBuffer.clear();
    
    wasBypassedLastCallback = true;
    resourcesReleased = true;
    isInitialized = false;
    
    release();
}


template class FIFOWrappedEngine<float>;
template class FIFOWrappedEngine<double>;

}  // namespace