// lib/BME280Sensor/BME280Sensor.h

#ifndef BME280_SENSOR_H
#define BME280_SENSOR_H

#include <Adafruit_BME280.h>
#include <Wire.h>

class BME280Sensor {
public:
  BME280Sensor();
  
  // Initializes the sensor. Returns true on success, false on failure.
  bool begin();

  float getTemperature();
  float getHumidity();
  float getPressure();

private:
  Adafruit_BME280 _bme; // The underlying Adafruit BME280 object
  bool _sensor_found;
};

#endif