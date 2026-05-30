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

    mDryWetSlider.setBounds(0, 0, 100, 100);
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

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);
}

DelayPluginAudioProcessorEditor::~DelayPluginAudioProcessorEditor() {}

//==============================================================================
void DelayPluginAudioProcessorEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with
    // a solid colour)
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

}

void DelayPluginAudioProcessorEditor::resized() {
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
