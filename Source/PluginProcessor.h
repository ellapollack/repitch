#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================

struct RingBuffer : AudioSampleBuffer
{
    using AudioSampleBuffer::AudioSampleBuffer;
    void pushSample(int,float);
    float getDelayedSample(int,float);

private:
    int writeIndex = -1;
};

struct Voice
{
    float gain = 0, gainTarget = 0, delay = 0, stride;
};

class RepitchAudioProcessor : public AudioProcessor
{
public:
    RepitchAudioProcessor();
    ~RepitchAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    std::atomic<double> bpm;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepitchAudioProcessor)
    std::unique_ptr<RingBuffer> ring;
    Voice voices[128];
    
    AudioProcessorValueTreeState parameters;
    float *pitchParam;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> periodSmoother;
};
