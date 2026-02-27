# Laser-Harp
Laser Harp repo
# Laser-Harp
Laser Harp repo


## What is a laser harp?

A **laser harp** is an electronic musical instrument that lets a performer “play” music by interrupting a set of light beams—most commonly **laser beams** (or sometimes high-power LEDs) arranged like the strings of a harp.

- Each beam acts like a **virtual string**.
- When your hand breaks a beam, the instrument detects that interruption using **light sensors** (typically photodiodes, photoresistors/LDRs, or phototransistors).
- A controller (often a microcontroller such as an Arduino, daisy, ESP32, etc.) translates that event into a musical message, for example:
  - triggering a note (MIDI Note On/Off),
  - triggering a sample,
  - changing an effect parameter (filters, reverb, delay),
  - or controlling volume/expression.

Laser harps are popular in live performances because they are both **musically expressive** and **visually striking**, giving the audience a clear “see the music” experience.


## Our AIM

Developing a laser harp usually focusing on a combination of musical, technical, and creative goals:

### 1. Create a new performance interface
The main aim is to build an instrument that feels different from a keyboard or guitar—one that turns **gestures in space** into music.

### 2. Combine sound with a strong visual show
Laser harps are designed to be stage-friendly. The beams provide immediate visual feedback and can be synchronized with lighting and effects to enhance a performance.

### 3. Explore expressive control
Beyond simple “note on/note off,” many laser harp projects aim to capture more nuance, such as:
- velocity (how fast a beam is broken),
- aftertouch-like behavior (keeping a hand in a beam),
- pitch bend or modulation using distance/position sensing,
- dynamic mapping of beams to scales/chords.

### 4. Build an accessible, customizable instrument
A DIY laser harp can be tailored to a specific musician’s needs:
- number of beams (“strings”),
- scale and tuning modes,
- instrument voices and sound design,
- portability and power options,
- safety and beam brightness choices.

### 5. Learn and demonstrate engineering concepts
Laser harp development is often an educational goal as well, combining:
- optics and alignment,
- electronics and sensor design,
- embedded programming,
- MIDI/audio systems,
- real-time responsiveness and latency reduction.


## Notes on safety

If using actual lasers, development should prioritize eye safety:
- avoid eye-level beams,
- use appropriate laser classes and protective design choices,
- prefer diffused/expanded beams and controlled environments,
- consider alternatives (LED + optics) when appropriate.
