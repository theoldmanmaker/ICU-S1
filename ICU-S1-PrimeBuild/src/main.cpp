// PlatformIO Project: TinyS3_Controller
// src/main.cpp - Final Refactored Main Sketch

#include <Arduino.h>
#include "XiaoFaceDetector.h" // Our clean, refactored library

// Create a global instance of our face detector
XiaoFaceDetector faceDetector;

void setup() {
  Serial.begin(115200);
  long startTime = millis();
  while (!Serial && millis() - startTime < 4000);

  Serial.println("\n--- TINYS3 Main Controller ---");
  
  // Initialize our detector library. All the UART setup is hidden inside.
  faceDetector.begin();
  
  Serial.println("System Initialized. Waiting for events from XIAO...");
}

void loop() {
  // Ask the library if there's a new message
  XiaoMessage msg = faceDetector.update();

  // If the message type is not NONE, then we received something new
  if (msg.type != NONE) {
    
    // Use a switch statement to cleanly handle the different event types
    switch (msg.type) {
      
      case DETECTION:
        Serial.print("EVENT: Face Detected -> Bounding Box: ");
        Serial.println(msg.data);
        //
        // <<< THIS IS WHERE YOUR ACTION LOGIC GOES >>>
        // Example: parse msg.data and move servos, change LEDs, etc.
        //
        break;

      case HEARTBEAT:
        Serial.print("EVENT: Heartbeat Received -> ");
        Serial.println(msg.data);
        break;

      case ERROR_XIAO:
        Serial.print("EVENT: Error from XIAO -> ");
        Serial.println(msg.data);
        break;

      case PARSE_ERROR:
        Serial.print("WARNING: Received unparseable data -> ");
        Serial.println(msg.data);
        break;

      case UNKNOWN_ACTION:
        Serial.print("WARNING: Received unknown action type in JSON");
        break;
    }
  }
}