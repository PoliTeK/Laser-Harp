# MIDI Test Requirements

Install Python MIDI library:
```bash
pip install mido python-rtmidi
```

Run the monitor:
```bash
python test_midi_monitor.py
```

## Expected Output

When you connect/disconnect pins D0-D6 to GND:
- **D0 → Note 60 (C4)** - Beam 0
- **D1 → Note 62 (D4)** - Beam 1
- **D2 → Note 64 (E4)** - Beam 2
- **D3 → Note 66 (F#4)** - Beam 3
- **D4 → Note 68 (G#4)** - Beam 4
- **D5 → Note 70 (A#4)** - Beam 5
- **D6 → Note 72 (C5)** - Beam 6

## Testing Procedure

1. Connect Daisy Seed via USB
2. Run `test_midi_monitor.py`
3. Connect a wire from GND to D0 → should see **NOTE ON 60**
4. Disconnect wire → should see **NOTE OFF 60**
5. Repeat for D1-D6

## Troubleshooting

If Daisy Seed doesn't appear as MIDI device:
- Check USB cable (must support data, not just power)
- Try different USB port
- Restart Daisy Seed (unplug/replug)
- Check Windows Device Manager for "Daisy Seed" under Sound/MIDI devices
