# Repitch

![](screenshot.png)

### To use,

- Download Repitch as a [standalone application](https://github.com/maxwellpollack/repitch/releases/latest/download/Repitch.app.zip) (Mac only),

### or

- Download Repitch as an audio plugin in [Audio Unit](https://github.com/maxwellpollack/repitch/releases/latest/download/repitch.component.zip) (Mac) or [VST](https://github.com/maxwellpollack/repitch/releases/latest/download/repitch.vst3.zip) (Windows) format,
- Install it in your DAW,
- Place it on a MIDI instrument track, and
- Assign it an audio input (called a "sidechain" in some DAWs).

Then, play some MIDI notes. *The audio input will be polyphonically pitch-shifted by their distances from **middle C**.*

The only parameter is the **"frequency"** (sample rate / size) of the [variable delay line](http://msp.ucsd.edu/techniques/latest/book-html/node115.html) used in the pitch-shifting algorithm, which is tunable to both pitch and tempo values.

---

*Made with ❤️ at [the Recurse Center](https://www.recurse.com)*
