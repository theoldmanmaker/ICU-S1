// PlatformIO Project: Xiao_Face_Detector
// src/main.cpp - Final JSON Face Detection Sender with Heartbeat

#include <Arduino.h>
#include "esp_camera.h"
#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"
#include "HardwareSerial.h"
#include <ArduinoJson.h> // The JSON library

// === PIN DEFINITIONS (Verified & Correct) ===
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39
#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13

// === UART DEFINITIONS (Verified & Correct) ===
// D6 on the XIAO board is GPIO43
#define UART_TX_PIN 43
#define UART_RX_PIN 44
HardwareSerial& UartToTinyS3 = Serial1;

// --- Heartbeat Timer ---
const long HEARTBEAT_INTERVAL = 30000; // 30 seconds
unsigned long lastHeartbeatTime = 0;

// Face detection models
static HumanFaceDetectMSR01 s1(0.1F, 0.5F, 10, 0.2F);
static HumanFaceDetectMNP01 s2(0.5F, 0.3F, 5);

// Helper function to send a structured JSON message
void sendJsonMessage(const char* action, const char* data) {
  JsonDocument doc;
  doc["action"] = action;
  doc["data"] = data;
  
  // Serialize the JSON object directly to the UART port
  serializeJson(doc, UartToTinyS3);
  // serializeJson does not add a newline, so we must add it manually
  UartToTinyS3.println();
}

void setup() {
  Serial.begin(115200);
  long startTime = millis();
  while (!Serial && millis() - startTime < 4000);

  Serial.println("--- XIAO JSON Sender ---");
  UartToTinyS3.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  
  // --- Camera Initialization ---
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM; config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM; config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM; config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM; config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_240X240;
  config.pixel_format = PIXFORMAT_RGB565;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    sendJsonMessage("error", "Camera init failed");
    return;
  }
  Serial.println("Camera Initialized. Starting detection loop.");
}

void loop() {
  // --- Heartbeat Logic ---
  if (millis() - lastHeartbeatTime >= HEARTBEAT_INTERVAL) {
    sendJsonMessage("alive", "XIAO is running");
    Serial.println("Sent: Heartbeat");
    lastHeartbeatTime = millis(); // Reset the timer
  }

  // --- Face Detection Logic ---
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera frame capture failed");
    // Don't send an error every frame, that would be too much spam
    return;
  }

  std::list<dl::detect::result_t> &candidates = s1.infer((uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3});
  std::list<dl::detect::result_t> &results = s2.infer((uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3}, candidates);

  if (results.size() > 0) {
    // For simplicity, we'll only send the first detected face per frame
    auto prediction = results.begin();
    int x1 = (int)prediction->box[0]; int y1 = (int)prediction->box[1];
    int x2 = (int)prediction->box[2]; int y2 = (int)prediction->box[3];
    int w = x2 - x1; int h = y2 - y1;

    char bbox_data[20]; // Buffer to hold "x,y,w,h"
    snprintf(bbox_data, sizeof(bbox_data), "%d,%d,%d,%d", x1, y1, w, h);
    
    sendJsonMessage("detection", bbox_data);
    Serial.print("Sent Detection: ");
    Serial.println(bbox_data);
  }

  esp_camera_fb_return(fb);
}


// // PlatformIO Project: Xiao_Face_Detector
// // src/main.cpp - Final Corrected Face Detection Sender

// #include <Arduino.h>
// #include "esp_camera.h"
// #include "human_face_detect_msr01.hpp"
// #include "human_face_detect_mnp01.hpp"
// #include "HardwareSerial.h"

// // === PIN DEFINITIONS FOR XIAO ESP32S3 (Verified) ===
// #define PWDN_GPIO_NUM     -1
// #define RESET_GPIO_NUM    -1
// #define XCLK_GPIO_NUM     10
// #define SIOD_GPIO_NUM     40
// #define SIOC_GPIO_NUM     39
// #define Y9_GPIO_NUM       48
// #define Y8_GPIO_NUM       11
// #define Y7_GPIO_NUM       12
// #define Y6_GPIO_NUM       14
// #define Y5_GPIO_NUM       16
// #define Y4_GPIO_NUM       18
// #define Y3_GPIO_NUM       17
// #define Y2_GPIO_NUM       15
// #define VSYNC_GPIO_NUM    38
// #define HREF_GPIO_NUM     47
// #define PCLK_GPIO_NUM     13

// // === UART DEFINITIONS FOR XIAO (Verified & Correct) ===
// // D6 on the board is physically connected to GPIO43
// #define UART_TX_PIN 43
// // D7 on the board is physically connected to GPIO44
// #define UART_RX_PIN 44

// // Use Hardware UART engine #1 and remap its pins
// HardwareSerial MySerial(1);

// // Declare the face detection models as global objects
// static HumanFaceDetectMSR01 s1(0.1F, 0.5F, 10, 0.2F);
// static HumanFaceDetectMNP01 s2(0.5F, 0.3F, 5);


