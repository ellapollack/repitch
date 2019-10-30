#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================

class TunableSlider : public Slider
{
public:
    
    TunableSlider(SliderStyle ss, TextEntryBoxPosition tebp) : Slider(ss, tebp), tuneToggle()
    { addAndMakeVisible(tuneToggle); }
    
    class TuneToggle : public Button
    {
    public:
        TuneToggle() : Button("tuneToggle") { setTriggeredOnMouseDown(true); }
    private:
        void mouseDown(const MouseEvent&) override;
        void paintButton(Graphics&, bool, bool) override;
    };
    
    TuneToggle tuneToggle;
    
private:
    void paint(Graphics&) override;
    void resized() override;
};

class RepitchAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    RepitchAudioProcessorEditor (RepitchAudioProcessor&, AudioProcessorValueTreeState&);
    ~RepitchAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

private:
    RepitchAudioProcessor& processor;
    AudioProcessorValueTreeState& vts;
    
    TunableSlider pitchSlider;
    Slider aSlider, dSlider, sSlider, rSlider;
    SliderAttachment pitchAttachment, aAttachment, dAttachment, sAttachment, rAttachment;
    ButtonAttachment snapAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepitchAudioProcessorEditor)
};
