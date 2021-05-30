
#pragma once


namespace bav::dsp
{
/*
 SynthBase: Base class for a polyphonic synthesiser-style instrument.
 This class owns, manages & manipulates a collection of SynthVoiceBase (or SynthVoiceBase-derived) objects.
 */

template < typename SampleType >
class SynthBase
{
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using MidiBuffer  = juce::MidiBuffer;
    using MidiMessage = juce::MidiMessage;
    using uint32      = juce::uint32;
    using Voice       = SynthVoiceBase< SampleType >;
    using ADSRParams  = juce::ADSR::Parameters;


public:
    SynthBase();

    virtual ~SynthBase();

    void initialize (const int initNumVoices, const double initSamplerate, const int initBlocksize);

    void prepare (const int blocksize);

    void reset();

    void setCurrentPlaybackSampleRate (const double newRate);

    void renderVoices (juce::MidiBuffer& midiMessages, juce::AudioBuffer< SampleType >& output);

    void releaseResources();

    void resetRampedValues (int blocksize);

    void bypassedBlock (const int numSamples, MidiBuffer& midiMessages);

    void processMidiEvent (const MidiMessage& m);

    void playChord (const juce::Array< int >& desiredPitches, const float velocity = 1.0f, const bool allowTailOffOfOld = false);

    void allNotesOff (const bool allowTailOff, const float velocity = 1.0f);

    void turnOffAllKeyupNotes (const bool  allowTailOff,
                               const bool  includePedalPitchAndDescant,
                               const float velocity,
                               const bool  overrideSostenutoPedal);

    bool isPitchActive (const int midiPitch, const bool countRingingButReleased = false, const bool countKeyUpNotes = false) const;
    void reportActiveNotes (juce::Array< int >& outputArray, const bool includePlayingButReleased = false, const bool includeKeyUpNotes = true) const;

    int  getNumActiveVoices() const;
    int  getNumVoices() const noexcept { return voices.size(); }
    void changeNumVoices (const int newNumVoices);

    void setNoteStealingEnabled (const bool shouldSteal) noexcept { shouldStealNotes = shouldSteal; }
    void updateMidiVelocitySensitivity (int newSensitivity);
    void updatePitchbendSettings (const int rangeUp, const int rangeDown);
    void setAftertouchGainOnOff (const bool shouldBeOn) { aftertouchGainIsOn = shouldBeOn; }

    void setPedalPitch (const bool isOn);
    void setPedalPitchUpperThresh (int newThresh);
    void setPedalPitchInterval (const int newInterval);

    void setDescant (const bool isOn);
    void setDescantLowerThresh (int newThresh);
    void setDescantInterval (const int newInterval);

    /* If you are not using MTS-ESP, you can call this method to alter the master tuning of your synth. If you are using MTS-ESP, calling this method does nothing. */
    void setConcertPitchHz (const int newConcertPitchhz);

    void updateStereoWidth (int newWidth) { panner.updateStereoWidth (newWidth); }
    void updateLowestPannedNote (int newPitchThresh) { panner.setLowestNote (newPitchThresh); }

    void setMidiLatch (const bool shouldBeOn, const bool allowTailOff = false);
    bool isLatched() const noexcept { return latchIsOn; }

    void updateADSRsettings (const float attack, const float decay, const float sustain, const float release);
    void updateQuickReleaseMs (const int newMs);

    bool isSustainPedalDown() const noexcept { return midi.isSustainPedalDown(); }
    bool isSostenutoPedalDown() const noexcept { return midi.isSostenutoPedalDown(); }
    bool isSoftPedalDown() const noexcept { return midi.isSoftPedalDown(); }
    bool isAftertouchGainOn() const noexcept { return aftertouchGainIsOn; }

    void setPlayingButReleasedMultiplier (float newGain) { playingButReleasedMultiplier = newGain; }
    void setSoftPedalMultiplier (float newGain) { softPedalMultiplier = newGain; }

    int getLastBlocksize() const noexcept { return lastBlocksize; }

    bool         isConnectedToMtsEsp() const { return pitch.tuning.isConnectedToMtsEsp(); }
    juce::String getScaleName() const { return pitch.tuning.getScaleName(); }

    auto getLastMovedControllerInfo() const { return midi.getLastMovedCCinfo(); }

protected:
    friend class SynthVoiceBase< SampleType >;

    // if overridden, called in the subclass when the top-level call to initialize() is made.
    virtual void initialized (const double initSamplerate, const int initBlocksize) { juce::ignoreUnused (initSamplerate, initBlocksize); }

    // if overridden, called in the subclass when the top-level call to prepare() is made.
    virtual void prepared (int blocksize) { juce::ignoreUnused (blocksize); }

    // if overridden, called in the subclass when the top-level call to reset() is made.
    virtual void resetTriggered() { }

    // if overridden, called in the subclassed whenever the samplerate is changed.
    virtual void samplerateChanged (double newSamplerate) { juce::ignoreUnused (newSamplerate); }

    // if overridden, called in the subclass when the top-level call to releaseResources() is made.
    virtual void release() { }

