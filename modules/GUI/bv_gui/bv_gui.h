#pragma once

#if 0

 BEGIN_JUCE_MODULE_DECLARATION

 ID:                 bv_gui
 vendor:             Ben Vining
 version:            0.0.1
 name:               bv_gui
 description:        GUI utility classes and some basic components
 dependencies:       bv_plugin

 END_JUCE_MODULE_DECLARATION
 
#endif


#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wunused-function"
#endif

#include <bv_plugin/bv_plugin.h>

#include "utilities/GuiUtils.h"
#include "utilities/Spline.h"
#include "icons/icons.h"
#include "components/SynthMidiKeyboardComponent/SynthMidiKeyboardComponent.h"
#include "components/SynthStaffDisplayComponent/SynthStaffDisplayComponent.h"

#include "components/parameter_connected/ParameterConnectedComponents.h"
#include "components/meter_connected/LevelMeters.h"

#ifdef __clang__
#    pragma clang diagnostic pop
#endif