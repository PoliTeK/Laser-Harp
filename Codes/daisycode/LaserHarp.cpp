#include "daisy_seed.h"
#include "MidiController.h"
#include "AudioSynthesizer.h"
#include "ConfigManager.h"

// ==============================================================================
// LASER HARP - Daisy Seed MIDI/Audio Controller
// ==============================================================================
// Arduino handles: Stepper motor, laser, LDR sensors, beam detection
// Daisy handles: 7 digital inputs → MIDI output + Audio synthesis
// ==============================================================================

using namespace daisy;
using namespace daisy::seed;

// Hardware
DaisySeed hardware;

// System components
ConfigManager configManager;
MidiController midiController;
AudioSynthesizer audioSynthesizer;

// 7 Digital inputs from Arduino (beam detection signals)
GPIO beamInputs[7];
bool beamStates[7];          // Current state of each beam
bool previousBeamStates[7];  // Previous state for edge detection
uint32_t lastDebounceTime[7]; // Debounce timing
const uint32_t DEBOUNCE_DELAY_MS = 20; // 20ms debounce

// MIDI note mapping (configured from ConfigManager)
uint8_t beamNotes[7];

// Audio callback
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    // Process audio synthesis
    audioSynthesizer.ProcessStereo(out[0], out[1], size);
}

// Initialize system
void InitializeSystem() {
    // Initialize hardware
    hardware.Configure();
    hardware.Init();
    hardware.SetAudioBlockSize(48); // 48 samples = 1ms @ 48kHz
    
    // Initialize configuration
    configManager.Init();
    
    // Configure 7 digital input pins from Arduino
    // Using pins D0-D6 as inputs with pull-down resistors
    for (int i = 0; i < 7; i++) {
        Pin inputPin;
        switch(i) {
            case 0: inputPin = D0; break;
            case 1: inputPin = D1; break;
            case 2: inputPin = D2; break;
            case 3: inputPin = D3; break;
            case 4: inputPin = D4; break;
            case 5: inputPin = D5; break;
            case 6: inputPin = D6; break;
        }
        beamInputs[i].Init(inputPin, GPIO::Mode::INPUT, GPIO::Pull::PULLDOWN);
        beamStates[i] = false;
        previousBeamStates[i] = false;
        lastDebounceTime[i] = 0;
    }
    
    // Initialize MIDI controller
    midiController.Init(&hardware, &configManager);
    
    // Initialize audio synthesizer
    audioSynthesizer.Init(hardware.AudioSampleRate(), &configManager);
    
    // Setup note mapping based on configuration
    uint8_t baseNote = configManager.GetBaseNote();
    uint8_t interval = configManager.GetConfig()->noteInterval;
    for (int i = 0; i < 7; i++) {
        beamNotes[i] = baseNote + (i * interval);
    }
    
    // Start audio
    hardware.StartAudio(AudioCallback);
    
    // Startup indication
    hardware.SetLed(true);
    System::Delay(500);
    hardware.SetLed(false);
    System::Delay(200);
    hardware.SetLed(true);
    System::Delay(200);
    hardware.SetLed(false);
}

// Read and debounce beam inputs from Arduino
void UpdateBeamInputs() {
    uint32_t currentTime = System::GetNow();
    
    for (int i = 0; i < 7; i++) {
        // Read current state (HIGH = beam broken)
        bool currentState = beamInputs[i].Read();
        
        // Debounce logic
        if (currentState != beamStates[i]) {
            if ((currentTime - lastDebounceTime[i]) > DEBOUNCE_DELAY_MS) {
                // State change confirmed
                beamStates[i] = currentState;
                lastDebounceTime[i] = currentTime;
                
                // Detect edges and trigger MIDI/Audio
                if (currentState && !previousBeamStates[i]) {
                    // Rising edge: Beam broken (Note ON)
                    uint8_t note = beamNotes[i];
                    uint8_t velocity = configManager.GetConfig()->midiVelocity;
                    
                    if (configManager.IsMidiEnabled()) {
                        midiController.SendNoteOn(note, velocity);
                    }
                    if (configManager.IsAudioEnabled()) {
                        audioSynthesizer.NoteOn(note, velocity);
                    }
                    
                    // LED feedback
                    hardware.SetLed(true);
                }
                else if (!currentState && previousBeamStates[i]) {
                    // Falling edge: Beam restored (Note OFF)
                    uint8_t note = beamNotes[i];
                    
                    if (configManager.IsMidiEnabled()) {
                        midiController.SendNoteOff(note);
                    }
                    if (configManager.IsAudioEnabled()) {
                        audioSynthesizer.NoteOff(note);
                    }
                    
                    // Turn off LED if no beams active
                    bool anyActive = false;
                    for (int j = 0; j < 7; j++) {
                        if (beamStates[j]) anyActive = true;
                    }
                    if (!anyActive) hardware.SetLed(false);
                }
                
                previousBeamStates[i] = currentState;
            }
        }
    }
}

// Main loop
int main(void) {
    // Initialize everything
    InitializeSystem();
    
    // Main loop
    for(;;) {
        // Update beam inputs (read from Arduino)
        UpdateBeamInputs();
        
        // Update MIDI controller
        midiController.Update();
        
        // Small delay to avoid CPU overload
        System::Delay(1);
    }
}