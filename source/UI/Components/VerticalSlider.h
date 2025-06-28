#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class VersticalSlider : public juce::Slider
{
public:
	VersticalSlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
	{
		setLookAndFeel(&mainSliderLookAndFeel);
		setRange(-60.0f, 12.0f, 0.1f);
		setValue(0.0f);
		setTextValueSuffix(" dB");
		setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colour::fromRGB(42, 42, 42));
		setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colour::fromRGB(42, 42, 42));
		setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::white);
		setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::white);
	}

	~VersticalSlider()
	{
		setLookAndFeel(nullptr);
	}
	
private:

	class VersticalSliderLookAndFeel : public juce::LookAndFeel_V4
	{
	public:
		VersticalSliderLookAndFeel()
		{
			//setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colours::white);
			//setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::white);
			//setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::white);
			//setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::white);
		}

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VersticalSliderLookAndFeel)
	} mainSliderLookAndFeel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VersticalSlider)
};

