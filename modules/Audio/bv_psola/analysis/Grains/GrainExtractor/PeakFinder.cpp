
namespace bav::dsp::psola
{
template < typename SampleType >
void PeakFinder< SampleType >::releaseResources()
{
    peakIndices.clear();
    peakCandidates.clear();
    peakSearchingOrder.clear();
    candidateDeltas.clear();
    finalHandful.clear();
    finalHandfulDeltas.clear();
}

template < typename SampleType >
void PeakFinder< SampleType >::prepare (int maxBlocksize)
{
    jassert (maxBlocksize > 0);

    peakIndices.ensureStorageAllocated (maxBlocksize);

    peakCandidates.ensureStorageAllocated (numPeaksToTest + 1);
    peakCandidates.clearQuick();
    peakSearchingOrder.ensureStorageAllocated (maxBlocksize);
    peakSearchingOrder.clearQuick();
    candidateDeltas.ensureStorageAllocated (numPeaksToTest);
    candidateDeltas.clearQuick();
    finalHandful.ensureStorageAllocated (numPeaksToTest);
    finalHandful.clearQuick();
    finalHandfulDeltas.ensureStorageAllocated (numPeaksToTest);
    finalHandfulDeltas.clearQuick();
}

template < typename SampleType >
void PeakFinder< SampleType >::findPeaks (IArray&           targetArray,
                                          const SampleType* reading,
                                          int               totalNumSamples,
                                          int               period)
{
    targetArray.clearQuick();

    // output grains are 2 periods long w/ 50% overlap
    const auto grainSize  = 2 * period;
    const auto halfPeriod = juce::roundToInt ((float) period * 0.5f);

    jassert (totalNumSamples >= grainSize);

    int analysisIndex =
        halfPeriod;  // marks the center of the analysis windows, which are 1 period long

    do
    {
        const auto frameStart = analysisIndex - halfPeriod;
        const auto frameEnd =
            std::min (totalNumSamples,
                      frameStart + period);  // analysis grains are 1 period long

        jassert (frameStart >= 0 && frameEnd <= totalNumSamples);

        targetArray.add (findNextPeak (
            frameStart,
            frameEnd,
            std::min (analysisIndex,
                      frameEnd),  // predicted peak location for this frame
            reading,
            targetArray,
            period,
            grainSize));

        jassert (! targetArray.isEmpty());

        const auto prevAnalysisIndex = analysisIndex;
        const auto targetArraySize   = targetArray.size();

        // analysisIndex marks the middle of our next analysis window, so it's where our next predicted peak should be:
        if (targetArraySize == 1)
            analysisIndex = targetArray.getUnchecked (0) + period;
        else
            analysisIndex =
                targetArray.getUnchecked (targetArraySize - 2) + grainSize;

        if (analysisIndex == prevAnalysisIndex)
            analysisIndex = prevAnalysisIndex + period;
        else
            jassert (analysisIndex > prevAnalysisIndex);
    } while (analysisIndex - halfPeriod < totalNumSamples);
}


template < typename SampleType >
int PeakFinder< SampleType >::findNextPeak (int               frameStart,
                                            int               frameEnd,
                                            int               predictedPeak,
                                            const SampleType* reading,
                                            const IArray&     targetArray,
                                            int               period,
                                            int               grainSize)
{
    jassert (frameEnd > frameStart);
    jassert (predictedPeak >= frameStart && predictedPeak <= frameEnd);

    peakSearchingOrder.clearQuick();
    sortSampleIndicesForPeakSearching (
        peakSearchingOrder, frameStart, frameEnd, predictedPeak);

    jassert (peakSearchingOrder.size() == frameEnd - frameStart);

    peakCandidates.clearQuick();

    for (int i = 0; i < numPeaksToTest; ++i)
        getPeakCandidateInRange (peakCandidates,
                                 reading,
                                 frameStart,
                                 frameEnd,
                                 predictedPeak,
                                 peakSearchingOrder);

    jassert (! peakCandidates.isEmpty());

    switch (peakCandidates.size())
    {
        case 1 : return peakCandidates.getUnchecked (0);

        case 2 : return choosePeakWithGreatestPower (peakCandidates, reading);

        default :
        {
            if (targetArray.size() <= 1)
                return choosePeakWithGreatestPower (peakCandidates, reading);

            return chooseIdealPeakCandidate (
                peakCandidates,
                reading,
                targetArray.getLast() + period,
                targetArray.getUnchecked (targetArray.size() - 2) + grainSize);
        }
    }
}


template < typename SampleType >
void PeakFinder< SampleType >::getPeakCandidateInRange (
    IArray&           candidates,
    const SampleType* input,
    int               startSample,
    int               endSample,
    int               predictedPeak,
    const IArray&     searchingOrder)
{
    jassert (! searchingOrder.isEmpty());

    int starting = -1;

    for (int poss : searchingOrder)
    {
        if (! candidates.contains (poss))
        {
            starting = poss;
            break;
        }
    }

    if (starting == -1) return;

    jassert (starting >= startSample && starting <= endSample);

    const auto weight = [] (int index, int predicted, int numSamples)
    {
        return static_cast< SampleType > (
            1.0 - ((abs (index - predicted) / numSamples) * 0.5));
    };

    const auto numSamples = endSample - startSample;

    auto localMin        = input[starting] * weight (starting, predictedPeak, numSamples);
    auto localMax        = localMin;
    auto indexOfLocalMin = starting;
    auto indexOfLocalMax = starting;

    for (int index : searchingOrder)
    {
        if (index == starting || candidates.contains (index)) continue;

        jassert (index >= startSample && index <= endSample);

        const auto currentSample =
            input[index] * weight (index, predictedPeak, numSamples);

        if (currentSample < localMin)
        {
            localMin        = currentSample;
            indexOfLocalMin = index;
        }

        if (currentSample > localMax)
        {
            localMax        = currentSample;
            indexOfLocalMax = index;
        }
    }

    if (indexOfLocalMax == indexOfLocalMin)
    {
        candidates.add (indexOfLocalMax);
    }
    else if (localMax < SampleType (0.0))
    {
        candidates.add (indexOfLocalMin);
    }
    else if (localMin > SampleType (0.0))
    {
        candidates.add (indexOfLocalMax);
    }
    else
    {
        candidates.add (std::min (indexOfLocalMax, indexOfLocalMin));
        candidates.add (std::max (indexOfLocalMax, indexOfLocalMin));
    }
}


template < typename SampleType >
int PeakFinder< SampleType >::chooseIdealPeakCandidate (
    const IArray&     candidates,
    const SampleType* reading,
    int               deltaTarget1,
    int               deltaTarget2)
{
    candidateDeltas.clearQuick();
    finalHandful.clearQuick();
    finalHandfulDeltas.clearQuick();

    // 1. calculate delta values for each peak candidate
    // delta represents how far off this peak candidate is from the expected peak location - in a way it's a measure of the jitter that picking a peak candidate as this frame's peak would introduce to the overall alignment of the stream of grains based on the previous grains

    for (int candidate : candidates)
    {
        candidateDeltas.add (
            (abs (candidate - deltaTarget1) + abs (candidate - deltaTarget2))
            * 0.5f);
    }

    // 2. whittle our remaining candidates down to the final candidates with the minimum delta values

    const auto finalHandfulSize =
        std::min (defaultFinalHandfulSize, candidateDeltas.size());

    float      minimum      = 0.0f;
    int        minimumIndex = 0;
    const auto dataSize     = candidateDeltas.size();

    for (int i = 0; i < finalHandfulSize; ++i)
    {
        bav::vecops::findMinAndMinIndex (
            candidateDeltas.getRawDataPointer(), dataSize, minimum, minimumIndex);

        finalHandfulDeltas.add (minimum);
        finalHandful.add (candidates.getUnchecked (minimumIndex));

        candidateDeltas.set (
            minimumIndex,
            10000.0f);  // make sure this value won't be chosen again, w/o deleting it from the candidateDeltas array
    }

    jassert (finalHandful.size() == finalHandfulSize
             && finalHandfulDeltas.size() == finalHandfulSize);

    // 3. choose the strongest overall peak from these final candidates, with peaks weighted by their delta values

    const auto deltaRange = bav::vecops::findRangeOfExtrema (
        finalHandfulDeltas.getRawDataPointer(), finalHandfulDeltas.size());

    if (deltaRange < 0.05f)  // prevent dividing by 0 in the next step...
        return finalHandful.getUnchecked (0);

    const auto deltaWeight = [] (float delta, float totalDeltaRange)
    {
        return 1.0f - (delta / totalDeltaRange);
    };

    auto chosenPeak = finalHandful.getUnchecked (0);
    auto strongestPeak =
        abs (reading[chosenPeak])
        * deltaWeight (finalHandfulDeltas.getUnchecked (0), deltaRange);

    for (int i = 1; i < finalHandfulSize; ++i)
    {
        const auto candidate = finalHandful.getUnchecked (i);

        if (candidate == chosenPeak) continue;

        auto testingPeak =
            abs (reading[candidate])
            * deltaWeight (finalHandfulDeltas.getUnchecked (i), deltaRange);

        if (testingPeak > strongestPeak)
        {
            strongestPeak = testingPeak;
            chosenPeak    = candidate;
        }
    }

    return chosenPeak;
}


template < typename SampleType >
int PeakFinder< SampleType >::choosePeakWithGreatestPower (
    const IArray& candidates, const SampleType* reading)
{
    auto strongestPeakIndex = candidates.getUnchecked (0);
    auto strongestPeak      = abs (reading[strongestPeakIndex]);

    for (int candidate : candidates)
    {
        const auto current = abs (reading[candidate]);

        if (current > strongestPeak)
        {
            strongestPeak      = current;
            strongestPeakIndex = candidate;
        }
    }

    return strongestPeakIndex;
}


template < typename SampleType >
void PeakFinder< SampleType >::sortSampleIndicesForPeakSearching (
    IArray& output,  // array to write the sorted sample indices to
    int     startSample,
    int     endSample,
    int     predictedPeak)
{
    jassert (predictedPeak >= startSample && predictedPeak <= endSample);

    output.clearQuick();

    output.set (0, predictedPeak);

    int p = 1, m = -1;

    for (int n = 1; n < (endSample - startSample); ++n)
    {
        const auto pos = predictedPeak + p;
        const auto neg = predictedPeak + m;

        if (n % 2 == 0)  // n is even
        {
            if (neg >= startSample)
            {
                output.set (n, neg);
                --m;
            }
            else
            {
                jassert (pos <= endSample);
                output.set (n, pos);
                ++p;
            }
        }
        else
        {
            if (pos <= endSample)
            {
                output.set (n, pos);
                ++p;
            }
            else
            {
                jassert (neg >= startSample);
                output.set (n, neg);
                --m;
            }
        }
    }
}

template class PeakFinder< float >;
template class PeakFinder< double >;

}  // namespace bav::dsp::psola
