#include "ServoController.h"
#include <Wire.h>

// =================================================================
// == SERVO CALIBRATION & CONFIGURATION                          ==
// =================================================================
#define SERVO_FREQ 50
#define EYELID_CHANNEL 0
#define EYE_X_CHANNEL  1
#define EYE_Y_CHANNEL  2
#define PWM_MIN 150
#define PWM_MAX 600

const int PULSE_EYELID_OPEN   = map(2390, 0, 4095, PWM_MIN, PWM_MAX);
const int PULSE_EYELID_CLOSED = map(3500, 0, 4095, PWM_MIN, PWM_MAX);

const int PULSE_EYE_X_MIDDLE = 370;
const int PULSE_EYE_X_LEFT   = 420;
const int PULSE_EYE_X_RIGHT  = 350;

const int PULSE_EYE_Y_MIDDLE = 370;
const int PULSE_EYE_Y_UP     = 383; // <-- ADJUSTED VALUE
const int PULSE_EYE_Y_DOWN   = 315;

// Animation Timing
const int MIN_TIME_BETWEEN_BLINKS = 500;
const int MAX_TIME_BETWEEN_BLINKS = 5000;
const int MIN_TIME_BETWEEN_EYE_MOVES = 800;  // ms
const int MAX_TIME_BETWEEN_EYE_MOVES = 3000; // ms
const int BLINK_SHUT_DURATION = 190;
const int SLOW_CLOSE_SPEED_DELAY = 10;
// =================================================================

ServoController::ServoController() : _pwm(Adafruit_PWMServoDriver()) {
  _isInitialized = false;
  _currentState = SystemState::WAKE_UP;
  _lastBlinkTime = 0;
  _nextBlinkInterval = 0;
  _lastEyeMoveTime = 0;
  _nextEyeMoveInterval = 0;
}

bool ServoController::isInitialized() {
  return _isInitialized;
}

bool ServoController::begin() {
  Serial.println("ServoController: Initializing I2C for PCA9685...");
  Wire.begin(8, 9);
  
  _pwm.begin();
  _pwm.setPWMFreq(SERVO_FREQ);

  Serial.println("ServoController: Setting servos to 'Asleep' position.");
  _pwm.setPWM(EYELID_CHANNEL, 0, PULSE_EYELID_CLOSED);
  _pwm.setPWM(EYE_X_CHANNEL, 0, PULSE_EYE_X_MIDDLE);
  _pwm.setPWM(EYE_Y_CHANNEL, 0, PULSE_EYE_Y_DOWN);

  _isInitialized = true;
  return true;
}

void ServoController::setState(SystemState newState) {
  if (!_isInitialized) return;


  _currentState = newState;
  Serial.print("ServoController: New State -> ");
  Serial.println((int)newState);

  switch (_currentState) {
    case SystemState::WAKE_UP:
      _openEyelids();
      _moveEyeTo(PULSE_EYE_X_MIDDLE, PULSE_EYE_Y_MIDDLE);
      break;

    case SystemState::SCANNING:
      // Initialize timers for both blinking and eye movement
      _lastBlinkTime = millis();
      _nextBlinkInterval = random(MIN_TIME_BETWEEN_BLINKS, MAX_TIME_BETWEEN_BLINKS);
      _lastEyeMoveTime = millis();
      _nextEyeMoveInterval = random(MIN_TIME_BETWEEN_EYE_MOVES, MAX_TIME_BETWEEN_EYE_MOVES);
      break;
      
    case SystemState::DETECTION:
      // "Stare down" action: Eyelids snap open, eye locks to center.
      _openEyelids(true); 
      _moveEyeTo(PULSE_EYE_X_MIDDLE, PULSE_EYE_Y_MIDDLE);
      break;

    case SystemState::NAPPING:
    case SystemState::FULL_ASLEEP:
      _closeEyelids();
      _moveEyeTo(PULSE_EYE_X_MIDDLE, PULSE_EYE_Y_DOWN);
      break;
    
    default:
      break;
  }
}

void ServoController::update() {
  if (!_isInitialized) return;

  

  // The update loop is for continuous actions within a state
  switch (_currentState) {
    case SystemState::SCANNING:
      _handleScanningState();
      break;
    default:
      break;
  }
}

void ServoController::_openEyelids(bool instantly) {
  if (instantly) {
    _pwm.setPWM(EYELID_CHANNEL, 0, PULSE_EYELID_OPEN); return;
  }
  for (int p = PULSE_EYELID_CLOSED; p >= PULSE_EYELID_OPEN; p--) {
    _pwm.setPWM(EYELID_CHANNEL, 0, p); delay(SLOW_CLOSE_SPEED_DELAY / 2);
  }
}

void ServoController::_closeEyelids(bool instantly) {
  if (instantly) {
    _pwm.setPWM(EYELID_CHANNEL, 0, PULSE_EYELID_CLOSED); return;
  }
  for (int p = PULSE_EYELID_OPEN; p <= PULSE_EYELID_CLOSED; p++) {
    _pwm.setPWM(EYELID_CHANNEL, 0, p); delay(SLOW_CLOSE_SPEED_DELAY);
  }
}

void ServoController::_moveEyeTo(int pulseX, int pulseY) {
  _pwm.setPWM(EYE_X_CHANNEL, 0, pulseX);
  _pwm.setPWM(EYE_Y_CHANNEL, 0, pulseY);
}

void ServoController::_handleScanningState() {
  // Check for blinking
  if (millis() - _lastBlinkTime >= _nextBlinkInterval) {
    _pwm.setPWM(EYELID_CHANNEL, 0, PULSE_EYELID_CLOSED);
    delay(BLINK_SHUT_DURATION);
    _pwm.setPWM(EYELID_CHANNEL, 0, PULSE_EYELID_OPEN);
    
    _lastBlinkTime = millis();
    _nextBlinkInterval = random(MIN_TIME_BETWEEN_BLINKS, MAX_TIME_BETWEEN_BLINKS);
  }

  // Check for random eye movement
  if (millis() - _lastEyeMoveTime >= _nextEyeMoveInterval) {
    //Serial.println("ServoController: Moving eye to new random position.");
    // Pick a new random target within the calibrated safe zones
    int newX = random(PULSE_EYE_X_RIGHT, PULSE_EYE_X_LEFT);
    int newY = random(PULSE_EYE_Y_DOWN, PULSE_EYE_Y_UP);
    _moveEyeTo(newX, newY);

    _lastEyeMoveTime = millis();
    _nextEyeMoveInterval = random(MIN_TIME_BETWEEN_EYE_MOVES, MAX_TIME_BETWEEN_EYE_MOVES);
  }
}