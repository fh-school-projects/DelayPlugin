/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayPluginAudioProcessor::DelayPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;

    mDelayTimeInSamples = 0;
    mCircularBufferReadHead = 0;

    mFeedbackLeft = 0;
    mFeedbackRight = 0;

    mDryWet = 0.5;

    mDelayTimeSmoothed = 0;

    addParameter(mDryWetParameter = new juce::AudioParameterFloat({"drywet", 1}, "Dry Wet", 0, 1.0, 0.5));
    addParameter(mFeedbackParameter = new juce::AudioParameterFloat({"feedback", 1}, "Feedback", 0, 0.98, 0.5));
    addParameter(mDelayTimeParameter = new juce::AudioParameterFloat({"delaytime", 1}, "Delay Time", 0.01, MAX_DELAY_TIME, 0.5));
}

DelayPluginAudioProcessor::~DelayPluginAudioProcessor() {
    if (mCircularBufferLeft != nullptr) {
        delete [] mCircularBufferLeft;
        mCircularBufferLeft = nullptr;
    }
    if (mCircularBufferRight != nullptr) {
        delete [] mCircularBufferRight;
        mCircularBufferRight = nullptr;
    }
}

//==============================================================================
const juce::String DelayPluginAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool DelayPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool DelayPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool DelayPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double DelayPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int DelayPluginAudioProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are
              // 0 programs, so this should be at least 1, even if you're not
              // really implementing programs.
}

int DelayPluginAudioProcessor::getCurrentProgram() { return 0; }

void DelayPluginAudioProcessor::setCurrentProgram(int index) {}

const juce::String DelayPluginAudioProcessor::getProgramName(int index) {
    return {};
}

void DelayPluginAudioProcessor::changeProgramName(int index,
                                                  const juce::String &newName) {
}

// allocates the stereo delay buffers once prepareToPlay is called, sized to
// hold the maximum delay time worth of samples at the current sample rate
// the () at the end zero-initializes the arrays to avoid garbage on first read
void DelayPluginAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
    mCircularBufferWriteHead = 0;

    mDelayTimeSmoothed = *mDelayTimeParameter;

    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = new float[(int)(sampleRate * MAX_DELAY_TIME)]();
    }
    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float[(int)(sampleRate * MAX_DELAY_TIME)]();
    }
}

void DelayPluginAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void DelayPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto *channelData = buffer.getWritePointer(channel);
    }

    auto *leftChannel = buffer.getWritePointer(0);
    auto *rightChannel = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); i++) {
        // lowpass smoothing (I learned about this at work!) on the delay time parameter so sudden knob
        // moves don't cause a discontinuous jump in the read head, which would
        // produce a click or pitch glitch
        mDelayTimeSmoothed = mDelayTimeSmoothed - 0.001 * (mDelayTimeSmoothed - *mDelayTimeParameter);
        mDelayTimeInSamples = getSampleRate() * mDelayTimeSmoothed;

        // writes the dry input plus last iteration's feedback sample into the circular
        // buffer — this is what causes the delay to repeat and decay over time
        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;

        mCircularBufferReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;
        if (mCircularBufferReadHead < 0)
            mCircularBufferReadHead += mCircularBufferLength;

        int readHead_x = (int)mCircularBufferReadHead;
        int readHead_x1 = readHead_x + 1;
        float readHeadFloat = mCircularBufferReadHead - readHead_x;

        if (readHead_x1 >= mCircularBufferLength) {
            readHead_x1 -= mCircularBufferLength;
        }

        // the read head lands mDelayTimeInSamples behind the write head, wrapping
        // around the buffer if negative — lin_interp blends between the two nearest
        // samples so fractional delay times don't produce stepping artifacts
        float delay_sample_left = lin_interp(mCircularBufferLeft[readHead_x], mCircularBufferLeft[readHead_x1], readHeadFloat);
        float delay_sample_right = lin_interp(mCircularBufferRight[readHead_x], mCircularBufferRight[readHead_x1], readHeadFloat);

        mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
        mFeedbackRight = delay_sample_right * *mFeedbackParameter;
        // slightly strange.. at mDryWet=1 you get fully dry, at 0 fully wet
        // worth double-checking this matches the UI.
        buffer.setSample(0, i, buffer.getSample(0, i) * *mDryWetParameter + delay_sample_left * (1 - *mDryWetParameter));
        buffer.setSample(1, i, buffer.getSample(1, i) * *mDryWetParameter + delay_sample_right * (1 - *mDryWetParameter));

        mCircularBufferWriteHead++;


        if (mCircularBufferWriteHead >= mCircularBufferLength) {
            mCircularBufferWriteHead = 0;
        }
    }
}

//==============================================================================
bool DelayPluginAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *DelayPluginAudioProcessor::createEditor() {
    return new DelayPluginAudioProcessorEditor(*this);
}

//==============================================================================
void DelayPluginAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DelayPluginAudioProcessor::setStateInformation(const void *data,
                                                    int sizeInBytes) {
    // You should use this method to restore your parameters from this memory
    // block, whose contents will have been created by the getStateInformation()
    // call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new DelayPluginAudioProcessor();
}

// inPhase is the fractional part of the read head. It blends between the two
// neighboring samples proportionally to that fraction
float DelayPluginAudioProcessor::lin_interp(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase * sample_x1;
}
