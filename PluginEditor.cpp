#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

void TunableSlider::paint(Graphics& g)
{
    Colour modeColour = tuneToggle.getToggleState() ? Colours::red : Colours::cyan;
    setColour(Slider::thumbColourId, modeColour);
    Slider::paint(g);
    
    g.setColour(modeColour);
    
    double value = getValue();
    
    if (tuneToggle.getToggleState())
    {
        g.setFont(value < 12 ? 0.25*getWidth() : 0.3*getWidth());
        g.drawText(MidiMessage::getMidiNoteName(int(value), true, true, 4), 0.2*getWidth(), 0, 0.6*getWidth(), getHeight(), Justification::centred, false);
    }
    else
    {
        double hzValue = 440 * pow(2, (value - 69) / 12);
        g.setFont(0.9*getWidth()/jmax(3.,ceil(log10(hzValue))-0.5));
        if (hzValue>100)
            hzValue = trunc(hzValue);
        g.drawText(String(hzValue), 0.2*getWidth(), 0, 0.6*getWidth(), getHeight(), Justification::centred, false);
    }
}

void TunableSlider::resized()
{
    Slider::resized();
    tuneToggle.setBounds(getWidth()/4, getHeight()*0.75, getWidth()/2, 30);
}

void TunableSlider::TuneToggle::mouseDown(const MouseEvent& a)
{
    setToggleState(a.getMouseDownX()>getWidth()/2, sendNotificationSync);
    TunableSlider* s = dynamic_cast<TunableSlider*>(getParentComponent());
    s->setValue(s->getValue());
    s->repaint();
}

void TunableSlider::TuneToggle::paintButton(Graphics& g, bool highlighted, bool down)
{
    bool toggleState = getToggleState();
    
    PathStrokeType pst (1,
                        PathStrokeType::JointStyle::curved,
                        PathStrokeType::EndCapStyle::rounded);
    
    Path hzIcon = Drawable::parseSVGPath("M 1 4 L 1 16 M 9 4 L 9 16 M 2 10 L 9 10 M 18 10 L 12 16 M 12 10 L 18 10 M 12 16 L 18 16");
    hzIcon.applyTransform(AffineTransform::translation(getWidth()/2-20, 0));
    g.setColour(toggleState ? Colours::darkgrey : Colours::cyan);
    g.strokePath(hzIcon, pst);
    
    Path noteIcon = Drawable::parseSVGPath("M 13 16 A 3 3 0 1 0 7 16 A 3 3 0 1 0 13 16 L 13 4 A 5 5 0 0 0 18 9");
    noteIcon.applyTransform(AffineTransform::translation(getWidth()/2, 0));
    g.setColour(toggleState ? Colours::red : Colours::darkgrey);
    g.strokePath(noteIcon, pst);
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
    pitchAttachment(vts, "pitch", pitchSlider),
    aAttachment(vts, "attack", aSlider),
    dAttachment(vts, "decay", dSlider),
    sAttachment(vts, "sustain", sSlider),
    rAttachment(vts, "release", rSlider),
    snapAttachment(vts, "snap", pitchSlider.tuneToggle)
{
    setSize (128, 213);
    
    addAndMakeVisible (pitchSlider);
    pitchSlider.setMouseDragSensitivity(512);
    
    addAndMakeVisible(aSlider);
    aSlider.setColour(Slider::thumbColourId, Colours::grey);
    
    addAndMakeVisible(dSlider);
    dSlider.setColour(Slider::thumbColourId, Colours::grey);
    
    addAndMakeVisible(sSlider);
    sSlider.setColour(Slider::thumbColourId, Colours::grey);
    
    addAndMakeVisible(rSlider);
    rSlider.setColour(Slider::thumbColourId, Colours::grey);
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
    pitchSlider.setBounds (0, 0, getWidth(), getWidth());
    aSlider.setBounds(0, getWidth()*0.97, getWidth()/4, getWidth()/2);
    dSlider.setBounds(getWidth()/4, getWidth()*0.97, getWidth()/4, getWidth()/2);
    sSlider.setBounds(getWidth()/2, getWidth()*0.97, getWidth()/4, getWidth()/2);
    rSlider.setBounds(3*getWidth()/4, getWidth()*0.97, getWidth()/4, getWidth()/2);
}
