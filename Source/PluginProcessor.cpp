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

//==============================================================================
void DelayPluginAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
    mCircularBufferWriteHead = 0;

    // Temp hardcode to 0.5 seconds
    mDelayTimeInSamples = sampleRate * 0.5 /* seconds */ ;

    if (mCircularBufferLeft == nullptr) {
        // For both of these, you are making an array of floats with the length
        // of the total samples for the delay
        mCircularBufferLeft = new float[(int)(sampleRate * MAX_DELAY_TIME)]();
    }
    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float[(int)(sampleRate * MAX_DELAY_TIME)]();
    }
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
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
        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i];
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i];

        mCircularBufferReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;
        if (mCircularBufferReadHead < 0)
            mCircularBufferReadHead += mCircularBufferLength;
        
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
