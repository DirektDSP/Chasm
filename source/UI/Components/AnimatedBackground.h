#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace UI
{
    namespace Components
    {
        class AnimatedBackground : public juce::Component
        {
        public:
            AnimatedBackground()
            {
                using namespace juce;

                // Load sprite sheets
                framesheets.push_back(ImageCache::getFromMemory(BinaryData::sprite_sheet_0_png, BinaryData::sprite_sheet_0_pngSize));
                framesheets.push_back(ImageCache::getFromMemory(BinaryData::sprite_sheet_1_png, BinaryData::sprite_sheet_1_pngSize));
                framesheets.push_back(ImageCache::getFromMemory(BinaryData::sprite_sheet_2_png, BinaryData::sprite_sheet_2_pngSize));
                framesheets.push_back(ImageCache::getFromMemory(BinaryData::sprite_sheet_3_png, BinaryData::sprite_sheet_3_pngSize));
                framesheets.push_back(ImageCache::getFromMemory(BinaryData::sprite_sheet_4_png, BinaryData::sprite_sheet_4_pngSize));
                framesheets.push_back(ImageCache::getFromMemory(BinaryData::sprite_sheet_5_png, BinaryData::sprite_sheet_5_pngSize));
                framesheets.push_back(ImageCache::getFromMemory(BinaryData::sprite_sheet_6_png, BinaryData::sprite_sheet_6_pngSize));
                framesheets.push_back(ImageCache::getFromMemory(BinaryData::sprite_sheet_7_png, BinaryData::sprite_sheet_7_pngSize));

                // Fill frames per sheet for safety
                for (const auto& sheet : framesheets)
                {
                    const int framesX = sheet.getWidth() / FRAME_WIDTH;
                    const int framesY = sheet.getHeight() / FRAME_HEIGHT;
                    framesPerSheet.push_back(framesX * framesY);
                }
            }

            ~AnimatedBackground() override = default;

            void setLeftFrame(int frame)
            {
                currentLeftFrame = juce::jlimit(0, totalFrames() - 1, frame);
                repaint();
            }

            void setRightFrame(int frame)
            {
                currentRightFrame = juce::jlimit(0, totalFrames() - 1, frame);
                repaint();
            }

            void paint(juce::Graphics& g) override
            {
                if (getWidth() <= 0 || getHeight() <= 0)
                    return;

                int leftHalfWidth = getWidth()*0.56f;
                int rightHalfWidth = getWidth()-leftHalfWidth;

                drawFrame(g, currentLeftFrame, 0, 0, leftHalfWidth, getHeight());
                drawFrame(g, currentRightFrame, leftHalfWidth, 0, rightHalfWidth, getHeight());
            }

        private:
            std::vector<juce::Image> framesheets;
            std::vector<int> framesPerSheet;

            const int FRAME_WIDTH = 1000;
            const int FRAME_HEIGHT = 600;
            const int splitX = static_cast<int>(FRAME_WIDTH*0.56f);

            int currentLeftFrame = 0;
            int currentRightFrame = 0;

            int totalFrames() const
            {
                int total = 0;
                for (auto f : framesPerSheet)
                    total += f;
                return total;
            }

            void drawFrame(juce::Graphics& g, int frameIndex, int dstX, int dstY, int dstW, int dstH)
            {
                if (frameIndex < 0 || dstW <= 0 || dstH <= 0)
                    return;

                // Find the correct sheet and local index
                int sheetIndex = 0;
                int localFrame = frameIndex;
                for (size_t i = 0; i < framesPerSheet.size(); ++i)
                {
                    if (localFrame < framesPerSheet[i])
                    {
                        sheetIndex = static_cast<int>(i);
                        break;
                    }
                    localFrame -= framesPerSheet[i];
                }

                #pragma region "Error Checking"

                // index too big
                if (sheetIndex >= static_cast<int>(framesheets.size()))
                    return;

                // make sure image is ok
                const auto& sheet = framesheets[sheetIndex];
                if (!sheet.isValid())
                    return;

                // get the max number of frames a really weird way lol
                const int framesX = sheet.getWidth() / FRAME_WIDTH;
                const int maxFrames = framesX * (sheet.getHeight() / FRAME_HEIGHT);

                // we gon TOO far
                if (localFrame >= maxFrames)
                    return;

                #pragma endregion

                // actually get the starting x and y for the frame we want
                const int frameX = (localFrame % framesX) * FRAME_WIDTH;
                const int frameY = (localFrame / framesX) * FRAME_HEIGHT;

                // if we end up going out of bounds
                if (frameX + FRAME_WIDTH > sheet.getWidth() || frameY + FRAME_HEIGHT > sheet.getHeight())
                    return;

                // we need to only get the section we want from the frame
                // this can be taken from the dstX since we know the left frame has dstX == 0
                if (dstX == 0){
                    // left frame
                    // draw from 0 -> the split
                    g.drawImage(sheet, dstX, dstY, dstW, dstH, frameX, frameY, splitX, FRAME_HEIGHT);

                } else {
                    // left'nt (it's giving calc is short for calculator)
                    g.drawImage(sheet, dstX, dstY, dstW, dstH, frameX+splitX, frameY, FRAME_WIDTH - splitX, FRAME_HEIGHT);
                }

                
            }

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimatedBackground)
        };
    }
}
