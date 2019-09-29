/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
*/

struct RingBuffer : AudioSampleBuffer
{
    using AudioSampleBuffer::AudioSampleBuffer;
    bool pushFrom(AudioSampleBuffer& buffer, int sample);
    bool addFrom(AudioSampleBuffer& buffer, int sample, float gain);
    bool addTo(AudioSampleBuffer& buffer, int sample, int delay, float gain);
    
private:
    int writeSample = -1;
};

struct Voice
{
    float gain = 0, gainTarget = 0, delay = 0, stride;
};

class RepitchAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    RepitchAudioProcessor();
    ~RepitchAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    SmoothedValue<float> fade = 0.5, freq = 32768, feedback = 0, gain = 0.8;
    std::atomic<int> clip {0};

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepitchAudioProcessor)
    RingBuffer ring;
    Voice voices[128];
};
