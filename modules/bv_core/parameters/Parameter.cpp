
namespace bav
{
Parameter::Parameter (RangedParam& p,
                      juce::String paramNameShort,
                      juce::String paramNameVerbose)
    : SerializableData (paramNameVerbose),
      rap (p),
      parameterNameShort (TRANS (paramNameShort)),
      parameterNameVerbose (TRANS (paramNameVerbose))
{
    currentDefault = rap.getDefaultValue();
    lastActionedValue = currentDefault;
}

Parameter::~Parameter()
{
}

bool Parameter::operator== (const Parameter& other)
{
    return parameterNameShort == other.parameterNameShort && parameterNameVerbose == other.parameterNameVerbose;
}

void Parameter::beginGesture()
{
    if (changing)
        return;
    
    if (um != nullptr)
    {
        um->beginNewTransaction();
        um->setCurrentTransactionName (TRANS("Changed") + " " + parameterNameVerbose);
    }
    
    changing = true;
    rap.beginChangeGesture();
    
    listeners.call ([](Listener& l){ l.gestureStateChanged (true); });
    
    if (onGestureStateChange)
        bav::callOnMessageThread< bool > (onGestureStateChange, true);
}

void Parameter::endGesture()
{
    if (! changing)
        return;
    
    changing = false;
    rap.endChangeGesture();
    
    listeners.call ([](Listener& l){ l.gestureStateChanged (false); });
    
    if (onGestureStateChange)
        bav::callOnMessageThread< bool > (onGestureStateChange, false);
}

bool Parameter::isChanging() const
{
    return changing;
}

float Parameter::getNormalizedDefault() const
{
    return currentDefault;
}

float Parameter::getDenormalizedDefault() const
{
    return denormalize (currentDefault);
}

void Parameter::setNormalizedDefault (float value)
{
    jassert (value >= 0.0f && value <= 1.0f);

    if (currentDefault == value) return;
    
    currentDefault = value;
    
    listeners.call ([&value](Listener& l){ l.defaultChanged (value); });

    if (onDefaultChange)
        bav::callOnMessageThread (onDefaultChange);
    
    if (um != nullptr)
    {
        
    }
}

void Parameter::setDenormalizedDefault (float value)
{
    setNormalizedDefault (normalize (value));
}

void Parameter::refreshDefault()
{
    currentDefault = getCurrentNormalizedValue();
}

void Parameter::resetToDefault()
{
    rap.setValueNotifyingHost (currentDefault);
}

void Parameter::setNormalizedValue (float value)
{
    if (value == rap.getValue()) return;
    
    if (! changing)
        beginGesture();
    
    rap.setValueNotifyingHost (value);
    
    listeners.call ([&value](Listener& l){ l.valueChanged (value); });
    
    if (onParameterChange)
        bav::callOnMessageThread (onParameterChange);
}

void Parameter::setDenormalizedValue (float value)
{
    setNormalizedValue (normalize (value));
}

float Parameter::getCurrentNormalizedValue() const
{
    return rap.getValue();
}

float Parameter::getCurrentDenormalizedValue() const
{
    return rap.convertFrom0to1 (rap.getValue());
}

float Parameter::normalize (float input) const
{
    return rap.convertTo0to1 (input);
}

float Parameter::denormalize (float input) const
{
    return rap.convertFrom0to1 (input);
}

void Parameter::setUndoManager (juce::UndoManager& managerToUse)
{
    um = &managerToUse;
}

void Parameter::doAction()
{
    const auto value = getCurrentNormalizedValue();

    if (value != lastActionedValue)
    {
        lastActionedValue = value;
        onAction();
    }
}


Parameter::Listener::Listener (Parameter& paramToUse)
: param (paramToUse)
{
    param.addListener (this);
}

Parameter::Listener::~Listener()
{
    param.removeListener (this);
}

void Parameter::Listener::valueChanged (float) { }

void Parameter::Listener::gestureStateChanged (bool) { }

void Parameter::Listener::defaultChanged (float) { }

}  // namespace bav
