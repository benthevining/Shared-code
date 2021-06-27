
#pragma once


namespace bav::dsp::psola
{
template < typename SampleType >
class AnalysisGrainExtractor
{
    using IArray = juce::Array< int >;
    using FArray = juce::Array< float >;

public:
    AnalysisGrainExtractor()  = default;
    ~AnalysisGrainExtractor() = default;

    void prepare (int maxBlocksize);

    void releaseResources();

    void analyzeInput (IArray&           targetArray,
                       const SampleType* inputSamples,
                       int               numSamples,
                       int               period);

private:
    void findPsolaPeaks (IArray&           targetArray,
                         const SampleType* reading,
                         int               totalNumSamples,
                         int               period);

    int findNextPeak (int               frameStart,
                      int               frameEnd,
                      int               predictedPeak,
                      const SampleType* reading,
                      const IArray&     targetArray,
                      int               period,
                      int               grainSize);

    void sortSampleIndicesForPeakSearching (IArray& output,
                                            int     startSample,
                                            int     endSample,
                                            int     predictedPeak);

    void getPeakCandidateInRange (IArray&           candidates,
                                  const SampleType* input,
                                  int               startSample,
                                  int               endSample,
                                  int               predictedPeak,
                                  const IArray&     searchingOrder);

    int chooseIdealPeakCandidate (const IArray&     candidates,
                                  const SampleType* reading,
                                  int               deltaTarget1,
                                  int               deltaTarget2);

    int choosePeakWithGreatestPower (const IArray&     candidates,
                                     const SampleType* reading);

    IArray peakIndices;
    IArray peakCandidates;
    IArray peakSearchingOrder;

    FArray candidateDeltas;
    IArray finalHandful;
    FArray finalHandfulDeltas;

    static constexpr auto numPeaksToTest          = 10;
    static constexpr auto defaultFinalHandfulSize = 5;
};


}  // namespace bav::dsp::psola
