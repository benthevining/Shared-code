/******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_dsp
 vendor:             Ben Vining
 version:            0.0.1
 name:               bv_dsp
 description:        DSP and audio utilities and effects
 dependencies:       juce_audio_utils juce_dsp bv_SharedCode
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/

#pragma once

// juce dependencies
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>


#include <bv_SharedCode/bv_SharedCode.h>


#include "Oscillators/oscillators.h"
#include "Oscillators/LFO/LFO.h"
#include "FIFOs/AudioFIFO.h"
#include "FIFOs/AudioAndMidiFIFO.h"
#include "FIFOWrappedEngine/FIFOWrappedEngine.h"
#include "FFT/bv_FFT.h"
#include "PitchDetector/pitch-detector.h"
#include "PSOLA/analysis/psola_analyzer.h"
#include "PSOLA/resynthesis/psola_shifter.h"

// FX
#include "FX/ReorderableFxChain.h"
#include "FX/dynamics/SmoothedGain.h"
#include "FX/dynamics/NoiseGate.h"
#include "FX/DeEsser.h"
#include "FX/Distortion.h"
#include "FX/dynamics/Compressor.h"
#include "FX/Reverb.h"
#include "FX/dynamics/Limiter.h"
#include "FX/stereo_image/MonoStereoConverter.h"
#include "FX/stereo_image/panning/PannerBase.h"
#include "FX/stereo_image/panning/MonoToStereoPanner.h"
#include "FX/stereo_image/panning/StereoPanner.h"
#include "FX/stereo_image/StereoWidener.h"
#include "FX/time/BeatRepeat.h"
#include "FX/time/Delay.h"
#include "FX/time/Freezer.h"
#include "FX/time/Looper.h"
#include "FX/pitch/PitchShifter.h"
#include "FX/pitch/PitchCorrector.h"