#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Adafruit_PWMServoDriver.h>
#include <ProjectState.h>

class ServoController {
public:
  ServoController();
  bool begin();
  void update();
  void setState(SystemState newState);
  bool isInitialized();

private:
  // Internal action methods
  void _openEyelids(bool instantly = false);
  void _closeEyelids(bool instantly = false);
  void _moveEyeTo(int pulseX, int pulseY);

  // Animation handling
  void _handleScanningState();

  Adafruit_PWMServoDriver _pwm;
  bool _isInitialized;
  SystemState _currentState;

  // Timers for animations
  unsigned long _lastBlinkTime;
  unsigned long _nextBlinkInterval;
  
  // New timers for random eye movement
  unsigned long _lastEyeMoveTime;
  unsigned long _nextEyeMoveInterval;
};

#endif // SERVO_CONTROLLER_H