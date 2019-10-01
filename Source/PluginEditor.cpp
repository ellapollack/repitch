/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

RepitchAudioProcessorEditor::RepitchAudioProcessorEditor (RepitchAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), vts(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (3*splashWidth, splashWidth);
    
    freqSlider.setSliderStyle (Slider::RotaryVerticalDrag);
    freqSlider.setTextBoxStyle (Slider::NoTextBox, false, 90, 0);
    addAndMakeVisible (&freqSlider);
    pitchAttachment.reset(new SliderAttachment (vts, "pitch", freqSlider));
    
    fadeSlider.setSliderStyle (Slider::RotaryVerticalDrag);
    fadeSlider.setTextBoxStyle (Slider::NoTextBox, false, 90, 0);
    addAndMakeVisible (&fadeSlider);
    fadeAttachment.reset(new SliderAttachment (vts, "fade", fadeSlider));
    
    fdbkSlider.setSliderStyle (Slider::RotaryVerticalDrag);
    fdbkSlider.setTextBoxStyle (Slider::NoTextBox, false, 90, 0);
    addAndMakeVisible (&fdbkSlider);
    feedbackAttachment.reset(new SliderAttachment (vts, "feedback", fdbkSlider));
    
    gainSlider.setSliderStyle (Slider::RotaryVerticalDrag);
    gainSlider.setTextBoxStyle (Slider::NoTextBox, false, 90, 0);
    addAndMakeVisible (&gainSlider);
    volumeAttachment.reset(new SliderAttachment (vts, "volume", gainSlider));
}

RepitchAudioProcessorEditor::~RepitchAudioProcessorEditor()
{
}

//==============================================================================
void RepitchAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

    g.setColour (Colours::grey);

    g.setFont(15);
    
    g.drawText("Freq", 0, 0, splashWidth, splashWidth*0.93, Justification::centredBottom);
    g.drawText("Fade", splashWidth, 0, 2*splashWidth/3, splashWidth*0.93, Justification::centredBottom);
    g.drawText("Feedback", 5*splashWidth/3, 0, 2*splashWidth/3, splashWidth*0.93, Justification::centredBottom);
    g.drawText("Volume", 7*splashWidth/3, 0, 2*splashWidth/3, splashWidth*0.93, Justification::centredBottom);
}

void RepitchAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    freqSlider.setBounds (0, 0, splashWidth, splashWidth);
    fadeSlider.setBounds (splashWidth, 0, 2*splashWidth/3, splashWidth);
    fdbkSlider.setBounds (5*splashWidth/3, 0, 2*splashWidth/3, splashWidth);
    gainSlider.setBounds (7*splashWidth/3, 0, 2*splashWidth/3, splashWidth);
    
}
