#include "LaserBeamManager.h"
#include <cmath>

// Constants based on Arduino implementation
const float FILTER_ALPHA = 0.3f;                    // Low-pass filter coefficient
const uint32_t DEBOUNCE_TIME_MS = 5;                // Debounce time in milliseconds
const uint32_t SENSOR_UPDATE_INTERVAL_US = 200;     // 200us = 5kHz update rate
const uint16_t DEFAULT_THRESHOLD = 700;             // From Arduino: analogVal <= 700
const int STEPS_PER_REVOLUTION = 200;               // Arduino: stepsPerRev = 200
const int DEFAULT_BEAMS = 6;                        // Arduino: corde = 6
const int PULSE_WIDTH_US = 50;                      // Arduino: pulseWidthMicros = 50
const int STEP_DELAY_US = 100;                      // Arduino: millisBtwnSteps = 100
const int BEAM_CHECK_TIME_MS = 3;                   // Arduino: tempo = 3

// Constructor
LaserBeamManager::LaserBeamManager() 
    : hardware_(nullptr), config_(nullptr), currentServoPosition_(0.0f), 
      targetServoPosition_(0.0f), lastServoUpdate_(0), servoState_(SERVO_IDLE),
      servoDirection_(1.0f), eventQueueHead_(0), eventQueueTail_(0), 
      eventQueueCount_(0), isCalibrating_(false), calibrationBeam_(0),
      calibrationSampleCount_(0), lastUpdateTime_(0), updateInterval_(SENSOR_UPDATE_INTERVAL_US) {
    
    // Initialize arrays with Arduino-based defaults
    for (int i = 0; i < 16; i++) {
        sensorValues_[i] = 0.0f;
        filteredValues_[i] = 1000.0f;              // Start above threshold
        beamStates_[i] = true;                     // Beam not broken initially
        previousStates_[i] = true;
        lastStateChange_[i] = 0;
        thresholds_[i] = DEFAULT_THRESHOLD;        // Arduino threshold
    }
    
    // Initialize stepper motor variables
    currentStepPosition_ = 0;
    targetStepPosition_ = 0;
    lastStepTime_ = 0;
    scanDirection_ = 1;                            // Arduino: starts moving forward
    currentBeamIndex_ = 0;
    stepsPerBeam_ = STEPS_PER_REVOLUTION / DEFAULT_BEAMS;  // Arduino: cordemezzi = stepsPerRev/corde
    atBeamPosition_ = false;
    beamCheckStartTime_ = 0;
    laserState_ = false;
    
    // Initialize GPIO pin structures
    stepPin_ = {};
    dirPin_ = {};
    laserPin_ = {};
}

// Destructor
LaserBeamManager::~LaserBeamManager() {
    // Stop any ongoing operations
    StopScanning();
}

// Initialization
void LaserBeamManager::Init(daisy::DaisySeed* hw, ConfigManager* config) {
    hardware_ = hw;
    config_ = config;
    
    // Initialize hardware components
    InitializeServo();
    InitializeADC();
    InitializeGPIO();
    
    // Load configuration
    if (config_) {
        LoadConfigurationParameters();
    }
    
    // Start in scanning mode
    StartScanning();
}

// Main update function - Non-blocking state machine
void LaserBeamManager::Update() {
    uint32_t currentTime = daisy::System::GetNow();
    
    // Check if it's time to update
    if ((currentTime - lastUpdateTime_) >= updateInterval_) {
        
        // Always read sensors at high frequency
        ReadSensors();
        FilterSensorValues();
        
        // Update stepper motor position
        UpdateServo();
        
        // Process beam state changes
        ProcessBeamStates();
        DetectBeamEvents();
        
        // Handle calibration if active
        if (isCalibrating_) {
            ProcessCalibration();
        }
        
        lastUpdateTime_ = currentTime;
    }
}

// Event management
bool LaserBeamManager::HasEvents() {
    return eventQueueCount_ > 0;
}

bool LaserBeamManager::GetNextEvent(BeamEvent* event) {
    // TODO: Implement event dequeue
    if (eventQueueCount_ == 0) {
        return false;
    }
    
    *event = eventQueue_[eventQueueHead_];
    eventQueueHead_ = (eventQueueHead_ + 1) % EVENT_QUEUE_SIZE;
    eventQueueCount_--;
    return true;
}

void LaserBeamManager::ClearEvents() {
    eventQueueHead_ = 0;
    eventQueueTail_ = 0;
    eventQueueCount_ = 0;
}

// Calibration functions
void LaserBeamManager::StartCalibration() {
    // TODO: Implement calibration start
    isCalibrating_ = true;
}

void LaserBeamManager::EndCalibration() {
    // TODO: Implement calibration end
    isCalibrating_ = false;
}

bool LaserBeamManager::IsCalibrating() {
    return isCalibrating_;
}

