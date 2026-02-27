#include "MidiController.h"

// Constructor
MidiController::MidiController() 
    : hardware_(nullptr), config_(nullptr), midiChannel_(1), 
      outputMode_(MIDI_USB_ONLY), enabled_(true), usbConnected_(false), 
      uartConnected_(false), queueHead_(0), queueTail_(0), queueCount_(0),
      messagesSent_(0), lastActivityTime_(0), runningStatus_(0), 
      activeNoteCount_(0), lastClockTime_(0), clockDivision_(24), 
      clockRunning_(false) {
    
    // Initialize active notes array
    for (int i = 0; i < 128; i++) {
        activeNotes_[i] = false;
    }
}

// Destructor
MidiController::~MidiController() {
    // Destructor body - will be implemented later
}

// Initialization
void MidiController::Init(daisy::DaisySeed* hw, ConfigManager* config) {
    hardware_ = hw;
    config_ = config;
    
    // Get MIDI channel from config
    if (config_) {
        midiChannel_ = config_->GetMidiChannel();
    }
    
    // Initialize MIDI interfaces based on output mode
    InitializeUSB();
    InitializeUART();
    
    // Reset message statistics
    messagesSent_ = 0;
    lastActivityTime_ = daisy::System::GetNow();
}

// Main update function
void MidiController::Update() {
    // Update connection status
    CheckUSBConnection();
    CheckUARTConnection();
    
    // Process any pending messages in the queue
    ProcessMessageQueue();
    
    // Update timestamp
    lastActivityTime_ = daisy::System::GetNow();
}

// Note messages
void MidiController::SendNoteOn(uint8_t note, uint8_t velocity) {
    if (!enabled_ || !IsValidNote(note) || !IsValidVelocity(velocity)) {
        return;
    }
    
    uint8_t status = CreateStatusByte(MIDI_NOTE_ON, midiChannel_);
    QueueMessage(status, note, velocity);
    TrackNoteOn(note);
}

void MidiController::SendNoteOff(uint8_t note) {
    SendNoteOff(note, 64); // Default velocity
}

void MidiController::SendNoteOff(uint8_t note, uint8_t velocity) {
    if (!enabled_ || !IsValidNote(note) || !IsValidVelocity(velocity)) {
        return;
    }
    
    uint8_t status = CreateStatusByte(MIDI_NOTE_OFF, midiChannel_);
    QueueMessage(status, note, velocity);
    TrackNoteOff(note);
}

void MidiController::SendAllNotesOff() {
    // Send MIDI CC All Notes Off
    SendControlChange(MIDI_CC_ALL_NOTES_OFF, 0);
    
    // Also send individual note offs for tracked notes
    for (int i = 0; i < 128; i++) {
        if (activeNotes_[i]) {
            SendNoteOff(i);
        }
    }
}

// Control messages
void MidiController::SendControlChange(uint8_t controller, uint8_t value) {
    if (!enabled_ || !IsValidController(controller)) {
        return;
    }
    
    uint8_t status = CreateStatusByte(MIDI_CONTROL_CHANGE, midiChannel_);
    QueueMessage(status, controller, value);
}

void MidiController::SendProgramChange(uint8_t program) {
    if (!enabled_ || program > 127) {
        return;
    }
    
    uint8_t status = CreateStatusByte(MIDI_PROGRAM_CHANGE, midiChannel_);
    QueueMessage(status, program);
}

void MidiController::SendPitchBend(uint16_t value) {
    if (!enabled_) {
        return;
    }
    
    uint8_t status = CreateStatusByte(MIDI_PITCH_BEND, midiChannel_);
    uint8_t lsb = value & 0x7F;
    uint8_t msb = (value >> 7) & 0x7F;
    QueueMessage(status, lsb, msb);
}

void MidiController::SendChannelPressure(uint8_t pressure) {
    if (!enabled_ || pressure > 127) {
        return;
    }
    
    uint8_t status = CreateStatusByte(MIDI_CHANNEL_PRESSURE, midiChannel_);
    QueueMessage(status, pressure);
}

