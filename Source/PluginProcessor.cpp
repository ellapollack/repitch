#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

void RingBuffer::pushSample(int channel, float sample)
{
    writeIndex++;
    writeIndex %= getNumSamples();
    setSample(channel, writeIndex, sample);
}

float RingBuffer::getDelayedSample(int channel, float delay)
{
    int index = fmod(writeIndex - delay, getNumSamples());
    if (index<0)
        index += getNumSamples();
    
    // Cubic interpolation
    
    float mu = index-trunc(index);
    float y0 = getSample(channel, int(index-1+getNumSamples()) % getNumSamples());
    float y1 = getSample(channel, index);
    float y2 = getSample(channel, int(index+1) % getNumSamples());
    float y3 = getSample(channel, int(index+2) % getNumSamples());
    
    return pow(mu,3) * (y3 - y2 - y0 + y1) -
           pow(mu,2) * (y2+y3) +
           mu * (y2-y0) +
           y1;
}

//==============================================================================
RepitchAudioProcessor::RepitchAudioProcessor() :
AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::mono(), true).withOutput  ("Output",  AudioChannelSet::stereo(), true)),
parameters (*this, nullptr, Identifier ("Repitch"), {std::make_unique<AudioParameterFloat> ("pitch", "Pitch", -128.f, 127.f, 60.f)})
{
    for (int note=0; note<128; ++note)
    {
        voices[note].stride = 1-pow(2, (note-60) / 12.);
    }
    
    pitchParam = parameters.getRawParameterValue("pitch");
}

RepitchAudioProcessor::~RepitchAudioProcessor()
{
}

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
    return 1;
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

void RepitchAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    periodSmoother.reset(sampleRate, samplesPerBlock/sampleRate);
    ring.reset(new RingBuffer(getTotalNumInputChannels(), 64*getSampleRate()));
}
    
void RepitchAudioProcessor::releaseResources()
{
}

bool RepitchAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}

void RepitchAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    int totalNumInputChannels = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
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
        
        for (int channel=0; channel<totalNumInputChannels; ++channel)
        {
            ring->pushSample(channel, buffer.getSample(channel, sample));
            buffer.clear(channel, sample, 1);
        }
            
        for (Voice& voice : voices)
        {
            if (voice.gain > 0.001 || voice.gainTarget > 0)
            {
                voice.gain += 512/getSampleRate() * (voice.gainTarget - voice.gain);
                
                if (totalNumInputChannels==1)
                {
                    for (int channel=0; channel<buffer.getNumChannels(); ++channel)
                    {
                        buffer.addSample(channel, sample, voice.gain / (1-voice.stride) *
                                         (cos(M_PI * voice.delay / period) / -2 + 0.5) *
                                         ring->getDelayedSample(0, voice.delay));
                        buffer.addSample(channel, sample, voice.gain / (1-voice.stride) *
                                         (cos(M_PI * voice.delay / period) / 2 + 0.5) *
                                         ring->getDelayedSample(0, voice.delay +
                                                                period));
                    }
                }
                else
                {
                    for (int channel=0; channel<totalNumInputChannels; ++channel)
                    {
                        buffer.addSample(channel, sample, voice.gain / (1-voice.stride) *
                                         (cos(M_PI * voice.delay / period) / -2 + 0.5) *
                                         ring->getDelayedSample(channel, voice.delay));
                        buffer.addSample(channel, sample, voice.gain / (1-voice.stride) *
                                         (cos(M_PI * voice.delay / period) / 2 + 0.5) *
                                         ring->getDelayedSample(channel, voice.delay +
                                                                period));
                    }
                }
                
                voice.delay = fmod(voice.delay + voice.stride, period);
                if (voice.delay<0)
                    voice.delay += period;
            }
        }
    }
}

bool RepitchAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* RepitchAudioProcessor::createEditor()
{
    return new RepitchAudioProcessorEditor (*this, parameters);
}

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

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RepitchAudioProcessor();
}