void LaserBeamManager::CalibrateCurrentBeam() {
    // TODO: Implement current beam calibration
}

// Servo control
void LaserBeamManager::StartScanning() {
    servoState_ = SERVO_SCANNING;
    scanDirection_ = 1;
    currentBeamIndex_ = 0;
    currentStepPosition_ = 0;
    targetStepPosition_ = 0;
    atBeamPosition_ = false;
    
    // Reset sensor states
    for (int i = 0; i < 16; i++) {
        beamStates_[i] = true; // Not broken initially
        lastStateChange_[i] = daisy::System::GetNow();
    }
}

void LaserBeamManager::StopScanning() {
    servoState_ = SERVO_IDLE;
    SetLaserState(false);
    atBeamPosition_ = false;
}

void LaserBeamManager::SetServoPosition(float angle) {
    targetServoPosition_ = angle;
}

float LaserBeamManager::GetServoPosition() {
    return currentServoPosition_;
}

ServoState LaserBeamManager::GetServoState() {
    return servoState_;
}

// Sensor functions
float LaserBeamManager::GetSensorValue(uint8_t sensorIndex) {
    if (sensorIndex < 16) {
        return sensorValues_[sensorIndex];
    }
    return 0.0f;
}

bool LaserBeamManager::IsBeamBroken(uint8_t beamIndex) {
    if (beamIndex < 16) {
        return !beamStates_[beamIndex];
    }
    return false;
}

void LaserBeamManager::SetSensorThreshold(uint8_t sensorIndex, uint16_t threshold) {
    if (sensorIndex < 16) {
        thresholds_[sensorIndex] = threshold;
    }
}

// Status and diagnostics
uint32_t LaserBeamManager::GetLastUpdateTime() {
    return lastUpdateTime_;
}

uint8_t LaserBeamManager::GetActiveBeamCount() {
    return config_ ? config_->GetConfig()->numBeams : DEFAULT_BEAMS;
}

bool LaserBeamManager::SelfTest() {
    // TODO: Implement self test
    return true;
}

// Private methods - Hardware initialization and core functionality

// Initialize stepper motor (replaces Arduino stepper setup)
void LaserBeamManager::InitializeServo() {
    // Configure step pin (equivalent to Arduino stepYPin = 3)
    daisy::GPIO::Config step_cfg;
    step_cfg.pin = daisy::seed::D0;  // Use Daisy Seed pin D3
    step_cfg.mode = daisy::GPIO::Mode::OUTPUT;
    stepPin_.Init(step_cfg);
    
    // Configure direction pin (equivalent to Arduino dirYPin = 6)  
    daisy::GPIO::Config dir_cfg;
    dir_cfg.pin = daisy::seed::D1;   // Use Daisy Seed pin D6
    dir_cfg.mode = daisy::GPIO::Mode::OUTPUT;
    dirPin_.Init(dir_cfg);
    
    // Initialize pins to LOW
    stepPin_.Write(false);
    dirPin_.Write(false);
    
    // Initialize position tracking
    currentStepPosition_ = 0;
    targetStepPosition_ = 0;
    lastStepTime_ = 0;
}

// Initialize laser control
void LaserBeamManager::InitializeLaser() {
    // Configure laser pin (equivalent to Arduino laser = 53)
    daisy::GPIO::Config laser_cfg;
    laser_cfg.pin = daisy::seed::D17;  // Use Daisy Seed pin D17
    laser_cfg.mode = daisy::GPIO::Mode::OUTPUT;
    laserPin_.Init(laser_cfg);
    
    // Start with laser off
    laserPin_.Write(false);
    laserState_ = false;
}

// Initialize ADC for sensor reading
void LaserBeamManager::InitializeADC() {
    // Configure ADC for LDR sensor (equivalent to Arduino A14)
    daisy::AdcChannelConfig adcConfig;
    adcConfig.InitSingle(daisy::seed::A0); // Use A0 pin for LDR
    
    // Initialize ADC
    hardware_->adc.Init(&adcConfig, 1);
    hardware_->adc.Start();
}

void LaserBeamManager::InitializeGPIO() {
    // Initialize laser control
    InitializeLaser();
}

// Set laser state
void LaserBeamManager::SetLaserState(bool on) {
    laserPin_.Write(on);
    laserState_ = on;
}

// Non-blocking stepper motor update (replaces Arduino blocking for-loops)
void LaserBeamManager::UpdateServo() {
    uint32_t currentTime = daisy::System::GetNow();
    
    switch (servoState_) {
        case SERVO_SCANNING:
            UpdateScanningMotion(currentTime);
            break;
            
        case SERVO_CALIBRATING:
            UpdateCalibrationMotion(currentTime);
            break;
            
        case SERVO_IDLE:
            // Motor stopped
            break;
            
        case SERVO_ERROR:
            // Handle error state
            break;
    }
}

