/*

Simple label component for displaying timestamps in a user interface.
Used for checking build times when developing.

*/


#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;

class TimestampLabel : public Label
{
public:
	TimestampLabel() : Label("", "")
	{
		setMouseCursor(MouseCursor::PointingHandCursor);
  		setTooltip("Visit DirektDSP.com for more information");
		setLookAndFeel(&mainLabelLookAndFeel);
	}

	// click = open link
	void mouseDown(const MouseEvent& event) override
	{
		if (event.mods.isLeftButtonDown())
		{
			URL("https://direktdsp.com").launchInDefaultBrowser();
		}
	}

	~TimestampLabel()
	{
		setLookAndFeel(nullptr);
	}

private:
	struct MainLabelLookAndFeel : public LookAndFeel_V4
	{
		MainLabelLookAndFeel()
		{
		}

	private:
		juce::Colour bgColour;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainLabelLookAndFeel)
	} mainLabelLookAndFeel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimestampLabel)
};