// void setup() {
//   // Start the main USB serial for debugging
//   Serial.begin(115200);
//   // Wait up to 4 seconds for the monitor to connect, so we don't miss startup messages
//   long startTime = millis();
//   while (!Serial && millis() - startTime < 4000);

//   Serial.println("XIAO Face Detector: Starting up...");

//   // Start the secondary UART port on the correct GPIO pins for D6/D7
//   MySerial.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  
//   // --- Camera Initialization ---
//   camera_config_t config;
//   config.ledc_channel = LEDC_CHANNEL_0;
//   config.ledc_timer = LEDC_TIMER_0;
//   config.pin_d0 = Y2_GPIO_NUM;
//   config.pin_d1 = Y3_GPIO_NUM;
//   config.pin_d2 = Y4_GPIO_NUM;
//   config.pin_d3 = Y5_GPIO_NUM;
//   config.pin_d4 = Y6_GPIO_NUM;
//   config.pin_d5 = Y7_GPIO_NUM;
//   config.pin_d6 = Y8_GPIO_NUM;
//   config.pin_d7 = Y9_GPIO_NUM;
//   config.pin_xclk = XCLK_GPIO_NUM;
//   config.pin_pclk = PCLK_GPIO_NUM;
//   config.pin_vsync = VSYNC_GPIO_NUM;
//   config.pin_href = HREF_GPIO_NUM;
//   config.pin_sccb_sda = SIOD_GPIO_NUM;
//   config.pin_sccb_scl = SIOC_GPIO_NUM;
//   config.pin_pwdn = PWDN_GPIO_NUM;
//   config.pin_reset = RESET_GPIO_NUM;
//   config.xclk_freq_hz = 20000000;
//   config.frame_size = FRAMESIZE_240X240;
//   config.pixel_format = PIXFORMAT_RGB565;
//   config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
//   config.fb_location = CAMERA_FB_IN_PSRAM;
//   config.jpeg_quality = 12;
//   config.fb_count = 1;

//   esp_err_t err = esp_camera_init(&config);
//   if (err != ESP_OK) {
//     Serial.printf("Camera init failed with error 0x%x", err);
//     return;
//   }

//   Serial.println("XIAO Face Detector: Camera Initialized. Starting detection loop.");
// }

// void loop() {
//   camera_fb_t *fb = esp_camera_fb_get();
//   if (!fb) {
//     Serial.println("Camera frame capture failed");
//     return;
//   }

//   // Perform face detection
//   std::list<dl::detect::result_t> &candidates = s1.infer((uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3});
//   std::list<dl::detect::result_t> &results = s2.infer((uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3}, candidates);

//   // If one or more faces are found, send their bounding boxes
//   if (results.size() > 0) {
//     for (std::list<dl::detect::result_t>::iterator prediction = results.begin(); prediction != results.end(); prediction++) {
//       int x1 = (int)prediction->box[0];
//       int y1 = (int)prediction->box[1];
//       int x2 = (int)prediction->box[2];
//       int y2 = (int)prediction->box[3];
//       int w = x2 - x1;
//       int h = y2 - y1;

//       // Format the data into a clean, comma-separated string
//       String bbox_data = String(x1) + "," + String(y1) + "," + String(w) + "," + String(h);
      
//       // Send the string over the hardware UART port (D6 / GPIO43)
//       MySerial.println(bbox_data);

//       // Print to the local USB monitor to confirm a message was sent
//       Serial.println("Sent to TinyS3: " + bbox_data);
//     }
//   }

//   // Return the frame buffer to the camera driver for reuse
//   esp_camera_fb_return(fb);
// }











// // PlatformIO Project: Xiao_Face_Detector
// // src/main.cpp - DEFINITIVE SENDER CODE

// #include <Arduino.h>
// #include "HardwareSerial.h"

// // --- Using the OFFICIAL default UART1 pins for the XIAO ESP32S3 ---
// #define XIAO_UART_TX_PIN 43  // D6 on the board, labeled TX
// #define XIAO_UART_RX_PIN 7  // D7 on the board, labeled RX

// // We will use the Serial1 object, which is tied to these default pins.
// // We don't need to create a new HardwareSerial object.
// HardwareSerial& UartToProS3 = Serial1;

// void setup() {
//   // Start the USB Serial for debugging messages
//   Serial.begin(115200);
//   while(!Serial && millis() < 4000);
//   Serial.println("XIAO: Definitive Sender is running.");

//   // Initialize Serial1 on its default pins D6 and D7
//   UartToProS3.begin(115200, SERIAL_8N1, XIAO_UART_RX_PIN, XIAO_UART_TX_PIN);
//   Serial.println("XIAO: UART communication started on default pins.");
// }

// void loop() {
//   // Send a message every single second, no matter what.
//   UartToProS3.println("XIAO_IS_ALIVE");

//   // Also print to the local monitor so we can be sure it's sending
//   Serial.println("Sent: XIAO_IS_ALIVE");
  
//   delay(1000);
// }