// Update motion during calibration
void LaserBeamManager::UpdateCalibrationMotion(uint32_t currentTime) {
    // For calibration, move slowly and methodically
    // TODO: Implement specific calibration motion
    UpdateScanningMotion(currentTime);
}

// Improved scanning motion (non-blocking version of Arduino loop)
void LaserBeamManager::UpdateScanningMotion(uint32_t currentTime) {
    // Check if it's time to make a step
    if ((currentTime - lastStepTime_) >= STEP_DELAY_US) {
        
        // Make one step
        MakeStep();
        lastStepTime_ = currentTime;
        
        // Check if we completed movement to current beam position
        if (HasReachedTargetPosition()) {
            
            // Turn on laser and check sensor
            SetLaserState(true);
            
            // Mark that we're at a beam position for sensor reading
            atBeamPosition_ = true;
            beamCheckStartTime_ = currentTime;
            
            // Calculate next target position
            CalculateNextBeamPosition();
        }
        
        // If we've been at beam position long enough, move to next
        if (atBeamPosition_ && (currentTime - beamCheckStartTime_) >= BEAM_CHECK_TIME_MS * 1000) { // Convert ms to us
            SetLaserState(false);
            atBeamPosition_ = false;
            currentBeamIndex_ = GetCurrentBeamIndex();
        }
    }
}

// Make a single stepper motor step (non-blocking)
void LaserBeamManager::MakeStep() {
    // Set direction
    dirPin_.Write(scanDirection_ > 0);
    
    // Generate step pulse
    stepPin_.Write(true);
    daisy::System::DelayUs(PULSE_WIDTH_US); // 50us pulse width
    stepPin_.Write(false);
    
    // Update position
    currentStepPosition_ += scanDirection_;
}

// Calculate next beam position (replaces Arduino direction logic)
void LaserBeamManager::CalculateNextBeamPosition() {
    int beamsPerDirection = config_ ? config_->GetConfig()->numBeams : DEFAULT_BEAMS;
    
    if (scanDirection_ > 0) {
        // Moving forward
        if (currentBeamIndex_ >= (beamsPerDirection - 1)) {
            // Reached end, reverse direction
            scanDirection_ = -1;
            currentBeamIndex_ = beamsPerDirection - 1;
        } else {
            currentBeamIndex_++;
        }
    } else {
        // Moving backward  
        if (currentBeamIndex_ <= 0) {
            // Reached start, reverse direction
            scanDirection_ = 1;
            currentBeamIndex_ = 0;
        } else {
            currentBeamIndex_--;
        }
    }
    
    // Calculate target step position for this beam
    targetStepPosition_ = currentBeamIndex_ * stepsPerBeam_;
}

// Check if motor reached target position
bool LaserBeamManager::HasReachedTargetPosition() {
    return abs(currentStepPosition_ - targetStepPosition_) <= 1;
}

// Get next beam index in sequence
uint8_t LaserBeamManager::GetCurrentBeamIndex() {
    return currentBeamIndex_;
}

// Read sensors (improved version of Arduino analogRead)
void LaserBeamManager::ReadSensors() {
    // Read ADC value (0-1 float, equivalent to Arduino 0-1023)
    float rawValue = hardware_->adc.GetFloat(0);
    
    // Convert to Arduino-equivalent scale (0-1023)
    sensorValues_[currentBeamIndex_] = rawValue * 1023.0f;
}

// Apply filtering to reduce noise
void LaserBeamManager::FilterSensorValues() {
    if (atBeamPosition_ && laserState_) {
        // Only process when laser is on and we're at beam position
        ApplyLowPassFilter(&filteredValues_[currentBeamIndex_], 
                          sensorValues_[currentBeamIndex_], 
                          FILTER_ALPHA);
    }
}

// Process beam state changes (improved Arduino if-else logic)
void LaserBeamManager::ProcessBeamStates() {
    if (!atBeamPosition_ || !laserState_) {
        return; // Only check when laser is on and positioned
    }
    
    uint8_t beamIndex = currentBeamIndex_;
    float sensorValue = filteredValues_[beamIndex];
    uint16_t threshold = thresholds_[beamIndex];
    
    // Determine beam state (Arduino: analogVal <= 700)
    bool beamBroken = (sensorValue <= threshold);
    bool previousState = beamStates_[beamIndex];
    
    // Check for state change with debouncing
    uint32_t currentTime = daisy::System::GetNow();
    if (beamBroken != previousState) {
        if ((currentTime - lastStateChange_[beamIndex]) > DEBOUNCE_TIME_MS * 1000) { // Convert ms to us
            // State change confirmed
            beamStates_[beamIndex] = beamBroken;
            lastStateChange_[beamIndex] = currentTime;
            
            // Generate event
            if (beamBroken && !previousState) {
                // Beam just broken
                uint8_t velocity = CalculateVelocity(sensorValue, beamIndex);
                QueueEvent(BEAM_BROKEN, beamIndex, velocity, sensorValue);
            } else if (!beamBroken && previousState) {
                // Beam restored
                QueueEvent(BEAM_RESTORED, beamIndex, 0, sensorValue);
            }
        }
    }
}

