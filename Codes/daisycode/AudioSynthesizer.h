#pragma once
#include "daisysp.h"
#include "ConfigManager.h"

// Voice states
enum VoiceState {
    VOICE_IDLE,
    VOICE_ATTACK,
    VOICE_DECAY,
    VOICE_SUSTAIN,
    VOICE_RELEASE
};

// Waveform types
enum WaveformType {
    WAVE_SINE = 0,
    WAVE_SAW,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    WAVE_NOISE
};

// Individual voice structure
struct Voice {
    // DSP components
    daisysp::Oscillator oscillator;
    daisysp::Adsr envelope;
    daisysp::OnePole filter;
    
    // Voice parameters
    bool active;
    uint8_t note;
    uint8_t velocity;
    float frequency;
    float amplitude;
    VoiceState state;
    
    // Timing
    uint32_t noteOnTime;
    uint32_t noteOffTime;
    
    // Modulation
    float pitchBend;
    float modulation;
};

class AudioSynthesizer {
public:
    AudioSynthesizer();
    ~AudioSynthesizer();
    
    // Initialization
    void Init(float sampleRate, ConfigManager* config);
    
    // Main audio processing (call from audio callback)
    void Process(float* output, size_t size);
    void ProcessStereo(float* outputLeft, float* outputRight, size_t size);
    
    // Note control
    void NoteOn(uint8_t note, uint8_t velocity);
    void NoteOff(uint8_t note);
    void AllNotesOff();
    
    // Voice management
    uint8_t GetActiveVoiceCount();
    bool IsVoiceActive(uint8_t voiceIndex);
    Voice* GetVoice(uint8_t voiceIndex);
    
    // Real-time parameter control
    void SetMasterVolume(float volume);
    void SetWaveform(WaveformType waveform);
    void SetReverbLevel(float level);
    void SetFilterCutoff(float cutoff);
    void SetFilterResonance(float resonance);
    
    // ADSR envelope control
    void SetAttackTime(float timeSeconds);
    void SetDecayTime(float timeSeconds);
    void SetSustainLevel(float level);
    void SetReleaseTime(float timeSeconds);
    
    // Effects control
    void SetReverbEnabled(bool enabled);
    void SetDelayEnabled(bool enabled);
    void SetDelayTime(float timeSeconds);
    void SetDelayFeedback(float feedback);
    
    // Modulation
    void SetPitchBend(float semitones);
    void SetModulation(float amount);
    void SetVibratoRate(float hz);
    void SetVibratoDepth(float semitones);
    
    // Presets and configuration
    void LoadPreset(uint8_t presetNumber);
    void SavePreset(uint8_t presetNumber);
    void UpdateFromConfig();
    
    // Analysis and monitoring
    float GetOutputLevel();
    float GetCPUUsage();
    uint32_t GetProcessingTime();
    
private:
    // Configuration
    ConfigManager* config_;
    float sampleRate_;
    
    // Voice management
    static const uint8_t MAX_VOICES = 16;
    Voice voices_[MAX_VOICES];
    uint8_t activeVoiceCount_;
    uint8_t voiceAllocationIndex_; // Round-robin voice allocation
    
    // Global parameters
    float masterVolume_;
    WaveformType currentWaveform_;
    
    // Global effects
    daisysp::OnePole reverb_;  // Will be replaced with ReverbSc later
    daisysp::DelayLine<float, 48000> delay_;
    daisysp::OnePole globalLowPass_;
    daisysp::OnePole globalHighPass_;
    
    // Modulation sources
    daisysp::Oscillator lfo_;           // Low frequency oscillator
    daisysp::Oscillator vibratoLfo_;    // Vibrato LFO
    float pitchBendAmount_;
    float modulationAmount_;
    
    // Effect parameters
    bool reverbEnabled_;
    float reverbLevel_;
    bool delayEnabled_;
    float delayTime_;
    float delayFeedback_;
    float filterCutoff_;
    float filterResonance_;
    
    // Performance monitoring
    uint32_t lastProcessingTime_;
    float currentOutputLevel_;
    
    // Private methods
    
    // Voice management
    Voice* GetFreeVoice();
    Voice* FindVoice(uint8_t note);
    void InitializeVoice(Voice* voice, uint8_t note, uint8_t velocity);
    void ReleaseVoice(Voice* voice);
    void UpdateVoiceParameters(Voice* voice);
    
    // Audio processing helpers
    void ProcessVoices(float* buffer, size_t size);
    void ProcessEffects(float* buffer, size_t size);
    void ProcessGlobalFilter(float* buffer, size_t size);
    void ApplyMasterVolume(float* buffer, size_t size);
    
    // Frequency and note conversion
    float GetNoteFrequency(uint8_t midiNote);
    float MidiToFrequency(float midiNote);
    float SemitonesToRatio(float semitones);
    
    // Modulation processing
    void UpdateModulation();
    float GetVibratoValue();
    float GetLFOValue();
    
    // Utility functions
    void ClearBuffer(float* buffer, size_t size);
    void MixBuffers(float* dest, const float* src, size_t size, float gain);
    float ClampValue(float value, float min, float max);
    
    // Configuration helpers
    void ApplyConfigToVoices();
    void UpdateEnvelopeSettings();
    void UpdateEffectSettings();
};