#pragma once

#include "JuceHeader.h"

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
    float t = fmod(writeIndex - delay, numSamples);
    if (t<0) t += numSamples;

    const float* audio = getReadPointer(channel);

    // sinc interpolation
    // auto-vectorizes with -03 and -ffast-math

    int t_0 = int(t);
    float dt_0 = t - t_0;
    if (dt_0==0.) return audio[t_0];
    float sum = 0.;

    const int n = 16;

    int i0 = t_0-n+n/2;
    if (i0<0) i0 = i0/numSamples-1;
    else i0 /= numSamples;

    int im = (t_0+n/2)/numSamples;

    for (int i=i0; i<=im; ++i) {

      int offset = t_0-i*numSamples;

      int jmax = offset+n/2;
      int jmin = jmax-n;

      if(jmin<0) jmin = 0;
      jmin -= offset;

      if (jmax >= numSamples-1) jmax = numSamples-1;
      jmax -= offset;

      for (int j=jmin; j<=jmax; ++j)
        sum += float(2*(j&1)-1) * audio[offset+j] / (j-dt_0);
    }

    return sum * sinpi[dt_0];
  }

    void increment()
    {
        writeIndex++;
        writeIndex %= getNumSamples();
    }

private:
  int writeIndex = 0;
  std::function<float(float)> sinpiFunc = [] (float x) { return sin(3.14159265359*x)/3.14159265359; };
  dsp::LookupTableTransform<float> sinpi {sinpiFunc, 0, 1, 512};
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

    static float pow10VRF(float start, float end, float value)
    { return pow(10.,jmap(double(value), log10(start), log10(end))); }

    static float log10VRF(float start, float end, float value)
    { return jmap(log10(value), log10(start), log10(end), 0.0f, 1.0f); }

    std::atomic<float> *pitchParam, *aParam, *dParam, *sParam, *rParam, *snapParam=nullptr;
    AudioProcessorValueTreeState parameters;
    ADSR::Parameters adsrParameters;
    SmoothedValue<float, ValueSmoothingTypes::Linear> pitchSmoother, sSmoother;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> aSmoother, dSmoother, rSmoother;
};