void MidiController::SendPolyPressure(uint8_t note, uint8_t pressure) {
    if (!enabled_ || !IsValidNote(note) || pressure > 127) {
        return;
    }
    
    uint8_t status = CreateStatusByte(MIDI_POLY_PRESSURE, midiChannel_);
    QueueMessage(status, note, pressure);
}

// Configuration
void MidiController::SetChannel(uint8_t channel) {
    if (IsValidChannel(channel)) {
        midiChannel_ = channel;
    }
}

void MidiController::SetOutputMode(MidiOutputMode mode) {
    outputMode_ = mode;
}

void MidiController::SetEnabled(bool enabled) {
    enabled_ = enabled;
}

// Queue management
bool MidiController::HasPendingMessages() {
    return queueCount_ > 0;
}

void MidiController::FlushMessageQueue() {
    // Process all pending messages immediately
    ProcessMessageQueue();
}

void MidiController::ClearMessageQueue() {
    queueHead_ = 0;
    queueTail_ = 0;
    queueCount_ = 0;
}

// Status and diagnostics
uint8_t MidiController::GetCurrentChannel() {
    return midiChannel_;
}

MidiOutputMode MidiController::GetOutputMode() {
    return outputMode_;
}

bool MidiController::IsEnabled() {
    return enabled_;
}

uint32_t MidiController::GetMessagesSent() {
    return messagesSent_;
}

uint32_t MidiController::GetLastActivityTime() {
    return lastActivityTime_;
}

bool MidiController::SelfTest() {
    // Perform basic self-test
    bool usbOk = true;
    bool uartOk = true;
    
    // Test queue functionality
    bool queueOk = (queueCount_ <= MESSAGE_QUEUE_SIZE);
    
    // Test MIDI channel validity
    bool channelOk = IsValidChannel(midiChannel_);
    
    return usbOk && uartOk && queueOk && channelOk;
}

// Advanced features
void MidiController::SendSysEx(uint8_t* data, size_t length) {
    if (!enabled_ || length < 1) return;
    
    // SysEx messages start with 0xF0 and end with 0xF7
    uint8_t* sysexMsg = new uint8_t[length + 2];
    sysexMsg[0] = 0xF0; // SysEx start
    for (size_t i = 0; i < length; i++) {
        sysexMsg[i + 1] = data[i];
    }
    sysexMsg[length + 1] = 0xF7; // SysEx end
    
    if (outputMode_ == MIDI_USB_ONLY || outputMode_ == MIDI_BOTH) {
        if (usbConnected_) {
            SendViaUSB(sysexMsg, length + 2);
        }
    }
    
    if (outputMode_ == MIDI_UART_ONLY || outputMode_ == MIDI_BOTH) {
        if (uartConnected_) {
            SendViaUART(sysexMsg, length + 2);
        }
    }
    
    delete[] sysexMsg;
    messagesSent_++;
}

void MidiController::SendMTC(uint8_t frameType, uint8_t value) {
    // MIDI Time Code Quarter Frame
    uint8_t mtcByte = 0xF1;
    uint8_t dataByte = (frameType << 4) | (value & 0x0F);
    SendMidiMessage(mtcByte, dataByte);
}

void MidiController::SendSongPosition(uint16_t position) {
    uint8_t lsb = position & 0x7F;
    uint8_t msb = (position >> 7) & 0x7F;
    uint8_t bytes[3] = {0xF2, lsb, msb};
    
    if (outputMode_ == MIDI_USB_ONLY || outputMode_ == MIDI_BOTH) {
        if (usbConnected_) {
            SendViaUSB(bytes, 3);
        }
    }
    
    if (outputMode_ == MIDI_UART_ONLY || outputMode_ == MIDI_BOTH) {
        if (uartConnected_) {
            SendViaUART(bytes, 3);
        }
    }
}

