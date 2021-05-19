
namespace bav
{

static inline int midiPanStringToInt (const juce::String& string)
{
    if (string.endsWithIgnoreCase (TRANS ("C")))
        return 64;
    
    if (string.endsWithIgnoreCase (TRANS ("R")))
        return juce::jmap (string.dropLastCharacters(1).getIntValue(), 1, 50, 65, 127);
    
    if (string.endsWithIgnoreCase (TRANS ("L")))
        return juce::jmap (string.dropLastCharacters(1).getIntValue(), 1, 50, 63, 0);
    
    return string.getIntValue();
}

static inline juce::String midiPanIntToString (const int midiPan)
{
    if (midiPan == 64)
        return juce::String (TRANS ("C"));
    
    if (midiPan > 64)
    {
        const auto amtRight = juce::jmap (midiPan, 65, 127, 1, 50);
        return juce::String (amtRight) + TRANS ("R");
    }
    
    const auto amtLeft = juce::jmap (midiPan, 63, 0, 1, 50);
    return juce::String (amtLeft) + TRANS ("L");
}
}


namespace bav::midi
{
/*  Returns the pitch class number for a given string.
 If the string is not in the required format i.e. A#4 etc. this
 will return -1.
 */
inline int pitchClassFromString (const juce::String& pitchClassName) noexcept
{
    int pitchClass = -1;
    const auto numChars = pitchClassName.length();
    
    if (numChars > 0)
    {
        const auto base = pitchClassName.toLowerCase()[0];
        
        switch (base)
        {
            case 'c':   pitchClass = 0;     break;
            case 'd':   pitchClass = 2;     break;
            case 'e':   pitchClass = 4;     break;
            case 'f':   pitchClass = 5;     break;
            case 'g':   pitchClass = 7;     break;
            case 'a':   pitchClass = 9;     break;
            case 'b':   pitchClass = 11;    break;
            default:    pitchClass = -1;    break;
        }
    }
    
    if (numChars > 1)
    {
        const auto sharpOrFlat = pitchClassName[1];
        
        switch (sharpOrFlat)
        {
            case '#':   ++pitchClass;       break;
            case 'b':   --pitchClass;       break;
            default:                        break;
        }
        
        if (sharpOrFlat == getSharpSymbol())
            ++pitchClass;
        else if (sharpOrFlat == getFlatSymbol())
            --pitchClass;
        
        pitchClass %= 12;
    }
    
    return pitchClass;
}

// converts a midi pitch to a string representation, eg note 69 = A4
inline juce::String pitchToString (const int midiNoteNumber, const bool asSharps = true)
{
    jassert (midiNoteNumber >= 0 && midiNoteNumber <= 127);
    
    const auto pitchClass = midiNoteNumber % 12;
    
    constexpr auto inv12 = 1.0f / 12.0f;
    const auto octave = juce::roundToInt (float(midiNoteNumber) * inv12) - 1;
    
    static const char* const sharpNoteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    static const char* const flatNoteNames[]  = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
    auto noteName = asSharps ? juce::String (sharpNoteNames[pitchClass]) : juce::String (flatNoteNames[pitchClass]);
    
    return noteName + juce::String (octave);
}

// converts a pitch class string to an integer representation, eg A4 = note 69
inline int stringToPitch (const juce::String& string)
{
    const auto octaveName = string.retainCharacters ("0123456789");
    
    const auto pitchClassTokens = juce::String("abcdefg#") + getSharpSymbol() + getFlatSymbol() + getNaturalSymbol();
    const auto pitchClassName = string.toLowerCase().retainCharacters (pitchClassTokens);
    
    const auto octave = octaveName.getIntValue();
    const auto pitchClass = pitchClassFromString (pitchClassName);
    jassert (pitchClass > -1);
    
    return (octave * 12) + pitchClass;
}


// returns true if the given midinote is a black key
template<typename noteType>
inline bool isMidiNoteBlackKey (noteType midipitch)
{
    jassert (midipitch >= noteType(0) && midipitch <= noteType(127));
    
    switch (midipitch % 12)
    {
        case 1:  return true;
        case 3:  return true;
        case 6:  return true;
        case 8:  return true;
        case 10: return true;
        default: return false;
    }
}


// returns true if the two pitches are the same pitch class, regardless of octave
template<typename noteType1, typename noteType2>
inline bool areNotesSamePitchClass (const noteType1 pitch1, const noteType2 pitch2)
{
    jassert (pitch1 >= noteType1(0) && pitch2 >= noteType2(0) && pitch1 <= noteType1(127) && pitch2 <= noteType2(127));
    return (pitch1 % 12 == pitch2 % 12);
}


// copies a range of events from one juce::MidiBuffer to another, optionally providing a sample offset
inline void copyRangeOfMidiBuffer (const juce::MidiBuffer& readingBuffer, juce::MidiBuffer& destBuffer,
                                   const int startSampleOfInput,
                                   const int startSampleOfOutput,
                                   const int numSamples)
{
    destBuffer.clear (startSampleOfOutput, numSamples);
    destBuffer.addEvents (readingBuffer, startSampleOfInput, numSamples, startSampleOfOutput - startSampleOfInput);
}


} // namespace

