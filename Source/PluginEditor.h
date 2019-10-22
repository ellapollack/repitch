#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

//==============================================================================

class TunableSlider : public Slider
{
public:
    using Slider::Slider;
    enum Mode {Hz, Pitch};
    void setSnap(bool s) {snap = s; setValue(snapValue(getValue(), DragMode::notDragging));};
    
private:
    bool snap = true;
    double snapValue(double, DragMode) override;
    void paint(Graphics&) override;
    
    static double roundToMultiple(double input, double freq, double multiple);
};

struct LineButton : Button
{
    LineButton(String name, String svgPath, Colour onColour, Colour offColour) : Button(name), icon(Drawable::parseSVGPath(svgPath)), onColour(onColour), offColour(offColour) {};
    Path icon;
    Colour onColour, offColour;
    void paintButton(Graphics&, bool, bool) override;
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
    LineButton hzButton, noteButton;
    std::unique_ptr<SliderAttachment> pitchAttachment, aAttachment, dAttachment, sAttachment, rAttachment;
    int scale = 128;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepitchAudioProcessorEditor)
};
