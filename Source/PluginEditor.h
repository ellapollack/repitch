#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

//==============================================================================

class TunableSlider : public Slider
{
public:
    TunableSlider(SliderStyle ss, TextEntryBoxPosition tebp, std::atomic<double>& bpm) : Slider(ss, tebp), bpm(bpm) {};
    enum Mode {Hz, Pitch, Tempo};
    void setMode(Mode m) {mode = m; setValue(snapValue(getValue(), DragMode::notDragging));};
    
private:
    Mode mode = Hz;
    std::atomic<double>& bpm;
    double snapValue(double, DragMode) override;
    String getTextFromValue(double) override;
    void paint(Graphics&) override;
    
    static double roundToMultiple(double input, double freq, double multiple);
};

struct LineButton : Button
{
    LineButton(String name, String svgPath, Colour onColour, Colour offColour) : Button(name), path(Drawable::parseSVGPath(svgPath)), onColour(onColour), offColour(offColour) {};
    Path path;
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
    LineButton hzButton, noteButton, tempoButton;
    std::unique_ptr<SliderAttachment> pitchAttachment;
    int scale = 128;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepitchAudioProcessorEditor)
};
