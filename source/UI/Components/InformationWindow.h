// This component is used to display information about the current plugin in a small window.
// main info incl. version, author, credits and shit

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace UI
{
    namespace Components
    {
        class InformationWindow : public juce::Component
        {
        public:
            InformationWindow()
            {
                // Set up the UI components and layout
            }

            ~InformationWindow()
            {
            }

        private:
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InformationWindow)
        };
    }
}
