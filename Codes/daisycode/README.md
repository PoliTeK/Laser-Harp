# LaserHarp Project

A laser harp project using the Daisy Seed platform.

## Description

This project implements a laser harp that detects the interruption of laser beams to generate sounds and musical notes. The Daisy Seed is used for audio processing and system control.

## Project Structure

- `LaserHarp.cpp` - Main file with the C++ code
- `Makefile` - Configuration file for compilation
- `README.md` - This file

## How to Compile

1. Make sure you have the ARM toolchain installed (`arm-none-eabi-gcc`)
2. Navigate to the project directory
3. Run the build:
   ```bash
   make
   ```

## How to Flash to the Daisy Seed

1. Put the Daisy Seed into DFU mode:
   - Press and hold the `BOOT` button
   - Press and release `RESET`
   - Release `BOOT`

2. Flash the firmware:
   ```bash
   make program-dfu
   ```

## Planned Features

- [ ] Laser beam interruption detection
- [ ] Musical note generation
- [ ] MIDI output
- [ ] Real-time audio effects
- [ ] Configuration controls

## Required Hardware

- Daisy Seed
- Laser modules
- Photoresistors or photodiodes
- Signal conditioning circuits
- Audio amplifier (optional)

## Notes


The current code implements a simple audio pass-through and blinks the LED to verify that the system works. The specific laser harp features will be implemented progressively.
