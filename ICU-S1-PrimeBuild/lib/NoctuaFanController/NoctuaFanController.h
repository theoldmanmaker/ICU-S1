// lib/NoctuaFanController/NoctuaFanController.h

#ifndef NOCTUA_FAN_CONTROLLER_H
#define NOCTUA_FAN_CONTROLLER_H

#include <Arduino.h>

class NoctuaFanController {
public:
  NoctuaFanController(uint8_t pwm_pin, uint8_t tacho_pin);
  void begin();
  
  // Speed is a percentage from 0 to 100
  void setSpeed(uint8_t percentage);

  // Returns the fan speed in Revolutions Per Minute (RPM)
  unsigned int getRPM();

private:
  uint8_t _pwm_pin;
  uint8_t _tacho_pin;
  
  // ESP32's LEDC has 16 channels. We'll use channel 0 for the fan.
  const int PWM_CHANNEL = 0; 
  const int PWM_FREQUENCY = 25000; // 25kHz is standard for 4-pin fans
  const int PWM_RESOLUTION = 8;    // 8-bit means 0-255 duty cycle
};

#endif