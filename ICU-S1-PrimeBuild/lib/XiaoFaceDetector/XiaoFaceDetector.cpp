// lib/XiaoFaceDetector/XiaoFaceDetector.cpp

#include "XiaoFaceDetector.h"

// We still use IO6 as the safe pin for receiving data
#define RECEIVER_RX_PIN 6
#define RECEIVER_TX_PIN 5 // Not used, but required by begin()

XiaoFaceDetector::XiaoFaceDetector() {
  // Point our internal serial object to the hardware Serial1 port
  _serial = &Serial1;
}

void XiaoFaceDetector::begin() {
  long baud_rate = 115200;
  _serial->begin(baud_rate, SERIAL_8N1, RECEIVER_RX_PIN, RECEIVER_TX_PIN);
}

// This is the core of the new logic.
XiaoMessage XiaoFaceDetector::update() {
  XiaoMessage message; // Create a new message, its type is NONE by default

  if (_serial->available()) {
    String line = _serial->readStringUntil('\n');
    
    DeserializationError error = deserializeJson(doc, line);

    if (error) {
      // If JSON parsing fails, report it
      message.type = PARSE_ERROR;
      message.data = line; // Return the junk data for logging
    } else {
      // If JSON parsing succeeds, extract the action and data
      const char* action = doc["action"];
      const char* data_payload = doc["data"];
      message.data = data_payload; // Store the data payload

      // Convert the string "action" into our efficient enum type
      if (strcmp(action, "detection") == 0) {
        message.type = DETECTION;
      } else if (strcmp(action, "alive") == 0) {
        message.type = HEARTBEAT;
      } else if (strcmp(action, "error") == 0) {
        message.type = ERROR_XIAO;
      } else {
        message.type = UNKNOWN_ACTION;
      }
    }
  }
  
  // Return the message. If nothing was received, its type will be NONE.
  return message;
}