void MidiController::SendClock() {
    SendMidiMessage(0xF8); // MIDI Clock
}

void MidiController::SendStart() {
    SendMidiMessage(0xFA); // MIDI Start
    clockRunning_ = true;
}

void MidiController::SendStop() {
    SendMidiMessage(0xFC); // MIDI Stop
    clockRunning_ = false;
}

void MidiController::SendContinue() {
    SendMidiMessage(0xFB); // MIDI Continue
    clockRunning_ = true;
}

void MidiController::SendSceneChange(uint8_t scene) {
    // Send as Program Change
    SendProgramChange(scene);
}

void MidiController::SendBankSelect(uint8_t bank) {
    // Bank Select MSB (CC 0)
    SendControlChange(0, bank);
}

// Private methods - implementation
void MidiController::SendMidiMessage(uint8_t status, uint8_t data1, uint8_t data2) {
    if (!enabled_) return;
    
    uint8_t midiBytes[3] = {status, data1, data2};
    
    // Send via USB if enabled
    if (outputMode_ == MIDI_USB_ONLY || outputMode_ == MIDI_BOTH) {
        if (usbConnected_) {
            SendViaUSB(midiBytes, 3);
        }
    }
    
    // Send via UART if enabled
    if (outputMode_ == MIDI_UART_ONLY || outputMode_ == MIDI_BOTH) {
        if (uartConnected_) {
            SendViaUART(midiBytes, 3);
        }
    }
    
    messagesSent_++;
    UpdateRunningStatus(status);
    lastActivityTime_ = daisy::System::GetNow();
}

void MidiController::SendMidiMessage(uint8_t status, uint8_t data1) {
    if (!enabled_) return;
    
    uint8_t midiBytes[2] = {status, data1};
    
    // Send via USB if enabled
    if (outputMode_ == MIDI_USB_ONLY || outputMode_ == MIDI_BOTH) {
        if (usbConnected_) {
            SendViaUSB(midiBytes, 2);
        }
    }
    
    // Send via UART if enabled
    if (outputMode_ == MIDI_UART_ONLY || outputMode_ == MIDI_BOTH) {
        if (uartConnected_) {
            SendViaUART(midiBytes, 2);
        }
    }
    
    messagesSent_++;
    UpdateRunningStatus(status);
    lastActivityTime_ = daisy::System::GetNow();
}

void MidiController::SendMidiMessage(uint8_t status) {
    if (!enabled_) return;
    
    uint8_t midiBytes[1] = {status};
    
    // Send via USB if enabled
    if (outputMode_ == MIDI_USB_ONLY || outputMode_ == MIDI_BOTH) {
        if (usbConnected_) {
            SendViaUSB(midiBytes, 1);
        }
    }
    
    // Send via UART if enabled
    if (outputMode_ == MIDI_UART_ONLY || outputMode_ == MIDI_BOTH) {
        if (uartConnected_) {
            SendViaUART(midiBytes, 1);
        }
    }
    
    messagesSent_++;
    UpdateRunningStatus(status);
    lastActivityTime_ = daisy::System::GetNow();
}

void MidiController::QueueMessage(uint8_t status, uint8_t data1, uint8_t data2) {
    if (IsQueueFull()) {
        return; // Queue is full, drop message
    }
    
    MidiMessage& msg = messageQueue_[queueTail_];
    msg.status = status;
    msg.data1 = data1;
    msg.data2 = data2;
    msg.hasData2 = true;
    msg.timestamp = lastActivityTime_; // TODO: Get actual timestamp
    
    queueTail_ = (queueTail_ + 1) % MESSAGE_QUEUE_SIZE;
    queueCount_++;
}

void MidiController::QueueMessage(uint8_t status, uint8_t data1) {
    if (IsQueueFull()) {
        return;
    }
    
    MidiMessage& msg = messageQueue_[queueTail_];
    msg.status = status;
    msg.data1 = data1;
    msg.data2 = 0;
    msg.hasData2 = false;
    msg.timestamp = lastActivityTime_;
    
    queueTail_ = (queueTail_ + 1) % MESSAGE_QUEUE_SIZE;
    queueCount_++;
}