// Load configuration parameters
void LaserBeamManager::LoadConfigurationParameters() {
    if (!config_) return;
    
    LaserHarpConfig* cfg = config_->GetConfig();
    
    // Update number of beams
    if (cfg->numBeams > 0 && cfg->numBeams <= 16) {
        // Recalculate steps per beam
        stepsPerBeam_ = STEPS_PER_REVOLUTION / cfg->numBeams; // Maintain Arduino logic
    }
    
    // Update thresholds if configured
    for (int i = 0; i < cfg->numBeams; i++) {
        if (cfg->sensorThresholds[i] > 0) {
            thresholds_[i] = cfg->sensorThresholds[i];
        }
    }
}

// Improved velocity calculation
uint8_t LaserBeamManager::CalculateVelocity(float analogValue, uint8_t beamIndex) {
    float threshold = (float)thresholds_[beamIndex];
    
    // How far below threshold (deeper break = higher velocity)
    float breakDepth = (threshold - analogValue) / threshold;
    breakDepth = fmaxf(0.0f, fminf(1.0f, breakDepth)); // Clamp 0-1
    
    // Apply curve for musical response
    float velocity = breakDepth * breakDepth; // Square for exponential feel
    
    // Convert to MIDI velocity (1-127)
    uint8_t midiVel = (uint8_t)(velocity * 126.0f) + 1;
    return midiVel;
}

// Queue event (thread-safe)
void LaserBeamManager::QueueEvent(BeamEventType type, uint8_t beam, uint8_t velocity, float analogValue) {
    if (IsEventQueueFull()) {
        return; // Drop event if queue full
    }
    
    // Create event
    BeamEvent event;
    event.type = type;
    event.beamIndex = beam;
    event.velocity = velocity;
    event.timestamp = daisy::System::GetNow();
    event.analogValue = analogValue;
    
    // Add to queue
    eventQueue_[eventQueueTail_] = event;
    eventQueueTail_ = (eventQueueTail_ + 1) % EVENT_QUEUE_SIZE;
    eventQueueCount_++;
}

// Apply low pass filter (anti-noise)
void LaserBeamManager::ApplyLowPassFilter(float* value, float newValue, float alpha) {
    *value = alpha * newValue + (1.0f - alpha) * (*value);
}

// Check if event queue is full
bool LaserBeamManager::IsEventQueueFull() {
    return eventQueueCount_ >= EVENT_QUEUE_SIZE;
}

void LaserBeamManager::CalculateServoPosition() {
    // Already handled by CalculateNextBeamPosition()
}

bool LaserBeamManager::MoveServoToPosition(float targetAngle) {
    // For stepper motor, position is discrete steps
    return true;
}

void LaserBeamManager::DetectBeamEvents() {
    // Event detection is handled in ProcessBeamStates()
}

void LaserBeamManager::QueueEvent(BeamEventType type, uint8_t beam, uint8_t velocity) {
    QueueEvent(type, beam, velocity, 0.0f);
}

void LaserBeamManager::ProcessCalibration() {
    // TODO: Implement calibration processing
    if (!isCalibrating_) return;
    
    // Store calibration samples for current beam
    if (calibrationSampleCount_ < 100) {
        calibrationValues_[calibrationBeam_][calibrationSampleCount_] = sensorValues_[calibrationBeam_];
        calibrationSampleCount_++;
    } else {
        // Calculate threshold for this beam
        CalculateThresholds();
        
        // Move to next beam
        calibrationBeam_++;
        calibrationSampleCount_ = 0;
        
        if (calibrationBeam_ >= GetActiveBeamCount()) {
            // Calibration complete
            EndCalibration();
        }
    }
}

void LaserBeamManager::CalculateThresholds() {
    // Calculate average and set threshold
    float sum = 0.0f;
    for (int i = 0; i < calibrationSampleCount_; i++) {
        sum += calibrationValues_[calibrationBeam_][i];
    }
    float average = sum / calibrationSampleCount_;
    
    // Set threshold slightly below average (85% of average)
    thresholds_[calibrationBeam_] = (uint16_t)(average * 0.85f);
}

void LaserBeamManager::ResetCalibrationData() {
    calibrationBeam_ = 0;
    calibrationSampleCount_ = 0;
}

float LaserBeamManager::MapAngleToBeam(float angle, uint8_t numBeams) {
    // Map angle (0-360) to beam index
    return (angle / 360.0f) * numBeams;
}