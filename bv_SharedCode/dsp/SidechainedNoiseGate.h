
namespace bav
{

namespace dsp
{

template<typename SampleType>
class SidechainedNoiseGate
{
public:
    SidechainedNoiseGate()
    {
        update();
        
        RMSFilter.setLevelCalculationType (juce::dsp::BallisticsFilterLevelCalculationType::RMS);
        RMSFilter.setAttackTime  (static_cast<SampleType> (0.0));
        RMSFilter.setReleaseTime (static_cast<SampleType> (50.0));
    }
    
    
    void setThreshold (SampleType newThreshold_dB)
    {
        thresholddB = newThreshold_dB;
        update();
    }
    
    /** Sets the ratio of the noise-gate (must be higher or equal to 1).*/
    void setRatio (SampleType newRatio)
    {
        jassert (newRatio >= static_cast<SampleType> (1.0));
        
        ratio = newRatio;
        update();
    }
    
    
    void setAttack (SampleType newAttack_ms)
    {
        attackTime = newAttack_ms;
        update();
    }
    
    
    void setRelease (SampleType newRelease_ms)
    {
        releaseTime = newRelease_ms;
        update();
    }
    
    
    void prepare (int numChannels, int maxBlocksize, double samplerate)
    {
        jassert (samplerate > 0);
        jassert (numChannels > 0);
        
        spec.sampleRate = samplerate;
        spec.maximumBlockSize = juce::uint32(maxBlocksize);
        spec.numChannels = juce::uint32(numChannels);
        
        RMSFilter.prepare (spec);
        envelopeFilter.prepare (spec);
        
        update();
        reset();
    }
    
    
    void reset()
    {
        RMSFilter.reset();
        envelopeFilter.reset();
    }
    
    
    void process (const juce::AudioBuffer<SampleType>& sidechain,
                  juce::AudioBuffer<SampleType>& signal) noexcept
    {
        const int numChannels = signal.getNumChannels();
        const int numSamples  = signal.getNumSamples();
        
        jassert (sidechain.getNumChannels() == numChannels);
        jassert (sidechain.getNumSamples() == numSamples);
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* sidechainSamples = sidechain.getReadPointer (channel);
            auto* readingSamples = signal.getReadPointer (channel);
            auto* writingSamples = signal.getWritePointer (channel);
            
            for (int i = 0; i < numSamples; ++i)
                writingSamples[i] = processSample (channel, sidechainSamples[i], readingSamples[i]);
        }
    }
    
    
    /** Performs the processing operation on a single sample at a time. */
    SampleType processSample (const int channel,
                              const SampleType sidechainValue,
                              const SampleType sampleToGate)
    {
        // RMS ballistics filter
        auto env = RMSFilter.processSample (channel, sidechainValue);
        
        // Ballistics filter
        env = envelopeFilter.processSample (channel, env);
        
        // VCA
        auto gain = (env > threshold) ? static_cast<SampleType> (1.0)
                                      : std::pow (env * thresholdInverse, currentRatio - static_cast<SampleType> (1.0));
        
        // Output
        return gain * sampleToGate;
    }
    
    
    
private:
    
    void update()
    {
        threshold = juce::Decibels::decibelsToGain (thresholddB, static_cast<SampleType> (-200.0));
        thresholdInverse = static_cast<SampleType> (1.0) / threshold;
        currentRatio = ratio;
        
        envelopeFilter.setAttackTime  (attackTime);
        envelopeFilter.setReleaseTime (releaseTime);
    }
    
    
    juce::dsp::ProcessSpec spec;
    
    SampleType threshold, thresholdInverse, currentRatio;
    juce::dsp::BallisticsFilter<SampleType> envelopeFilter, RMSFilter;
    
    SampleType thresholddB = -100, ratio = 10.0, attackTime = 1.0, releaseTime = 100.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidechainedNoiseGate)
};
    

template class SidechainedNoiseGate<float>;
template class SidechainedNoiseGate<double>;

}  // namespace dsp

}  // namespace bav
