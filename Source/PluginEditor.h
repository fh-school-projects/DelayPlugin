/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomHaringKnobs.h"

//==============================================================================
/**
*/
class DelayPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DelayPluginAudioProcessorEditor (DelayPluginAudioProcessor&);
    ~DelayPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DelayPluginAudioProcessor& audioProcessor;
    
    juce::Slider mDryWetSlider;
    juce::Slider mFeedbackSlider;
    juce::Slider mDelayTimeSlider;

    juce::Label mDryWetLabel;
    juce::Label mFeedbackLabel;
    juce::Label mDelayTimeLabel;

    HaringKnob<0> mDryWetKnobLookAndFeel;
    HaringKnob<1> mFeedbackKnobLookAndFeel;
    HaringKnob<2> mDelayTimeKnobLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayPluginAudioProcessorEditor)
};
