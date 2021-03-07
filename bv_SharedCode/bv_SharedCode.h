/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_SharedCode
 vendor:             Ben Vining
 version:            0.0.1
 name:               Ben Vining's codebase
 description:        General utilities useful for developing plugins.
 dependencies:       juce_audio_utils
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

// dependency
#include <juce_audio_utils/juce_audio_utils.h>

// the rest of this module
#include "dsp/AudioFIFO.h"
#include "dsp/Panner.h"
#include "midi/MidiFIFO.h"
#include "midi/MidiUtilities.h"
#include "VectorOps.h"
#include "dsp/FIFOWrappedEngine.h"


namespace bav
{

namespace gui
{
    
    inline juce::Button::ButtonState buttonStateFromBool (const bool isOn)
    {
        if (isOn)
            return juce::Button::ButtonState::buttonDown;

        return juce::Button::ButtonState::buttonNormal;
    }
    
}  // namespace gui
    

namespace dsp
{
    
    inline int periodInSamples (double samplerate, int freqHz)
    {
        jassert (freqHz > 0);
        return juce::roundToInt(samplerate / freqHz);
    }
    
    inline int periodInSamples (double samplerate, float freqHz)
    {
        jassert (freqHz > 0.0f);
        return juce::roundToInt(samplerate / freqHz);
    }
    
    inline int freqFromPeriod (double samplerate, int period)
    {
        jassert (period > 0);
        return juce::roundToInt(samplerate / period);
    }
    
    inline int sampsToMs (double samplerate, int numSamples)
    {
        jassert (samplerate > 0.0);
        return juce::roundToInt((numSamples / samplerate) * 1000.0f);
    }
    
    inline int msToSamps (double samplerate, int ms) { return juce::roundToInt(samplerate / 1000.0f * ms); }
    
}  // namespace dsp

}  // namespace bav
