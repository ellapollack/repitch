#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

const String NOTE_NAMES[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

//==============================================================================

struct Voice
{
    ADSR envelope;
    float gain = 0, delay = 0, stride;
};

struct RingBuffer : AudioSampleBuffer
{
    using AudioSampleBuffer::AudioSampleBuffer;
    
    void pushSample(int channel, float sample)
    {
        setSample(channel, writeIndex, sample);
    }

    float getSampleAtDelay(int channel, float delay)
    {
        int numSamples = getNumSamples();
        float index = fmod(writeIndex - delay + ceil(delay/numSamples)*numSamples, numSamples);
        
        // linear interpolation
        
        return (1+trunc(index)-index) * getSample(channel, int(index)) +
        (index-trunc(index)) * getSample(channel, int(index+1)%getNumSamples());
    }
    
    void increment()
    {
        writeIndex++;
        writeIndex %= getNumSamples();
    }

private:
    int writeIndex = 0;
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

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepitchAudioProcessor)
    std::unique_ptr<RingBuffer> ring;
    Voice voices[128];
    
    float *pitchParam, *aParam, *dParam, *sParam, *rParam, *snapParam=nullptr;
    AudioProcessorValueTreeState parameters;
    ADSR::Parameters adsrParameters;
    SmoothedValue<float> pitchSmoother, aSmoother, dSmoother, sSmoother, rSmoother;
};
