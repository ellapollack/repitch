#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

RepitchAudioProcessor::RepitchAudioProcessor() :
AudioProcessor (BusesProperties()
                .withInput("Input", AudioChannelSet::mono(), true)
                .withOutput("Output", AudioChannelSet::stereo(), true)),
parameters (*this, nullptr, Identifier ("Repitch"), { std::make_unique<AudioParameterFloat> ("pitch", "Pitch", 0.f, 127.f, 48.f),
    std::make_unique<AudioParameterFloat> ("attack", "Attack", -2.f, 1.f, 0.f),
    std::make_unique<AudioParameterFloat> ("decay", "Decay", -2.f, 1.f, 0.f),
    std::make_unique<AudioParameterFloat> ("sustain", "Sustain", 0.f, 1.f, 1.f),
    std::make_unique<AudioParameterFloat> ("release", "Release", -2.f, 1.f, 0.f)
})
{
    pitchParam = parameters.getRawParameterValue("pitch");
    aParam = parameters.getRawParameterValue("attack");
    dParam = parameters.getRawParameterValue("decay");
    sParam = parameters.getRawParameterValue("sustain");
    rParam = parameters.getRawParameterValue("release");
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
    pitchSmoother.reset(sampleRate, samplesPerBlock/sampleRate);
    aSmoother.reset(sampleRate, samplesPerBlock/sampleRate);
    dSmoother.reset(sampleRate, samplesPerBlock/sampleRate);
    sSmoother.reset(sampleRate, samplesPerBlock/sampleRate);
    rSmoother.reset(sampleRate, samplesPerBlock/sampleRate);
    
    ring.reset(new RingBuffer(getTotalNumInputChannels(), 64*sampleRate));
    for (Voice voice : voices)
        voice.envelope.setSampleRate(sampleRate);
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
        buffer.clear(i, 0, buffer.getNumSamples());
    
    MidiBuffer::Iterator midi (midiMessages);
    MidiMessage m;
    int mSample = 0;
    
    pitchSmoother.setTargetValue(*pitchParam);
    aSmoother.setTargetValue(*aParam);
    dSmoother.setTargetValue(*dParam);
    sSmoother.setTargetValue(*sParam);
    rSmoother.setTargetValue(*rParam);

    for (int sample=0; sample<buffer.getNumSamples(); ++sample)
    {
        adsrParameters.attack = pow(10,aSmoother.getNextValue());
        adsrParameters.decay = pow(10,dSmoother.getNextValue());
        adsrParameters.sustain = sSmoother.getNextValue();
        adsrParameters.release = pow(10,rSmoother.getNextValue());
        
        float pitch = pitchSmoother.getNextValue();
        float period = getSampleRate() / 440 * pow(2, (69 - pitch) / 12);
        
        while (sample==mSample && midi.getNextEvent(m, mSample))
        {
            if (m.isNoteOnOrOff())
            {
                int note = m.getNoteNumber();
                if (m.isNoteOn())
                {
                    if (!voices[note].envelope.isActive())
                        voices[note].delay = 0;
                        voices[note].gain = m.getFloatVelocity();
                    
                    voices[note].envelope.noteOn();
                }
                else
                    voices[note].envelope.noteOff();
            }
        }

        for (int channel=0; channel<totalNumInputChannels; ++channel)
        {
            ring->pushSample(channel, buffer.getSample(channel, sample));
            buffer.clear(channel, sample, 1);
        }
            
        for (int note=0; note<128; ++note)
        {
            Voice& voice = voices[note];
            
            if (voice.envelope.isActive())
            {
                voice.stride = 1 - pow(2, (note-pitch) / 12.);
                voice.envelope.setParameters(adsrParameters);
                float envelope = voice.envelope.getNextSample();
                
                for (int channel=0; channel<(totalNumInputChannels==1 ? totalNumOutputChannels : totalNumInputChannels); ++channel)
                {
                    int sourceChannel = totalNumInputChannels==1 ? 0 : channel;
                    
                    buffer.addSample(channel, sample, voice.gain * envelope *
                                     ((cos(M_PI * voice.delay / period) / -2 + 0.5) *
                                     ring->getSampleAtDelay(sourceChannel,
                                                            voice.delay) +
                                     (cos(M_PI * voice.delay / period) / 2 + 0.5) *
                                    ring->getSampleAtDelay(sourceChannel,
                                                           voice.delay + period)));
                }

                voice.delay = fmod(voice.delay + voice.stride, period);
                if (voice.delay<0)
                    voice.delay += period;
            }
        }
        ring->inc();
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
