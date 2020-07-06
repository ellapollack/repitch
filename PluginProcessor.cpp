#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

// For when microtonal scale support is implemented...

//double roundToFreqMultiple(double value, double freq, double multiple)
//{
//    return 69 + 6 * multiple * (round((value-69) / (6 * multiple) - log(freq/440)/log(multiple)) + log(freq/440)/log(multiple));
//}

RepitchAudioProcessor::RepitchAudioProcessor() :
AudioProcessor (BusesProperties()
                .withInput("Input", AudioChannelSet::mono(), true)
                .withOutput("Output", AudioChannelSet::stereo(), true)),
parameters (*this, nullptr, Identifier ("Repitch"), {
    std::make_unique<AudioParameterFloat> ("pitch",
                                           "Pitch",
                                           NormalisableRange<float> (0., 127., nullptr,
                                               nullptr,
                                               [this](float start, float end, float value) { if (snapParam != nullptr && *snapParam) return round(value); else return value; }
                                           ),
                                           60.),
    std::make_unique<AudioParameterFloat> ("attack", "Attack", NormalisableRange<float> (0.01, 10., pow10VRF, log10VRF), 0.01),
    std::make_unique<AudioParameterFloat> ("decay", "Decay", NormalisableRange<float> (0.01, 10., pow10VRF, log10VRF), 10.),
    std::make_unique<AudioParameterFloat> ("sustain", "Sustain", NormalisableRange<float> (0, 1), 1.),
    std::make_unique<AudioParameterFloat> ("release", "Release", NormalisableRange<float> (0.01, 10., pow10VRF, log10VRF), 0.01),
    std::make_unique<AudioParameterBool>("snap", "Snap", true)
})

{
    pitchParam = parameters.getRawParameterValue("pitch");
    aParam = parameters.getRawParameterValue("attack");
    dParam = parameters.getRawParameterValue("decay");
    sParam = parameters.getRawParameterValue("sustain");
    rParam = parameters.getRawParameterValue("release");
    snapParam = parameters.getRawParameterValue("snap");
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

void RepitchAudioProcessor::processBlock (AudioBuffer<float>& audio, MidiBuffer& midi)
{
    ScopedNoDenormals noDenormals;
    int totalNumInputChannels = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        audio.clear(i, 0, audio.getNumSamples());
    
    MidiBuffer::Iterator midiIter (midi);
    MidiMessage m;
    int mSample = 0;

    pitchSmoother.setTargetValue(*pitchParam);
    aSmoother.setTargetValue(*aParam);
    dSmoother.setTargetValue(*dParam);
    sSmoother.setTargetValue(*sParam);
    rSmoother.setTargetValue(*rParam);

    for (int sample=0; sample<audio.getNumSamples(); ++sample)
    {
        adsrParameters.attack = aSmoother.getNextValue();
        adsrParameters.decay = dSmoother.getNextValue();
        adsrParameters.sustain = sSmoother.getNextValue();
        adsrParameters.release = rSmoother.getNextValue();
        float pitch = pitchSmoother.getNextValue();
        
        float period = getSampleRate() / 440 * pow(2, (69 - pitch) / 12);

        // for (const MidiMessageMetadata m : midi)
        while (sample==mSample && midiIter.getNextEvent(m, mSample))
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
            ring->pushSample(channel, audio.getSample(channel, sample));
            audio.clear(channel, sample, 1);
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
                    
                    audio.addSample(channel, sample, voice.gain * envelope *
                                     ((cos(3.14159265359 * voice.delay / period) / -2 + 0.5) *
                                     ring->getSampleAtDelay(sourceChannel,
                                                            voice.delay) +
                                     (cos(3.14159265359 * voice.delay / period) / 2 + 0.5) *
                                    ring->getSampleAtDelay(sourceChannel,
                                                           voice.delay + period)));
                }

                voice.delay = fmod(voice.delay + voice.stride, period);
                if (voice.delay<0)
                    voice.delay += period;
            }
        }
        ring->increment();
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
