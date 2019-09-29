/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

RepitchAudioProcessorEditor::RepitchAudioProcessorEditor (RepitchAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (3*splashWidth, splashWidth);
    
    // these define the parameters of our slider object
    freqSlider.setSliderStyle (Slider::RotaryVerticalDrag);
    freqSlider.setRange(-128, 127, 0);
    freqSlider.setTextBoxStyle (Slider::NoTextBox, false, 90, 0);
    freqSlider.setValue(30);
    
    // this function adds the slider to the editor
    addAndMakeVisible (&freqSlider);
    
    freqSlider.addListener (this);
    
    fadeSlider.setSliderStyle (Slider::RotaryVerticalDrag);
    fadeSlider.setRange(0, 1, 0);
    fadeSlider.setTextBoxStyle (Slider::NoTextBox, false, 90, 0);
    fadeSlider.setValue(0.5);
    
    // this function adds the slider to the editor
    addAndMakeVisible (&fadeSlider);
    
    fadeSlider.addListener (this);
    
    fdbkSlider.setSliderStyle (Slider::RotaryVerticalDrag);
    fdbkSlider.setRange(0, 1, 0);
    fdbkSlider.setTextBoxStyle (Slider::NoTextBox, false, 90, 0);
    fdbkSlider.setValue(0);
    
    // this function adds the slider to the editor
    addAndMakeVisible (&fdbkSlider);
    
    fdbkSlider.addListener (this);
    
    gainSlider.setSliderStyle (Slider::RotaryVerticalDrag);
    gainSlider.setRange(0, 1, 0);
    gainSlider.setTextBoxStyle (Slider::NoTextBox, false, 90, 0);
    gainSlider.setValue(0.8);
    
    // this function adds the slider to the editor
    addAndMakeVisible (&gainSlider);
    
    gainSlider.addListener (this);
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

void RepitchAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    if (slider==&freqSlider)
        processor.freq.setTargetValue(processor.getSampleRate()/440*pow(2,(69-slider->getValue())/12-1));
    else if (slider==&fadeSlider)
        processor.fade.setTargetValue(slider->getValue());
    else if (slider==&fdbkSlider)
        processor.feedback.setTargetValue(slider->getValue());
    else if (slider==&gainSlider)
        processor.gain.setTargetValue(slider->getValue());
}
