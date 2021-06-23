
#pragma once

#include <bv_core/bv_core.h>
#include <bv_mobile_utils/bv_mobile_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>

namespace bav::gui
{
void scale (juce::Component& component, const juce::Rectangle< float >& boundsRatio);

template < typename... Args >
static inline void addAndMakeVisible (juce::Component* parent, juce::Component* firstChild, Args... rest)
{
    addAndMakeVisible (parent, firstChild);
    addAndMakeVisible (parent, std::forward< Args > (rest)...);
}

void addAndMakeVisible (juce::Component* parent, juce::Component* child);

juce::Button::ButtonState boolToButtonState (const bool isOn) noexcept;
bool                      buttonStateToBool (juce::Button::ButtonState state) noexcept;


struct GUIInitializer
{
    GUIInitializer (juce::Component& componentToUse);

    virtual ~GUIInitializer();

private:
#if JUCE_OPENGL
    OpenGLContext openGLContext;
#endif
};


/* RAII mechanism for changing the cursor & resetting it later */
struct ScopedCursor
{
    ScopedCursor (
        const juce::MouseCursor& cursor,
        juce::MouseCursor        cursorToResetTo = juce::MouseCursor::NormalCursor);

    virtual ~ScopedCursor();

private:
    const juce::MouseCursor resetCursor;
};


/* Sets the cursor to the system's default "wait" cursor, then back to the normal one */
struct ScopedWaitCursor : ScopedCursor
{
    ScopedWaitCursor();
};


}  // namespace bav::gui
