// lib/NoctuaFanController/NoctuaFanController.cpp

#include "NoctuaFanController.h"

NoctuaFanController::NoctuaFanController(uint8_t pwm_pin, uint8_t tacho_pin) {
  _pwm_pin = pwm_pin;
  _tacho_pin = tacho_pin;
}

void NoctuaFanController::begin() {
  // Configure the LEDC peripheral for PWM signal
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  // Attach the PWM channel to the physical GPIO pin
  ledcAttachPin(_pwm_pin, PWM_CHANNEL);

  // Configure the tachometer pin as an input with an internal pull-up resistor
  pinMode(_tacho_pin, INPUT_PULLUP);
  
  Serial.println("Noctua Fan Controller Initialized.");
}

void NoctuaFanController::setSpeed(uint8_t percentage) {
  // Constrain percentage to be between 0 and 100
  percentage = constrain(percentage, 0, 100);
  // Map the 0-100 percentage to a 0-255 PWM duty cycle
  uint32_t duty_cycle = map(percentage, 0, 100, 0, 255);
  // Write the duty cycle to the PWM channel
  ledcWrite(PWM_CHANNEL, duty_cycle);
}

unsigned int NoctuaFanController::getRPM() {
  // This simple method uses pulseIn to measure the time of one pulse.
  // It's blocking, but fine for this initial test.
  // Timeout is 1 second.
  unsigned long pulse_duration = pulseIn(_tacho_pin, HIGH, 1000000);

  // If pulseIn times out (returns 0), the fan has stopped.
  if (pulse_duration == 0) {
    return 0;
  }
  
  // Formula: (1 second in microseconds / pulse duration) gives frequency.
  // Multiply by 60 for minutes. Divide by 2 because fans give 2 pulses per revolution.
  return (1000000L / pulse_duration) * 60 / 2;
}