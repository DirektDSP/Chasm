/*

Simple label component for displaying timestamps in a user interface.
Used for checking build times when developing.

*/


#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;

class ExtLabel : public Label
{
public:
	ExtLabel() : Label("", "")
	{
		setMouseCursor(MouseCursor::PointingHandCursor);
  		setTooltip("Visit DirektDSP.com for more information");
		setLookAndFeel(&mainLabelLookAndFeel);

		// create a string with the current build time
		auto buildTime = String(__DATE__) + " " + String(__TIME__);

		setText("DirektDSP - " + buildTime, NotificationType::dontSendNotification);
	}

	// click = open link
	void mouseDown(const MouseEvent& event) override
	{
		if (event.mods.isLeftButtonDown())
		{
			URL("https://direktdsp.com").launchInDefaultBrowser();
		}
	}

	~ExtLabel()
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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtLabel)
};
