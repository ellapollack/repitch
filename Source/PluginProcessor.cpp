/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

bool RingBuffer::pushFrom(AudioBuffer<float>& buffer, int sample)
{
    writeSample++;
    writeSample %= getNumSamples();
    bool clip = false;
    for (int channel=0; channel<buffer.getNumChannels(); ++channel)
    {
        float value = buffer.getSample(channel, sample);
        if (value < -1) {value = -1; clip = true;}
        if (value > 1) {value = 1; clip = true;}
        setSample(channel, writeSample, value);
    }
    return clip;
}

bool RingBuffer::addFrom(AudioBuffer<float>& buffer, int sample, float gain)
{
    bool clip = false;
    for (int channel=0; channel<buffer.getNumChannels(); ++channel)
    {
        addSample(channel, writeSample, gain * buffer.getSample(channel, sample));
        if (getSample(channel, writeSample) > 1)
        {
            setSample(channel, writeSample, 1);
            clip = true;
        }
        if (getSample(channel, writeSample) < -1)
        {
            setSample(channel, writeSample, -1);
            clip = true;
        }
    }
    return clip;
}

bool RingBuffer::addTo(AudioSampleBuffer &buffer, int sample, int delay, float gain)
{
    int index = (writeSample-delay) % getNumSamples();
    bool clip = false;
    if (index<0)
        index += getNumSamples();
    for (int channel=0; channel<buffer.getNumChannels(); ++channel)
    {
        buffer.addSample(channel, sample, gain * getSample(channel, index));
        if (buffer.getSample(channel, sample) > 1)
        {
            buffer.setSample(channel, sample, 1);
            clip = true;
        }
        if (buffer.getSample(channel, sample) < -1)
        {
            buffer.setSample(channel, sample, -1);
            clip = true;
        }
    }
    return clip;
}

//==============================================================================
RepitchAudioProcessor::RepitchAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
     AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
ring(2, 2097152)
{
    ring.clear();
    for (int note=0; note<128; ++note)
    {
        voices[note].stride = 1-pow(2, (note-60) / 12.);
    }
}

RepitchAudioProcessor::~RepitchAudioProcessor()
{
}

//==============================================================================
const String RepitchAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RepitchAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RepitchAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RepitchAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RepitchAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RepitchAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RepitchAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RepitchAudioProcessor::setCurrentProgram (int index)
{
}

const String RepitchAudioProcessor::getProgramName (int index)
{
    return {};
}

void RepitchAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void RepitchAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    fade.reset(sampleRate, samplesPerBlock/sampleRate);
    freq.reset(sampleRate, samplesPerBlock/sampleRate);
    feedback.reset(sampleRate, samplesPerBlock/sampleRate);
    gain.reset(sampleRate, samplesPerBlock/sampleRate);
    
}
    
void RepitchAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool RepitchAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}

void RepitchAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    MidiBuffer::Iterator midi (midiMessages);
    MidiMessage m;
    int mSample = 0;

    for (int sample=0; sample<buffer.getNumSamples(); ++sample)
    {
        while (sample==mSample && midi.getNextEvent(m, mSample))
        {
            if (m.isNoteOnOrOff())
            {
                int note = m.getNoteNumber();
                if (m.isNoteOn())
                {
                    voices[note].delay = 0;
                    voices[note].gainTarget = m.getFloatVelocity();
                }
                else
                    voices[note].gainTarget = 0;
            }
        }
        
        ring.pushFrom(buffer, sample);
        buffer.clear(sample, 1);
        
        int currFreq = freq.getNextValue();
        float currGain = gain.getNextValue();
        
        for (Voice& voice : voices)
        {
            if (voice.gain > 0.001 || voice.gainTarget > 0)
            {
                voice.gain += pow(5*getSampleRate(),-fade.getNextValue()) * (voice.gainTarget - voice.gain);
                
                clip &= ring.addTo(buffer, sample, voice.delay, currGain/(1-voice.stride)*voice.gain*pow(sin(voice.delay / currFreq * M_PI / 2), 2));
                clip &= ring.addTo(buffer, sample, voice.delay + currFreq, currGain/(1-voice.stride)*voice.gain*pow(cos(voice.delay / currFreq * M_PI / 2), 2));
                
                voice.delay += voice.stride;
                voice.delay = fmod(voice.delay, currFreq);
                if (voice.delay<0)
                    voice.delay += currFreq;
            }
        }
        clip &= ring.addFrom(buffer, sample, feedback.getNextValue());
    }
}

//==============================================================================
bool RepitchAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* RepitchAudioProcessor::createEditor()
{
    return new RepitchAudioProcessorEditor (*this);
}

//==============================================================================
void RepitchAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RepitchAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RepitchAudioProcessor();
}
