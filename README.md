# Repitch

![](screenshot.png)

is a real-time MIDI-polyphonic pitch-shifting audio plugin. It uses a [Variable Delay Line](http://msp.ucsd.edu/techniques/latest/book-html/node115.html) and was made with [JUCE](http://www.juce.com).

---

## To use:

> - Download Repitch as a [standalone application](https://github.com/maxwellpollack/repitch/releases/latest/download/Repitch.app.zip) (Mac only)

### or

> - Download Repitch as an audio plugin in [Audio Unit](https://github.com/maxwellpollack/repitch/releases/latest/download/repitch.component.zip) (Mac) or [VST](https://github.com/maxwellpollack/repitch/releases/latest/download/repitch.vst3.zip) (Windows) format
> - Install it in your [DAW](https://en.wikipedia.org/wiki/Digital_audio_workstation)
> - Place it on a MIDI instrument track with sidechain audio input

Then, play some MIDI notes. *The audio input will be polyphonically pitch-shifted by their distances from **middle C**.*

The only parameter is the "frequency" (sample rate / size) of the buffer used in the pitch-shifting algorithm, which is tunable to both pitch and tempo values.
