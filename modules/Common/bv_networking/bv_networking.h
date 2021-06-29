#pragma once

#if 0

BEGIN_JUCE_MODULE_DECLARATION

ID:                bv_networking
vendor:            Ben Vining
version:           0.0.1
name:              bv_networking
description:       Some general networking and communication utilities.
dependencies:      bv_core juce_osc

END_JUCE_MODULE_DECLARATION

#endif


#include <bv_core/bv_core.h>
#include <juce_osc/juce_osc.h>

#include "DataSynchronizer/DataSynchronizer.h"

#include "OSC/OscListener.h"
#include "OSC/OscManager.h"
#include "OSC/ConnectionChecker/OscConnectionChecker.h"
#include "OSC/OscDataSynchronizer/OscDataSynchronizer.h"

#include "IPC/IpcDataSynchronizer.h"

#include "DataSynchronizer/server.h"