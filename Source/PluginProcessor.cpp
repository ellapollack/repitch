/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

int RingBuffer::wrap(int index)
{
    index %= getNumSamples();
    if (index<0)
        index += getNumSamples();
    return index;
}

void RingBuffer::pushFrom(AudioBuffer<float>& buffer, int sample, float gain)
{
    writeSample++;
    writeSample %= getNumSamples();
    for (int channel=0; channel<buffer.getNumChannels(); ++channel)
    {
        setSample(channel, writeSample, gain * buffer.getSample(channel, sample));
    }
}

void RingBuffer::addTo(AudioBuffer<float> &buffer, int sample, float delay, float gain)
{
    int index = wrap(writeSample-delay);
    for (int channel=0; channel<buffer.getNumChannels(); ++channel)
    {
        // Cubic interpolation
        float y0 = getSample(channel, wrap(index-1));
        float y1 = getSample(channel, index);
        float y2 = getSample(channel, wrap(index+1));
        float y3 = getSample(channel, wrap(index+2));
        float mu = index-trunc(index);
        
        float a0 = y3 - y2 - y0 + y1;
        float a1 = y0 - y1 - a0;
        float a2 = y2 - y0;
        float a3 = y1;
        
        float output = a0*mu*mu*mu+a1*mu*mu+a2*mu+a3;

        buffer.setSample(channel, sample, tanh(atanh(buffer.getSample(channel, sample)) + gain * output));
    }
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
ring(32,pow(2,21)),
parameters (*this, nullptr, Identifier ("Repitch"),
{
    std::make_unique<AudioParameterFloat> ("pitch",            // parameter ID
                                           "Pitch",            // parameter name
                                           0.f,              // minimum value
                                           127.f,              // maximum value
                                           36.f),             // default value
})
{
    for (int note=0; note<128; ++note)
    {
        voices[note].stride = 1-pow(2, (note-60) / 12.);
    }
    
    pitchParam = parameters.getRawParameterValue("pitch");

    ring.clear();
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
    periodSmoother.reset(sampleRate, samplesPerBlock/sampleRate);
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
    AudioPlayHead* ph = getPlayHead();
    AudioPlayHead::CurrentPositionInfo cpi;
    cpi.resetToDefault();
    if (ph != nullptr)
        ph->getCurrentPosition(cpi);
    bpm = cpi.bpm;
    
    MidiBuffer::Iterator midi (midiMessages);
    MidiMessage m;
    int mSample = 0;
    
    periodSmoother.setTargetValue(getSampleRate()/440*pow(2,(69-*pitchParam)/12-1));

    for (int sample=0; sample<buffer.getNumSamples(); ++sample)
    {
        float period = periodSmoother.getNextValue();
        
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
        
        ring.pushFrom(buffer, sample, 1);
        buffer.clear(sample, 1);
        
        for (Voice& voice : voices)
        {
            if (voice.gain > 0.001 || voice.gainTarget > 0)
            {
                voice.gain += 512/getSampleRate() * (voice.gainTarget - voice.gain);
                
                ring.addTo(buffer, sample, voice.delay, voice.gain / (1-voice.stride) * (cos(M_PI * voice.delay / period) / -2 + 0.5));
                ring.addTo(buffer, sample, voice.delay + period, voice.gain / (1-voice.stride) * (cos(M_PI * voice.delay / period) / 2 + 0.5));
                    
                voice.delay += voice.stride;
                voice.delay = fmod(voice.delay, period);
                if (voice.delay<0)
                    voice.delay += period;
            }
        }
    }
}

//==============================================================================
bool RepitchAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* RepitchAudioProcessor::createEditor()
{
    return new RepitchAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void RepitchAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RepitchAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
 
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RepitchAudioProcessor();
}
