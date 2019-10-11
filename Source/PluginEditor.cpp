#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

double TunableSlider::roundToMultiple(double value, double freq, double multiple)
{
    return 69 + 6 * multiple * (round((value-69) / (6 * multiple) - log(freq/440)/log(multiple)) + log(freq/440)/log(multiple));
}

double TunableSlider::snapValue(double input, DragMode dm)
{
    switch (mode)
    {
        case Hz:
            return input;
        case Pitch:
            return round(input);
        case Tempo:
            double bps = bpm / 60;
            if (bps>0)
                return getRange().clipValue(roundToMultiple(input, bps, 2));
            else
                return input;
    }
}

String TunableSlider::getTextFromValue(double value)
{
    switch (mode)
    {
        case Hz:
            return String(440 * pow(2, (value - 69) / 12));
        case Pitch:
        {
            String noteNames[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
            return noteNames[int(value+1200)%12] + String(floor(value/12)-1);
        }
        case Tempo:
        {
            double bps = bpm / 60;
            if (bps>0)
            {
                double tempoValue = pow(2,(value-69) / 12 - log2(bps/440));
                if (tempoValue>1)
                    return "1/" + String(round(tempoValue));
                else
                    return String(round(1/tempoValue));
            }
            else return "N/A";
        }
    }
}

void TunableSlider::paint(Graphics& g)
{
    Slider::paint(g);
    g.setColour(findColour(Slider::thumbColourId).brighter());
    g.setFont(25.f);
    g.drawText(getTextFromValue(getValue()), 28, 36, 72, 36, Justification::centred, false);
}

//==============================================================================

void LineButton::paintButton(Graphics& g, bool highlighted, bool down)
{
    g.setColour(getToggleState() || down ? onColour : offColour);
    g.strokePath(path, PathStrokeType(1, PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded));
}

//==============================================================================

RepitchAudioProcessorEditor::RepitchAudioProcessorEditor (RepitchAudioProcessor& p, AudioProcessorValueTreeState& vts) :
    AudioProcessorEditor (&p),
    processor (p),
    vts(vts),
    pitchSlider(Slider::RotaryVerticalDrag, Slider::NoTextBox, p.bpm),
    hzButton("Hz", "M 1 4 L 1 16 M 9 4 L 9 16 M 2 10 L 9 10 M 18 10 L 12 16 M 12 10 L 18 10 M 12 16 L 18 16", Colours::cyan, Colours::grey),
    noteButton("Musical Note", "M 13 16 A 3 3 0 1 0 7 16 A 3 3 0 1 0 13 16 L 13 4 A 5 5 0 0 0 18 9", Colours::red, Colours::grey),
    tempoButton("Metronome", "M 5 19 L 9 4 L 11 4 L 15 19 L 5 19 M 6.33333 14 L 13.66666 14 M 10 14 L 15 6", Colours::lime, Colours::grey)
{
    setSize (scale, scale);
    
    addAndMakeVisible (&pitchSlider);
    pitchAttachment.reset(new SliderAttachment (vts, "pitch", pitchSlider));
    
    addAndMakeVisible(&hzButton);
    hzButton.setRadioGroupId(7);
    hzButton.setClickingTogglesState(true);
    hzButton.onClick = [this](){
        pitchSlider.setColour(Slider::thumbColourId, hzButton.onColour);
        pitchSlider.setMode(TunableSlider::Hz);
    };
    
    addAndMakeVisible(&noteButton);
    noteButton.setRadioGroupId(7);
    noteButton.setClickingTogglesState(true);
    noteButton.onClick = [this](){
        pitchSlider.setColour(Slider::thumbColourId, noteButton.onColour);
        pitchSlider.setMode(TunableSlider::Pitch);
    };
    
    addAndMakeVisible(&tempoButton);
    tempoButton.setRadioGroupId(7);
    tempoButton.setClickingTogglesState(true);
    tempoButton.onClick = [this](){
        pitchSlider.setColour(Slider::thumbColourId, tempoButton.onColour);
        pitchSlider.setMode(TunableSlider::Tempo);
    };
    
    hzButton.triggerClick();
}

RepitchAudioProcessorEditor::~RepitchAudioProcessorEditor()
{
}

void RepitchAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::black);

    g.setColour(Colours::grey);

    g.setFont(15);
    
    g.drawText("Pitch", 0, 0, scale, scale*0.91, Justification::centredBottom);
}

void RepitchAudioProcessorEditor::resized()
{
    pitchSlider.setBounds (0, 0, scale, scale);
    hzButton.setBounds(34,72,20,30);
    noteButton.setBounds(54,72, 20, 30);
    tempoButton.setBounds(74,72,20,30);
}
