#pragma once
#include <stdint.h>

// Configuration structure for the Laser Harp
// Simplified version: Arduino handles beam detection, Daisy handles MIDI/Audio
struct LaserHarpConfig {
    // Beam configuration (7 inputs from Arduino)
    uint8_t numBeams;           // Number of laser beams (fixed to 7)
    uint8_t baseNote;           // MIDI note for first beam (C4 = 60)
    uint8_t noteInterval;       // Interval between notes (1=chromatic, 2=whole tone, etc.)
    
    // MIDI configuration
    uint8_t midiChannel;        // MIDI channel (1-16)
    uint8_t midiVelocity;       // Default MIDI velocity
    bool midiEnabled;           // Enable/disable MIDI output
    bool audioEnabled;          // Enable/disable audio output
    
    // Audio configuration
    float reverbLevel;          // Reverb level (0.0 - 1.0)
    float masterVolume;         // Master volume (0.0 - 1.0)
    uint8_t waveform;           // Oscillator waveform type
    float attackTime;           // ADSR attack time (seconds)
    float decayTime;            // ADSR decay time (seconds)
    float sustainLevel;         // ADSR sustain level (0.0 - 1.0)
    float releaseTime;          // ADSR release time (seconds)
};

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    // Initialization
    void Init();
    
    // Configuration management
    void LoadDefaults();
    void SaveConfig();
    void LoadConfig();
    bool IsConfigValid();
    
    // Getters
    LaserHarpConfig* GetConfig();
    const LaserHarpConfig* GetConfig() const;
    
    // Individual parameter access
    uint8_t GetNumBeams() const;
    uint8_t GetBaseNote() const;
    uint8_t GetMidiChannel() const;
    float GetMasterVolume() const;
    bool IsAudioEnabled() const;
    bool IsMidiEnabled() const;
    
    // Individual parameter setters
    void SetNumBeams(uint8_t numBeams);
    void SetBaseNote(uint8_t baseNote);
    void SetMidiChannel(uint8_t channel);
    void SetMasterVolume(float volume);
    void SetAudioEnabled(bool enabled);
    void SetMidiEnabled(bool enabled);
    
    // Calibration helpers
    void StartCalibration();
    void EndCalibration();
    bool IsCalibrating() const;
    
private:
    LaserHarpConfig config_;
    bool isCalibrating_;
    bool configLoaded_;
    
    // Internal validation
    void ValidateConfig();
    void ClampValues();
    
    // Storage helpers (for future EEPROM/Flash storage)
    void WriteToStorage();
    bool ReadFromStorage();
    uint32_t CalculateChecksum();
};