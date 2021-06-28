
#pragma once

#include "Grains/SynthesisGrain.h"

namespace bav::dsp::psola
{
template < typename SampleType >
class Shifter
{
public:
    using Analyzer    = Analyzer< SampleType >;
    using Storage     = AnalysisGrainStorage< SampleType >;
    using Grain       = psola::SynthesisGrain< SampleType >;
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    Shifter (Analyzer& parentAnalyzer);

    void setPitch (float desiredFrequency, double samplerate);

    void getSamples (AudioBuffer& output, int channel = 0);
    void getSamples (SampleType* outputSamples, int numSamples);

    SampleType getNextSample();

private:
    void   startNewGrain();
    Grain* getAvailableGrain() const;
    bool   areAnyGrainsActive() const;

    Analyzer&       analyzer;
    const Storage&  storage {analyzer.getStorage()};

    juce::OwnedArray< Grain > grains;

    int desiredPeriod {0};
    int samplesToNextGrain {0};
    int currentSample {0};  // the current sample in the frame

    events::Listener l;
};


}  // namespace bav::dsp::psola
