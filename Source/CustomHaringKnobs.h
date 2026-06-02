#pragma once
#include <JuceHeader.h>

template<int KnobIndex>
struct KnobImageData
{
    static const char* imageData();
    static int imageSize();
};

// each knob index maps to a different image in BinaryData via template specialization
// so the compiler wires up the right png at compile time with no runtime branching
template<> inline const char* KnobImageData<0>::imageData() { return BinaryData::knob0_png; }
template<> inline int KnobImageData<0>::imageSize() { return BinaryData::knob0_pngSize; }

template<> inline const char* KnobImageData<1>::imageData() { return BinaryData::knob1_png; }
template<> inline int KnobImageData<1>::imageSize() { return BinaryData::knob1_pngSize; }

template<> inline const char* KnobImageData<2>::imageData() { return BinaryData::knob2_png; }
template<> inline int KnobImageData<2>::imageSize() { return BinaryData::knob2_pngSize; }


template<int KnobIndex>
class HaringKnob : public juce::LookAndFeel_V4
{
public:
    // loads the knob and track images from BinaryData into JUCE's ImageCache
    // ImageCache ref-counts the decoded bitmaps so multiple knobs sharing the same
    // image don't decode it more than once
    HaringKnob()
    {
        knobImage  = juce::ImageCache::getFromMemory(KnobImageData<KnobIndex>::imageData(),
                                                     KnobImageData<KnobIndex>::imageSize());

        trackImage = juce::ImageCache::getFromMemory(BinaryData::track_png,
                                                     BinaryData::track_pngSize);
    }

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        float centreX = x + width  * 0.5f;
        float centreY = y + height * 0.5f;

        float trackW = width * 1.44f;
        float trackH = height * 1.44f;

        // draws the track image slightly larger than the knob bounds (1.44x) so the
        // decorative ring visually extends beyond the hit area of the slider
        g.drawImage(trackImage,
                    (int)(centreX - trackW * 0.5f), (int)(centreY - trackH * 0.5f),
                    (int)trackW, (int)trackH,
                    0, 0, trackImage.getWidth(), trackImage.getHeight());

        float knobScale = 1.0f / 3.0f * 0.8f;
        float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // builds an AffineTransform that centers the raw bitmap on the origin, scales it
        // down, rotates it to the current parameter angle, then translates it to the
        // knob's center — order matters here since JUCE applies transforms right-to-left
        juce::AffineTransform t = juce::AffineTransform::identity
            .translated(-knobImage.getWidth()  * 0.5f,
                        -knobImage.getHeight() * 0.5f)
            .scaled(knobScale)
            .rotated(angle)
            .translated(centreX, centreY);

        g.drawImageTransformed(knobImage, t, false);
    }

private:
    juce::Image knobImage;
    juce::Image trackImage;
};
