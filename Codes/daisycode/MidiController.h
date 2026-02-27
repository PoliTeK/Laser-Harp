#pragma once
#include "daisy_seed.h"
#include "ConfigManager.h"
#include "hid/midi.h"

// MIDI message types
enum MidiMessageType {
    MIDI_NOTE_OFF = 0x80,
    MIDI_NOTE_ON = 0x90,
    MIDI_POLY_PRESSURE = 0xA0,
    MIDI_CONTROL_CHANGE = 0xB0,
    MIDI_PROGRAM_CHANGE = 0xC0,
    MIDI_CHANNEL_PRESSURE = 0xD0,
    MIDI_PITCH_BEND = 0xE0,
    MIDI_SYSTEM_EXCLUSIVE = 0xF0
};

// Common MIDI control change numbers
enum MidiControlChange {
    MIDI_CC_MODULATION = 1,
    MIDI_CC_VOLUME = 7,
    MIDI_CC_PAN = 10,
    MIDI_CC_EXPRESSION = 11,
    MIDI_CC_SUSTAIN = 64,
    MIDI_CC_REVERB = 91,
    MIDI_CC_CHORUS = 93,
    MIDI_CC_ALL_NOTES_OFF = 123
};

// MIDI output modes
enum MidiOutputMode {
    MIDI_USB_ONLY,
    MIDI_UART_ONLY,
    MIDI_BOTH
};

// Structure for queued MIDI messages
struct MidiMessage {
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
    uint32_t timestamp;
    bool hasData2;  // Some messages only have 1 data byte
};

class MidiController {
public:
    MidiController();
    ~MidiController();
    
    // Initialization
    void Init(daisy::DaisySeed* hw, ConfigManager* config);
    
    // Main update function (call in main loop)
    void Update();
    
    // Note messages
    void SendNoteOn(uint8_t note, uint8_t velocity);
    void SendNoteOff(uint8_t note);
    void SendNoteOff(uint8_t note, uint8_t velocity);
    void SendAllNotesOff();
    
    // Control messages
    void SendControlChange(uint8_t controller, uint8_t value);
    void SendProgramChange(uint8_t program);
    void SendPitchBend(uint16_t value);
    void SendChannelPressure(uint8_t pressure);
    void SendPolyPressure(uint8_t note, uint8_t pressure);
    
    // Configuration
    void SetChannel(uint8_t channel);
    void SetOutputMode(MidiOutputMode mode);
    void SetEnabled(bool enabled);
    
    // Queue management
    bool HasPendingMessages();
    void FlushMessageQueue();
    void ClearMessageQueue();
    
    // Status and diagnostics
    uint8_t GetCurrentChannel();
    MidiOutputMode GetOutputMode();
    bool IsEnabled();
    uint32_t GetMessagesSent();
    uint32_t GetLastActivityTime();
    bool SelfTest();
    
    // Advanced features
    void SendSysEx(uint8_t* data, size_t length);
    void SendMTC(uint8_t frameType, uint8_t value);  // MIDI Time Code
    void SendSongPosition(uint16_t position);
    void SendClock();
    void SendStart();
    void SendStop();
    void SendContinue();
    
    // Preset and scene management
    void SendSceneChange(uint8_t scene);
    void SendBankSelect(uint8_t bank);
    
private:
    // Hardware references
    daisy::DaisySeed* hardware_;
    ConfigManager* config_;
    
    // MIDI interfaces
    daisy::MidiUartHandler uartMidi_;
    daisy::MidiUsbHandler usbMidi_;
    
    // Configuration
    uint8_t midiChannel_;
    MidiOutputMode outputMode_;
    bool enabled_;
    bool usbConnected_;
    bool uartConnected_;
    
    // Message queue
    static const uint8_t MESSAGE_QUEUE_SIZE = 64;
    MidiMessage messageQueue_[MESSAGE_QUEUE_SIZE];
    uint8_t queueHead_;
    uint8_t queueTail_;
    uint8_t queueCount_;
    
    // Status tracking
    uint32_t messagesSent_;
    uint32_t lastActivityTime_;
    uint8_t runningStatus_;  // For MIDI running status optimization
    
    // Note tracking (for all notes off functionality)
    bool activeNotes_[128];  // Track which notes are currently on
    uint8_t activeNoteCount_;
    
    // Timing and synchronization
    uint32_t lastClockTime_;
    uint16_t clockDivision_;
    bool clockRunning_;
    
    // Private methods
    
    // Core MIDI transmission
    void SendMidiMessage(uint8_t status, uint8_t data1, uint8_t data2);
    void SendMidiMessage(uint8_t status, uint8_t data1);
    void SendMidiMessage(uint8_t status);
    
    // Queue management
    void QueueMessage(uint8_t status, uint8_t data1, uint8_t data2);
    void QueueMessage(uint8_t status, uint8_t data1);
    void ProcessMessageQueue();
    bool IsQueueFull();
    
    // Hardware interfaces
    void SendViaUSB(uint8_t* data, size_t length);
    void SendViaUART(uint8_t* data, size_t length);
    void InitializeUSB();
    void InitializeUART();
    
    // Connection management
    void CheckUSBConnection();
    void CheckUARTConnection();
    void HandleConnectionChange();
    
    // Utility functions
    uint8_t CreateStatusByte(MidiMessageType messageType, uint8_t channel);
    bool IsValidChannel(uint8_t channel);
    bool IsValidNote(uint8_t note);
    bool IsValidVelocity(uint8_t velocity);
    bool IsValidController(uint8_t controller);
    
    // Note tracking
    void TrackNoteOn(uint8_t note);
    void TrackNoteOff(uint8_t note);
    bool IsNoteActive(uint8_t note);
    
    // Running status optimization
    void UpdateRunningStatus(uint8_t status);
    bool CanUseRunningStatus(uint8_t status);
    
    // Diagnostics
    void LogMidiActivity(const char* action, uint8_t data1, uint8_t data2);
    void UpdateStatistics();
};