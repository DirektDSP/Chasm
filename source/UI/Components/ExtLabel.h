#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ExtLabel : public juce::Label
{
public:
	ExtLabel() : Label("", "")
	{
		setLookAndFeel(&mainLabelLookAndFeel);
	}

	// click = open link
	void mouseDown(const juce::MouseEvent& event) override
	{
		if (event.mods.isLeftButtonDown())
		{
			juce::URL("https://direktdsp.com").launchInDefaultBrowser();
		}
	}

	~ExtLabel()
	{
		setLookAndFeel(nullptr);
	}

private:
	struct MainLabelLookAndFeel : public juce::LookAndFeel_V3
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
