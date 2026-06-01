/*
 ==============================================================================

 This file contains the basic framework code for a JUCE plugin editor.

 ==============================================================================
 */

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
DelayPluginAudioProcessorEditor::DelayPluginAudioProcessorEditor(
    DelayPluginAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
    auto &params = processor.getParameters();

    // Dry/wet slider
    juce::AudioParameterFloat *dryWetParameter =
        (juce::AudioParameterFloat *)params.getUnchecked(0);

    mDryWetSlider.setLookAndFeel(&mDryWetKnobLookAndFeel);
    mDryWetSlider.setBounds(0, 0, 100, 100); // repositioned in resized()
    mDryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDryWetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox,
                                  true, 0, 0);
    mDryWetSlider.setRange(dryWetParameter->range.start,
                           dryWetParameter->range.end);
    mDryWetSlider.setValue(*dryWetParameter);
    addAndMakeVisible(mDryWetSlider);

    mDryWetSlider.onDragStart = [dryWetParameter] {
        dryWetParameter->beginChangeGesture();
    };
    mDryWetSlider.onValueChange = [this, dryWetParameter] {
        *dryWetParameter = mDryWetSlider.getValue();
    };
    mDryWetSlider.onDragEnd = [dryWetParameter] {
        dryWetParameter->endChangeGesture();
    };

    // Feedback slider
    juce::AudioParameterFloat *feedbackParameter =
        (juce::AudioParameterFloat *)params.getUnchecked(1);

    mFeedbackSlider.setLookAndFeel(&mFeedbackKnobLookAndFeel);
    mFeedbackSlider.setBounds(100, 0, 100,
                              100); // 100px to the right of dry/wet
    mFeedbackSlider.setSliderStyle(
        juce::Slider::SliderStyle::RotaryVerticalDrag);
    mFeedbackSlider.setTextBoxStyle(
        juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mFeedbackSlider.setRange(feedbackParameter->range.start,
                             feedbackParameter->range.end);
    mFeedbackSlider.setValue(*feedbackParameter);
    addAndMakeVisible(mFeedbackSlider);

    mFeedbackSlider.onDragStart = [feedbackParameter] {
        feedbackParameter->beginChangeGesture();
    };
    mFeedbackSlider.onValueChange = [this, feedbackParameter] {
        *feedbackParameter = mFeedbackSlider.getValue();
    };
    mFeedbackSlider.onDragEnd = [feedbackParameter] {
        feedbackParameter->endChangeGesture();
    };

    // Delay time slider
    juce::AudioParameterFloat *delayTimeParameter =
        (juce::AudioParameterFloat *)params.getUnchecked(2);

    mDelayTimeSlider.setLookAndFeel(&mDelayTimeKnobLookAndFeel);
    mDelayTimeSlider.setBounds(200, 0, 100,
                               100); // 100px to the right of feedback
    mDelayTimeSlider.setSliderStyle(
        juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDelayTimeSlider.setTextBoxStyle(
        juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDelayTimeSlider.setRange(delayTimeParameter->range.start,
                              delayTimeParameter->range.end);
    mDelayTimeSlider.setValue(*delayTimeParameter);
    addAndMakeVisible(mDelayTimeSlider);

    mDelayTimeSlider.onDragStart = [delayTimeParameter] {
        delayTimeParameter->beginChangeGesture();
    };
    mDelayTimeSlider.onValueChange = [this, delayTimeParameter] {
        *delayTimeParameter = mDelayTimeSlider.getValue();
    };
    mDelayTimeSlider.onDragEnd = [delayTimeParameter] {
        delayTimeParameter->endChangeGesture();
    };

    // Labels
    mDryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    mDryWetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mDryWetLabel);

    mFeedbackLabel.setText("Feedback", juce::dontSendNotification);
    mFeedbackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mFeedbackLabel);

    mDelayTimeLabel.setText("Delay Time", juce::dontSendNotification);
    mDelayTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mDelayTimeLabel);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);
}

DelayPluginAudioProcessorEditor::~DelayPluginAudioProcessorEditor() {
    mDryWetSlider.setLookAndFeel(nullptr);
    mFeedbackSlider.setLookAndFeel(nullptr);
    mDelayTimeSlider.setLookAndFeel(nullptr);
}

//==============================================================================
void DelayPluginAudioProcessorEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with
    // a solid colour)
    g.fillAll(juce::Colour(0xffffedd6));

}

void DelayPluginAudioProcessorEditor::resized() {
    const int knobSize = 100;
    const int labelHeight = 20;
    const int numKnobs = 3;
    const int totalWidth = knobSize * numKnobs;
    const int totalHeight = knobSize + labelHeight;
    const int xOffset = (getWidth() - totalWidth) / 2;
    const int yOffset = (getHeight() - totalHeight) / 2;

    mDryWetSlider.setBounds(xOffset, yOffset, knobSize, knobSize);
    mFeedbackSlider.setBounds(xOffset + knobSize, yOffset, knobSize, knobSize);
    mDelayTimeSlider.setBounds(xOffset + 2 * knobSize, yOffset, knobSize, knobSize);

    mDryWetLabel.setBounds(xOffset, yOffset + knobSize, knobSize, labelHeight);
    mFeedbackLabel.setBounds(xOffset + knobSize, yOffset + knobSize, knobSize, labelHeight);
    mDelayTimeLabel.setBounds(xOffset + 2 * knobSize, yOffset + knobSize, knobSize, labelHeight);
}