void MidiController::ProcessMessageQueue() {
    // Process all pending messages in the queue
    while (queueCount_ > 0) {
        MidiMessage& msg = messageQueue_[queueHead_];
        
        if (msg.hasData2) {
            SendMidiMessage(msg.status, msg.data1, msg.data2);
        } else {
            SendMidiMessage(msg.status, msg.data1);
        }
        
        // Move to next message
        queueHead_ = (queueHead_ + 1) % MESSAGE_QUEUE_SIZE;
        queueCount_--;
    }
}

bool MidiController::IsQueueFull() {
    return queueCount_ >= MESSAGE_QUEUE_SIZE;
}

void MidiController::SendViaUSB(uint8_t* data, size_t length) {
    usbMidi_.SendMessage(data, length);
    // Note: SendMessage returns void, no error checking available
}

void MidiController::SendViaUART(uint8_t* data, size_t length) {
    uartMidi_.SendMessage(data, length);
}

void MidiController::InitializeUSB() {
    daisy::MidiUsbHandler::Config usbConfig;
    usbConfig.transport_config.periph = daisy::MidiUsbTransport::Config::INTERNAL;
    usbMidi_.Init(usbConfig);
    usbConnected_ = true; // Assume connected for now
}

void MidiController::InitializeUART() {
    daisy::MidiUartHandler::Config uartConfig;
    // Use default UART configuration - pins D14/D15 for UART1
    uartMidi_.Init(uartConfig);
    uartConnected_ = true; // Assume connected for now
}

void MidiController::CheckUSBConnection() {
    // Check if USB device is connected
    // This is a simplified check - in real implementation you might
    // want to check USB enumeration status
    usbConnected_ = true; // For now, assume always connected
}

void MidiController::CheckUARTConnection() {
    // Check if UART device is connected
    // This is difficult to detect automatically for UART MIDI
    uartConnected_ = true; // For now, assume always connected
}

void MidiController::HandleConnectionChange() {
    // Handle connection/disconnection events
    // Could be used to notify the application about MIDI device status
}

uint8_t MidiController::CreateStatusByte(MidiMessageType messageType, uint8_t channel) {
    return (uint8_t)messageType | ((channel - 1) & 0x0F);
}

bool MidiController::IsValidChannel(uint8_t channel) {
    return channel >= 1 && channel <= 16;
}

bool MidiController::IsValidNote(uint8_t note) {
    return note <= 127;
}

bool MidiController::IsValidVelocity(uint8_t velocity) {
    return velocity <= 127;
}

bool MidiController::IsValidController(uint8_t controller) {
    return controller <= 127;
}

void MidiController::TrackNoteOn(uint8_t note) {
    if (note < 128 && !activeNotes_[note]) {
        activeNotes_[note] = true;
        activeNoteCount_++;
    }
}

void MidiController::TrackNoteOff(uint8_t note) {
    if (note < 128 && activeNotes_[note]) {
        activeNotes_[note] = false;
        if (activeNoteCount_ > 0) {
            activeNoteCount_--;
        }
    }
}

bool MidiController::IsNoteActive(uint8_t note) {
    return note < 128 && activeNotes_[note];
}

void MidiController::UpdateRunningStatus(uint8_t status) {
    runningStatus_ = status;
}

bool MidiController::CanUseRunningStatus(uint8_t status) {
    return status == runningStatus_ && status >= 0x80 && status < 0xF0;
}

void MidiController::LogMidiActivity(const char* action, uint8_t data1, uint8_t data2) {
    // TODO: Implement MIDI activity logging if needed
    // Could be useful for debugging
}

void MidiController::UpdateStatistics() {
    // Update internal statistics
    // Could track message rates, error counts, etc.
}