    // this method should return an instance of your synth's voice subclass
    virtual Voice* createVoice() = 0;

private:
    void renderVoicesInternal (AudioBuffer& output, const int startSample, const int numSamples);

    void addNumVoices (const int voicesToAdd);
    void removeNumVoices (const int voicesToRemove);
    void numVoicesChanged();

    void noteOn (const int midiPitch, const float velocity, const bool isKeyboard = true, const int midiChannel = -1);
    void noteOff (const int midiNoteNumber, const float velocity, const bool allowTailOff, const bool isKeyboard = true);
    void startVoice (Voice* voice, const int midiPitch, const float velocity, const bool isKeyboard, const int midiChannel = -1);
    void stopVoice (Voice* voice, const float velocity, const bool allowTailOff);
    void turnOnList (const juce::Array< int >& toTurnOn, const float velocity, const bool partOfChord = false);
    void turnOffList (const juce::Array< int >& toTurnOff, const float velocity, const bool allowTailOff, const bool partOfChord = false);

    void pitchCollectionChanged();
    
    void updateChannelPressure (int newIncomingAftertouch);

    Voice* findFreeVoice (const bool stealIfNoneAvailable);
    Voice* findVoiceToSteal();

    Voice* getVoicePlayingNote (const int midiPitch) const;

    /*==============================================================================================================
     ===============================================================================================================*/

    juce::OwnedArray< Voice > voices;

    bool latchIsOn {false};

    juce::Array< int > currentNotes;
    juce::Array< int > desiredNotes;

    ADSRParams adsrParams;
    ADSRParams quickReleaseParams;

    double sampleRate {0.0};

    bool shouldStealNotes {true};

    midi::VelocityHelper velocityConverter;
    midi::PitchPipeline  pitch;

    bool aftertouchGainIsOn {true};

    float playingButReleasedMultiplier;

    float softPedalMultiplier;  // the multiplier by which each voice's output will be multiplied when the soft pedal is down

    juce::Array< Voice* > usableVoices;  // this array is used to sort the voices when a 'steal' is requested

    int lastBlocksize;

    MidiBuffer aggregateMidiBuffer;  // this midi buffer will be used to collect the harmonizer's aggregate MIDI output
    MidiBuffer midiInputStorage;     // each block of midi that comes in is stored in here so we can refer to it later

    //--------------------------------------------------

    class MidiManager : public midi::MidiProcessor
    {
    public:
        MidiManager (SynthBase& s) : synth (s) { }

    private:
        void handleNoteOn (int midiPitch, float velocity);
        void handleNoteOff (int midiPitch, float velocity);
        void handlePitchwheel (int wheelValue);
        void handleAftertouch (int noteNumber, int aftertouchValue);
        void handleChannelPressure (int channelPressureValue);
        void handleSustainPedal (int controllerValue);
        void handleSostenutoPedal (int controllerValue);
        void handleSoftPedal (int controllerValue);
        void handleAllSoundOff();

        SynthBase& synth;
    };

    MidiManager midi {*this};
    
    //--------------------------------------------------
    
    class PanningManager
    {
        using Array = juce::Array< int >;
        
    public:
        PanningManager (SynthBase& b): synth (b) { }
        
        void prepare (const int numVoices, bool clearArrays = true);
        void reset();
        
        void updateStereoWidth (const int newWidth);
        
        void setLowestNote (int newLowestNote);
        int  getLowestNote() const { return lowestPannedNote; }
        
        int  getNextPanVal();
        void panValTurnedOff (int panVal);
    
    private:
        void updatePanValueLookupTables (int newWidth);
        void mapArrayIndexes();
        int  getClosestNewPanValFromOld (int oldPan);
        int  findClosestValueInNewArray (int targetValue, Array& newArray);
        
        SynthBase& synth;
        
        int stereoWidth {100};
        int lowestPannedNote {0};
        
        Array arrayIndexesMapped;
        Array possiblePanVals, panValsInAssigningOrder, unsentPanVals;
        Array newPanVals, newUnsentVals;
        Array distances;
    };
    
    PanningManager panner {*this};

    //--------------------------------------------------

    class AutomatedHarmonyVoice
    {
    public:
        AutomatedHarmonyVoice (SynthBase& synthToUse, bool shiftUp);

        void apply();

        void setEnabled (bool shouldBeEnabled);
        void setThreshold (int newThresh);
        void setInterval (int newInterval);

        void turnNoteOffIfOn();
        void setNoteToOff() { lastPitch = -1; }

        bool isAutomatedPitch (int midiNote);

        // call this function when processing an automated note-off and the voice's keyboard key is still being held
        void autoNoteOffKeyboardKeyHeld (int midiNote);

        Voice* getVoice();

    private:
        const bool shiftingUp;

        bool isOn {false};
        int  lastPitch {-1};
        int  thresh {0};
        int  interval {12};

    private:
        SynthBase& synth;
    };

    AutomatedHarmonyVoice pedal {*this, false};
    AutomatedHarmonyVoice descant {*this, true};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthBase)
};


}  // namespace bav::dsp
