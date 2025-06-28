#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginEditor.h"
#include "PluginProcessor.h"

/*
class BypassButton : public juce::ToggleButton
{

public:
	BypassButton() : juce::ToggleButton("")
	{
		setLookAndFeel(&mainButtonLookAndFeel);
		setImage();
	}

	~BypassButton()
	{
		setLookAndFeel(nullptr);
	}

	void setImage()
	{
		offImage_ = ImageCache::getFromMemory(BinaryData::Toggle_png, BinaryData::Toggle_pngSize);
		onImage_ = ImageCache::getFromMemory(BinaryData::Toggle1_png, BinaryData::Toggle1_pngSize);
	}

	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override
	{
		const auto frameId = static_cast<int>(getToggleState());
		const auto image = frameId ? onImage_ : offImage_;
		g.drawImage(image, getLocalBounds().toFloat());


	}

private:
	juce::Image offImage_;
	juce::Image onImage_;



	class BypassButtonLookAndFeel : public juce::LookAndFeel_V4
	{
	public:
		BypassButtonLookAndFeel()
		{
		}

		void drawTickBox(Graphics& g, Component& component,
			float x,
			float y,
			float w,
			float h,
			bool ticked,
			bool isEnabled,
			bool isMouseOverButton,
			bool isButtonDown) override
		{
			// Do nothing, we're not drawing a tick box
		}

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BypassButtonLookAndFeel)
	} mainButtonLookAndFeel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BypassButton)
};
*/

/*
class ToggleSwitchButton : public juce::ToggleButton
{

public:
	ToggleSwitchButton() : juce::ToggleButton("")
	{
		setLookAndFeel(&mainButtonLookAndFeel);
		setImage();
	}

	~ToggleSwitchButton()
	{
		setLookAndFeel(nullptr);
	}

	void setImage()
	{
		offImage_ = ImageCache::getFromMemory(BinaryData::swSlide0000_png, BinaryData::swSlide0000_pngSize);
		onImage_ = ImageCache::getFromMemory(BinaryData::swSlide0001_png, BinaryData::swSlide0001_pngSize);
	}

	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override
	{
		const auto frameId = static_cast<int>(getToggleState());
		const auto image = frameId ? onImage_ : offImage_;
		g.drawImage(image, getLocalBounds().toFloat());


	}

private:
	juce::Image offImage_;
	juce::Image onImage_;



	class ToggleSwitchButtonLookAndFeel : public juce::LookAndFeel_V4
	{
	public:
		ToggleSwitchButtonLookAndFeel()
		{
		}

		void drawTickBox(Graphics& g, Component& component,
			float x,
			float y,
			float w,
			float h,
			bool ticked,
			bool isEnabled,
			bool isMouseOverButton,
			bool isButtonDown) override
		{
			// Do nothing, we're not drawing a tick box
		}

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleSwitchButtonLookAndFeel)
	} mainButtonLookAndFeel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToggleSwitchButton)
};
*/

