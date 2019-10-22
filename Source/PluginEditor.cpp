#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

double TunableSlider::roundToMultiple(double value, double freq, double multiple)
{
    return 69 + 6 * multiple * (round((value-69) / (6 * multiple) - log(freq/440)/log(multiple)) + log(freq/440)/log(multiple));
}

double TunableSlider::snapValue(double input, DragMode dm)
{
    if (snap) return round(input);
    else return input;
}

void TunableSlider::paint(Graphics& g)
{
    Slider::paint(g);
    g.setColour(findColour(Slider::thumbColourId).brighter());
    g.setFont(30.f);
    
    double value = getValue();
    
    if (snap)
    {
        const String noteNames[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        g.drawText(noteNames[int(value+1200)%12] + String(floor(value/12)-1), 16, 0, 96, getHeight(), Justification::centred, false);
    }
    else
    {
        g.drawText(String(440 * pow(2, (value - 69) / 12)), 16, 0, 96, getHeight(), Justification::centred, false);
    }
}

//==============================================================================

void LineButton::paintButton(Graphics& g, bool highlighted, bool down)
{
    g.setColour(getToggleState() || down ? onColour : offColour);
    g.strokePath(icon, PathStrokeType(1, PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded));
}

//==============================================================================

RepitchAudioProcessorEditor::RepitchAudioProcessorEditor (RepitchAudioProcessor& p, AudioProcessorValueTreeState& vts) :
    AudioProcessorEditor (&p),
    processor (p),
    vts(vts),
    pitchSlider(Slider::RotaryVerticalDrag, Slider::NoTextBox),
    aSlider(Slider::LinearVertical, Slider::NoTextBox),
    dSlider(Slider::LinearVertical, Slider::NoTextBox),
    sSlider(Slider::LinearVertical, Slider::NoTextBox),
    rSlider(Slider::LinearVertical, Slider::NoTextBox),
    hzButton("Hz", "M 1 4 L 1 16 M 9 4 L 9 16 M 2 10 L 9 10 M 18 10 L 12 16 M 12 10 L 18 10 M 12 16 L 18 16", Colours::cyan, Colours::darkgrey),
    noteButton("Musical Note", "M 13 16 A 3 3 0 1 0 7 16 A 3 3 0 1 0 13 16 L 13 4 A 5 5 0 0 0 18 9", Colours::red, Colours::darkgrey)
{
    setSize (scale, 5*scale/3);
    
    addAndMakeVisible (&pitchSlider);
    pitchAttachment.reset(new SliderAttachment (vts, "pitch", pitchSlider));
    
    addAndMakeVisible(&hzButton);
    hzButton.setRadioGroupId(7);
    hzButton.setClickingTogglesState(true);
    hzButton.onClick = [this](){
        pitchSlider.setColour(Slider::thumbColourId, hzButton.onColour);
        pitchSlider.setSnap(false);
    };
    
    addAndMakeVisible(&noteButton);
    noteButton.setRadioGroupId(7);
    noteButton.setClickingTogglesState(true);
    noteButton.onClick = [this](){
        pitchSlider.setColour(Slider::thumbColourId, noteButton.onColour);
        pitchSlider.setSnap(true);
    };
    noteButton.triggerClick();
    
    addAndMakeVisible(&aSlider);
    aSlider.setColour(Slider::thumbColourId, Colours::grey);
    aAttachment.reset(new SliderAttachment(vts, "attack", aSlider));
    
    addAndMakeVisible(&dSlider);
    dSlider.setColour(Slider::thumbColourId, Colours::grey);
    dAttachment.reset(new SliderAttachment(vts, "decay", dSlider));
    
    addAndMakeVisible(&sSlider);
    sSlider.setColour(Slider::thumbColourId, Colours::grey);
    sAttachment.reset(new SliderAttachment(vts, "sustain", sSlider));
    
    addAndMakeVisible(&rSlider);
    rSlider.setColour(Slider::thumbColourId, Colours::grey);
    rAttachment.reset(new SliderAttachment(vts, "release", rSlider));
}

RepitchAudioProcessorEditor::~RepitchAudioProcessorEditor()
{
}

void RepitchAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::black);
    g.setColour(Colours::lightgrey);
    Path aPath = Drawable::parseSVGPath("M9,1 L1,22 M9,1 L17,22 M4,15 L14,15");
    aPath.applyTransform(AffineTransform::scale(0.5,0.5).translated(getWidth()/8-4,getHeight()-22));
    Path dPath = Drawable::parseSVGPath("M4,1 L4,22 M4,1 L11,1 14,2 16,4 17,6 18,9 18,14 17,17 16,19 14,21 11,22 4,22");
    dPath.applyTransform(AffineTransform::scale(0.5,0.5).translated(3*getWidth()/8-4.5,getHeight()-22));
    Path sPath = Drawable::parseSVGPath("M17,4 L15,2 12,1 8,1 5,2 3,4 3,6 4,8 5,9 7,10 13,12 15,13 16,14 17,16 17,19 15,21 12,22 8,22 5,21 3,19");
    sPath.applyTransform(AffineTransform::scale(0.5,0.5).translated(5*getWidth()/8-4.5,getHeight()-22));
    Path rPath = Drawable::parseSVGPath("M4,1 L4,22 M4,1 L13,1 16,2 17,3 18,5 18,7 17,9 16,10 13,11 4,11 M11,11 L18,22");
    rPath.applyTransform(AffineTransform::scale(0.5,0.5).translated(7*getWidth()/8-4.5,getHeight()-22));
    
    g.strokePath(aPath, PathStrokeType(1, PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded));
    g.strokePath(dPath, PathStrokeType(1, PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded));
    g.strokePath(sPath, PathStrokeType(1, PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded));
    g.strokePath(rPath, PathStrokeType(1, PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded));
}

void RepitchAudioProcessorEditor::resized()
{
    pitchSlider.setBounds (0, 0, scale, scale);
    hzButton.setBounds(44,96,20,30);
    noteButton.setBounds(64,96,20,30);
    
    aSlider.setBounds(0, getWidth()*0.97, getWidth()/4, getWidth()/2);
    dSlider.setBounds(getWidth()/4, getWidth()*0.97, getWidth()/4, getWidth()/2);
    sSlider.setBounds(getWidth()/2, getWidth()*0.97, getWidth()/4, getWidth()/2);
    rSlider.setBounds(3*getWidth()/4, getWidth()*0.97, getWidth()/4, getWidth()/2);
}
