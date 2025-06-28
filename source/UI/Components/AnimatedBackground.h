#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AnimatedBackground : public juce::Slider
{
public:

	AnimatedBackground() : juce::Slider(SliderStyle::RotaryHorizontalVerticalDrag, TextEntryBoxPosition::NoTextBox)
	{	
		setMouseCursor(juce::MouseCursor::PointingHandCursor);
		setLookAndFeel(&mainSliderLookAndFeel);
	}

	~AnimatedBackground()
	{
		setLookAndFeel(nullptr);
	}

	class AnimatedBackgroundLookAndFeel : public juce::LookAndFeel_V4
	{
	public:

		AnimatedBackgroundLookAndFeel()
		{
			image = juce::ImageCache::getFromMemory(BinaryData::Knob1_png, BinaryData::Knob1_pngSize);
			// byImage = juce::ImageCache::getFromMemory(juce::BinaryData::LittlePhatty_OFF_png, juce::BinaryData::LittlePhatty_OFF_pngSize);
		}

		~AnimatedBackgroundLookAndFeel() override
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


		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimatedBackgroundLookAndFeel)
	} mainSliderLookAndFeel;
	
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimatedBackground)
};

