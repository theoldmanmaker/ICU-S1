// lib/XiaoFaceDetector/XiaoFaceDetector.h

#ifndef XIAO_FACE_DETECTOR_H
#define XIAO_FACE_DETECTOR_H

#include <Arduino.h>
#include <ArduinoJson.h> // The class now needs this to parse JSON

// An enumeration to easily identify the type of message received.
// This is much more efficient than comparing strings in the main loop.
enum XiaoEventType {
  NONE,           // No new message
  DETECTION,      // A face was detected
  HEARTBEAT,      // The "alive" heartbeat message
  ERROR_XIAO,     // An error reported by the XIAO
  PARSE_ERROR,    // The received data was not valid JSON
  UNKNOWN_ACTION  // Valid JSON, but the "action" was not recognized
};

// A structure to hold the data from a single parsed message.
struct XiaoMessage {
  XiaoEventType type = NONE; // The type of event
  String data = "";          // The data payload as a string
};

class XiaoFaceDetector {
public:
  XiaoFaceDetector();
  void begin();
  
  // The update function now returns a XiaoMessage structure
  XiaoMessage update();

private:
  HardwareSerial* _serial;
  // Keep a JSON document inside the class to reuse memory
  JsonDocument doc;
};

#endif // XIAO_FACE_DETECTOR_H