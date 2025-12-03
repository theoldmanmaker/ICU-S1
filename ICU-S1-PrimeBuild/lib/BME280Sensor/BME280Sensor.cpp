// lib/BME280Sensor/BME280Sensor.cpp

#include "BME280Sensor.h"

BME280Sensor::BME280Sensor() {
  _sensor_found = false;
}

bool BME280Sensor::begin() {
  // The Adafruit library uses the default I2C pins (GPIO 8 & 9)
  // The 0x76 is the default I2C address for most BME280 modules.
  if (!_bme.begin(0x76)) {
    Serial.println("ERROR: Could not find a valid BME280 sensor, check wiring!");
    _sensor_found = false;
    return false;
  }
  _sensor_found = true;
  Serial.println("BME280 Sensor Initialized.");
  return true;
}

float BME280Sensor::getTemperature() {
  if (!_sensor_found) return NAN; // Return Not-A-Number if sensor not found
  return _bme.readTemperature();
}

float BME280Sensor::getHumidity() {
  if (!_sensor_found) return NAN;
  return _bme.readHumidity();
}

float BME280Sensor::getPressure() {
  if (!_sensor_found) return NAN;
  return _bme.readPressure() / 100.0F; // Convert to hPa
}