#pragma once
#include "daisy_seed.h"
#include "ConfigManager.h"

// Event types for beam interruptions
enum BeamEventType {
    BEAM_BROKEN,        // Beam was interrupted
    BEAM_RESTORED,      // Beam was restored
    BEAM_CALIBRATION    // Calibration event
};

// Structure for beam events
struct BeamEvent {
    BeamEventType type;     // Type of event
    uint8_t beamIndex;      // Which beam (0-15)
    uint8_t velocity;       // Velocity/intensity (0-127)
    uint32_t timestamp;     // When the event occurred
    float analogValue;      // Raw analog sensor value
};

// Servo control states
enum ServoState {
    SERVO_IDLE,
    SERVO_SCANNING,
    SERVO_CALIBRATING,
    SERVO_ERROR
};

class LaserBeamManager {
public:
    LaserBeamManager();
    ~LaserBeamManager();
    
    // Initialization
    void Init(daisy::DaisySeed* hw, ConfigManager* config);
    
    // Main update function (call in main loop)
    void Update();
    
    // Event management
    bool HasEvents();
    bool GetNextEvent(BeamEvent* event);
    void ClearEvents();
    
    // Calibration functions
    void StartCalibration();
    void EndCalibration();
    bool IsCalibrating();
    void CalibrateCurrentBeam();
    
    // Servo control
    void StartScanning();
    void StopScanning();
    void SetServoPosition(float angle);
    float GetServoPosition();
    ServoState GetServoState();
    
    // Sensor functions
    float GetSensorValue(uint8_t sensorIndex);
    bool IsBeamBroken(uint8_t beamIndex);
    void SetSensorThreshold(uint8_t sensorIndex, uint16_t threshold);
    
    // Status and diagnostics
    uint32_t GetLastUpdateTime();
    uint8_t GetActiveBeamCount();
    bool SelfTest();
    
private:
    // Hardware references
    daisy::DaisySeed* hardware_;
    ConfigManager* config_;
    
    // Servo control
    daisy::TimerHandle servo_;
    float currentServoPosition_;
    float targetServoPosition_;
    uint32_t lastServoUpdate_;
    ServoState servoState_;
    float servoDirection_;      // 1.0 or -1.0 for direction
    
    // ADC for sensor reading
    daisy::AdcChannelConfig adcConfig_[16];
    daisy::AdcHandle adc_;
    
    // Sensor data
    float sensorValues_[16];        // Current analog values
    float filteredValues_[16];      // Filtered values for stability
    bool beamStates_[16];           // Current beam states (true = beam intact)
    bool previousStates_[16];       // Previous states for edge detection
    uint32_t lastStateChange_[16];  // Timestamp of last state change
    uint16_t thresholds_[16];       // Per-beam thresholds
    
    // Event queue
    static const uint8_t EVENT_QUEUE_SIZE = 32;
    BeamEvent eventQueue_[EVENT_QUEUE_SIZE];
    uint8_t eventQueueHead_;
    uint8_t eventQueueTail_;
    uint8_t eventQueueCount_;
    
    // Calibration data
    bool isCalibrating_;
    uint8_t calibrationBeam_;
    float calibrationValues_[16][100];  // Store multiple readings per beam
    uint8_t calibrationSampleCount_;
    
    // Timing
    uint32_t lastUpdateTime_;
    uint32_t updateInterval_;
    
    // Stepper motor control (from Arduino implementation)
    daisy::GPIO stepPin_;       // Step pin for stepper motor
    daisy::GPIO dirPin_;        // Direction pin for stepper motor
    daisy::GPIO laserPin_;      // Laser control pin
    int currentStepPosition_;  // Current step position
    int targetStepPosition_;   // Target step position
    uint32_t lastStepTime_;    // Last step timestamp
    int scanDirection_;        // Scan direction (1 or -1)
    uint8_t currentBeamIndex_; // Current beam being scanned
    int stepsPerBeam_;         // Steps per beam position
    bool atBeamPosition_;      // Whether we're at a beam position
    uint32_t beamCheckStartTime_; // When we started checking this beam
    bool laserState_;          // Current laser state
    
    // Private methods
    
    // Servo control
    void UpdateServo();
    void CalculateServoPosition();
    bool MoveServoToPosition(float targetAngle);
    
    // Sensor processing
    void ReadSensors();
    void FilterSensorValues();
    void ProcessBeamStates();
    void DetectBeamEvents();
    
    // Event management
    void QueueEvent(BeamEventType type, uint8_t beam, uint8_t velocity);
    void QueueEvent(BeamEventType type, uint8_t beam, uint8_t velocity, float analogValue);
    bool IsEventQueueFull();
    
    // Calibration helpers
    void ProcessCalibration();
    void CalculateThresholds();
    void ResetCalibrationData();
    
    // Utility functions
    uint8_t CalculateVelocity(float analogValue, uint8_t beamIndex);
    float MapAngleToBeam(float angle, uint8_t numBeams);
    uint8_t GetCurrentBeamIndex();
    void ApplyLowPassFilter(float* value, float newValue, float alpha);
    
    // Hardware setup
    void InitializeServo();
    void InitializeADC();
    void InitializeGPIO();
    
    // Additional stepper motor methods
    void InitializeLaser();
    void SetLaserState(bool on);
    void UpdateScanningMotion(uint32_t currentTime);
    void UpdateCalibrationMotion(uint32_t currentTime);
    void MakeStep();
    void CalculateNextBeamPosition();
    bool HasReachedTargetPosition();
    void LoadConfigurationParameters();
};