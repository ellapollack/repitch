/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

class RepitchAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    RepitchAudioProcessorEditor (RepitchAudioProcessor&, AudioProcessorValueTreeState&);
    ~RepitchAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RepitchAudioProcessor& processor;
    AudioProcessorValueTreeState& vts;
    Slider freqSlider, fadeSlider, fdbkSlider, gainSlider;
    std::unique_ptr<SliderAttachment> pitchAttachment, fadeAttachment, feedbackAttachment, volumeAttachment;
    int splashWidth = 128, splashHeight = 128;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepitchAudioProcessorEditor)
};
