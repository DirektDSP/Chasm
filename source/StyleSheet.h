#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginEditor.h"
#include "PluginProcessor.h"

class RasterKnob : public juce::Slider
{
public:

	RasterKnob() : juce::Slider(SliderStyle::RotaryHorizontalVerticalDrag, TextEntryBoxPosition::NoTextBox)
	{	
		setMouseCursor(MouseCursor::PointingHandCursor);
		setLookAndFeel(&mainSliderLookAndFeel);
	}

	~RasterKnob()
	{
		setLookAndFeel(nullptr);
	}

	class RasterKnobLookAndFeel : public juce::LookAndFeel_V4
	{
	public:

		RasterKnobLookAndFeel()
		{
			image = juce::ImageCache::getFromMemory(BinaryData::Knob1_png, BinaryData::Knob1_pngSize);
			// byImage = juce::ImageCache::getFromMemory(BinaryData::LittlePhatty_OFF_png, BinaryData::LittlePhatty_OFF_pngSize);
		}

		~RasterKnobLookAndFeel() override
		{
		}
		
		void drawRotarySlider(
			juce::Graphics& g,
			int x,
			int y,
			int width,
			int height,
			float sliderPosProportional,
			float /*rotaryStartAngle*/,
			float /*rotaryEndAngle*/,
			juce::Slider& /*slider*/) override
		{
			const auto frames = 128;
			const auto frameId = static_cast<int>(ceil(sliderPosProportional * (static_cast<float>(frames) - 1.0f)));

			// image is vertical strip, each frame is 1/100th of the height

			g.drawImage(image,
				x,
				y,
				width,
				height,
				0,
				frameId * image.getHeight() / frames,
				image.getWidth(),
				image.getHeight() / frames);
		}

	private:
		juce::Image image;
		juce::Image byImage;

		bool bp = false;


		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RasterKnobLookAndFeel)
	} mainSliderLookAndFeel;
	
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RasterKnob)
};



class BypassButton : public juce::ToggleButton
{

public:
	BypassButton() : juce::ToggleButton("")
	{
		setLookAndFeel(&mainButtonLookAndFeel);
	}

	~BypassButton()
	{
		setLookAndFeel(nullptr);
	}

private:
	class BypassButtonLookAndFeel : public juce::LookAndFeel_V4
	{
	public:
		BypassButtonLookAndFeel()
		{
			image = ImageCache::getFromMemory(BinaryData::Knob1_png, BinaryData::Knob1_pngSize);
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
			const auto frames = 2;
			const auto frameId = static_cast<int>(ticked);

           Rectangle<float> Bounds(x, y, w, h);

			g.drawImage(image,
				x,
				y,
				w,
				h,
				0,
				frameId * image.getHeight() / frames,
				image.getWidth(),
				image.getHeight() / frames);
		}
       

	private:
		juce::Image image;
           
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BypassButtonLookAndFeel)
	} mainButtonLookAndFeel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BypassButton)
};


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


class GainSlider : public juce::Slider
{
public:
	GainSlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
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

	~GainSlider()
	{
		setLookAndFeel(nullptr);
	}
	
private:

	class GainSliderLookAndFeel : public juce::LookAndFeel_V4
	{
	public:
		GainSliderLookAndFeel()
		{
			//setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colours::white);
			//setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::white);
			//setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::white);
			//setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::white);
		}

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainSliderLookAndFeel)
	} mainSliderLookAndFeel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainSlider)
};
