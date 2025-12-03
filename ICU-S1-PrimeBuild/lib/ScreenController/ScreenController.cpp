#include "ScreenController.h"

// --- PIN DEFINITIONS ---
#define TFT_SCLK 36
#define TFT_MOSI 35
#define TFT_CS   1
#define TFT_DC   10
#define TFT_RST  14

// --- TIMING CONSTANTS ---
const unsigned long WAKE_UP_DURATION = 3000;
const unsigned long DETECTION_DURATION = 5000;
const unsigned long DETECTION_TEXT_DURATION = 5000;
const unsigned long NAPPING_DURATION = 10000;
const unsigned long FULLSLEEP_DURATION = 10000;
const unsigned long SCANNING_TEXT_DURATION = 5000;
const unsigned long SCANNING_ANIM_DURATION = 10000;
int outterRingWidth = 10;

// --- MESSAGES ---
const char *scanningMessages[] = {
    "..SCANNING..", "WHERE ARE\nTHE HUMANS?", "COME OUT\nCOME OUT...", "I SEE YOU...", "DON'T BE SHY"};
const int numScanningMessages = sizeof(scanningMessages) / sizeof(char *);
const char *detectionMessages[] = {
    "GOTCHA!", "FREEZE\nHUMAN!", "I GOT YOU\nDIRTBAG!", "YOU ARE\nTERMINATED!", "TIME TO BBQ\nSOME MEAT!"};
const int numDetectionMessages = sizeof(detectionMessages) / sizeof(char *);

// Helper function for centering text
int16_t getCenterX(Arduino_GFX *gfx, const char *text, uint8_t size) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->setTextSize(size);
  gfx->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return (gfx->width() - w) / 2;
}

// Constructor
ScreenController::ScreenController() {
  _bus = nullptr;
  _gfx = nullptr;
  _isInitialized = false; // Set to false on creation
  _currentState = SystemState::WAKE_UP;
  _isWakeUpCompleteFlag = false;
  _stateEnterTime = 0;
  _scanningSubState = DisplaySubState::SHOWING_TEXT;
  _scanningLastSubStateChangeTime = 0;
  _scanningMessageIndex = 0;
  _detectionSubState = DisplaySubState::SHOWING_TEXT;
  _detectionLastSubStateChangeTime = 0;
  _detectionMessageIndex = 0;
}

// begin() method
bool ScreenController::begin() {
  _bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI);
  _gfx = new Arduino_GC9A01(_bus, TFT_RST);


  Serial.println("ScreenController initializing GFX...");
  if (!_gfx->begin()) {
    Serial.println("FATAL: GFX initialization failed!");
    _isInitialized = false; // Ensure flag is false on failure
    return false;
  }
  _gfx->setRotation(0);
  _gfx->invertDisplay(true);
  
  delay(5000);


  Serial.println("ScreenController initialized successfully.");
  _isInitialized = true; // Set to true ONLY on success
  return true;
}

// Public isInitialized() method
bool ScreenController::isInitialized() {
  return _isInitialized;
}

// update() method with guard clause
void ScreenController::update() {
  if (!_isInitialized) {
    return; // Do nothing if begin() has not succeeded
  }
  runUpdate();
}

bool ScreenController::isWakeUpComplete() {
  return _isWakeUpCompleteFlag;
}

void ScreenController::setState(SystemState newState) {
  if (!_isInitialized) {
    return; // Don't try to change state on an uninitialized screen
  }
  _currentState = newState;
  _stateEnterTime = millis();
  _gfx->fillScreen(BLACK);

  if (newState == SystemState::WAKE_UP) {
    _isWakeUpCompleteFlag = false;
    _gfx->setTextColor(WHITE);
    _drawCenteredMultiLineText("POWERING\nUP", 120, 3);
  } else if (newState == SystemState::SCANNING) {
    _scanningMessageIndex = -1;
    _scanningSubState = DisplaySubState::SHOWING_ANIMATION;
    _scanningLastSubStateChangeTime = 0;
  } else if (newState == SystemState::DETECTION) {
    _detectionMessageIndex = -1;
    _detectionSubState = DisplaySubState::SHOWING_TEXT;
    _detectionLastSubStateChangeTime = 0;
  } else if (newState == SystemState::NAPPING) {
    _gfx->setTextColor(CYAN);
    _drawCenteredMultiLineText("ZZZzzz", 120, 3);
  } else if (newState == SystemState::FULL_ASLEEP) {
    _gfx->setTextColor(WHITE);
    _drawCenteredMultiLineText("POWERING\nDOWN...", 120, 3);
  } else if (newState == SystemState::ERROR) {
    _gfx->setTextColor(WHITE);
    _drawCenteredMultiLineText("SYSTEM\nERROR", 120, 3);
  }
}

void ScreenController::runUpdate() {
  switch (_currentState) {
    case SystemState::WAKE_UP:    _updateWakeUp();    break;
    case SystemState::SCANNING:   _updateScanning();  break;
    case SystemState::DETECTION:  _updateDetection(); break;
    case SystemState::NAPPING:    _updateNapping();   break;
    case SystemState::FULL_ASLEEP: _updateFullSleep(); break;
    case SystemState::ERROR:      _updateError("CRITICAL"); break;
  }
}

void ScreenController::_drawCenteredMultiLineText(const String &text, int y_center, uint8_t size) {
  _gfx->setTextSize(size);
  int16_t x;
  String line;
  String remaining_text = text;
  int current_line = 0;
  int line_count = 1;
  for (unsigned int i = 0; i < text.length(); i++) {
    if (text.charAt(i) == '\n') line_count++;
  }
  int16_t x1, y1;
  uint16_t w, h;
  _gfx->getTextBounds("A", 0, 0, &x1, &y1, &w, &h);
  int total_height = h * line_count + (4 * (line_count - 1));
  int start_y = y_center - (total_height / 2);

  while (remaining_text.length() > 0) {
    int newline_index = remaining_text.indexOf('\n');
    if (newline_index == -1) {
      line = remaining_text;
      remaining_text = "";
    } else {
      line = remaining_text.substring(0, newline_index);
      remaining_text = remaining_text.substring(newline_index + 1);
    }
    x = getCenterX(_gfx, line.c_str(), size);
    _gfx->setCursor(x, start_y + (current_line * (h + 4)));
    _gfx->println(line);
    current_line++;
  }
}

void ScreenController::_updateWakeUp() {
  unsigned long now = millis();
  float pulse = (sin(now / 800.0f) + 1.0f) / 2.0f;
  uint8_t greenValue = 20 + (180 * pulse);
  uint16_t pulseColor = _gfx->color565(0, greenValue, 0);
  int centerX = _gfx->width() / 2;
  int centerY = _gfx->height() / 2;
  for (int i = 0; i < outterRingWidth; i++) {
    _gfx->drawCircle(centerX, centerY, (centerX - i), pulseColor);
  }
  if (!_isWakeUpCompleteFlag && (now - _stateEnterTime > WAKE_UP_DURATION)) {
    _isWakeUpCompleteFlag = true;
  }
}

void ScreenController::_updateScanning() {
  unsigned long now = millis();
  if (_scanningSubState == DisplaySubState::SHOWING_TEXT) {
    if (now - _scanningLastSubStateChangeTime > SCANNING_TEXT_DURATION) {
      _scanningSubState = DisplaySubState::SHOWING_ANIMATION;
      _scanningLastSubStateChangeTime = now;
      _gfx->fillScreen(BLACK);
    }
  } else if (_scanningSubState == DisplaySubState::SHOWING_ANIMATION) {
    int centerX = _gfx->width() / 2;
    int centerY = _gfx->height() / 2;
    float base_radius_old = fmod((now - 50) / 20.0f, 120.0f);
    for (int i = 0; i < 3; i++) {
      float radius = fmod(base_radius_old + (i * 40.0f), 120.0f);
      _gfx->drawCircle(centerX, centerY, (int)radius, BLACK);
    }
    float base_radius_new = fmod(now / 20.0f, 120.0f);
    for (int i = 0; i < 3; i++) {
      float radius = fmod(base_radius_new + (i * 40.0f), 120.0f);
      uint8_t brightness = 255 * (1.0f - (radius / 120.0f));
      _gfx->drawCircle(centerX, centerY, (int)radius, _gfx->color565(0, 100, brightness));
    }
    if (now - _scanningLastSubStateChangeTime > SCANNING_ANIM_DURATION) {
      _scanningSubState = DisplaySubState::SHOWING_TEXT;
      _scanningLastSubStateChangeTime = now;
      _gfx->fillScreen(BLACK);
      _scanningMessageIndex = (_scanningMessageIndex + 1) % numScanningMessages;
      _gfx->setTextColor(CYAN);
      _drawCenteredMultiLineText(scanningMessages[_scanningMessageIndex], 120, 3);
    }
  }
}

void ScreenController::_updateDetection() {
  unsigned long now = millis();
  float pulse = (sin(now / 800.0f) + 1.0f) / 2.0f;
  uint8_t redValue = 20 + (180 * pulse);
  uint16_t pulseColor = _gfx->color565(redValue, 0, 0);
  int centerX = _gfx->width() / 2;
  int centerY = _gfx->height() / 2;
  for (int i = 0; i < outterRingWidth; i++) {
    _gfx->drawCircle(centerX, centerY, (centerX - i), pulseColor);
  }
  if (now - _detectionLastSubStateChangeTime > DETECTION_TEXT_DURATION) {
    _detectionLastSubStateChangeTime = now;
    _gfx->fillScreen(BLACK);
    _detectionMessageIndex = (_detectionMessageIndex + 1) % numDetectionMessages;
    _gfx->setTextColor(RED);
    _drawCenteredMultiLineText(detectionMessages[_detectionMessageIndex], 120, 3);
  }
}

void ScreenController::_updateNapping() {
  unsigned long now = millis();
  float pulse = (sin(now / 800.0f) + 1.0f) / 2.0f;
  uint8_t blueValue = 20 + (180 * pulse);
  uint16_t pulseColor = _gfx->color565(0, 0, blueValue);
  int centerX = _gfx->width() / 2;
  int centerY = _gfx->height() / 2;
  for (int i = 0; i < outterRingWidth; i++) {
    _gfx->drawCircle(centerX, centerY, (centerX - i), pulseColor);
  }
}

void ScreenController::_updateFullSleep() {
  unsigned long now = millis();
  float pulse = (sin(now / 800.0f) + 1.0f) / 2.0f;
  uint8_t blueValue = 20 + (180 * pulse);
  uint8_t greenValue = 20 + (180 * pulse);
  uint16_t pulseColor = _gfx->color565(0, greenValue, blueValue);
  int centerX = _gfx->width() / 2;
  int centerY = _gfx->height() / 2;
  for (int i = 0; i < outterRingWidth; i++) {
    _gfx->drawCircle(centerX, centerY, (centerX - i), pulseColor);
  }
}

void ScreenController::_updateError(const String &text) {
  unsigned long now = millis();
  float pulse = (sin(now / 200.0f) + 1.0f) / 2.0f;
  uint8_t redValue = 100 + (155 * pulse);
  uint16_t pulseColor = _gfx->color565(redValue, 0, 0);
  _gfx->fillScreen(pulseColor);
  _gfx->setTextColor(WHITE);
  _drawCenteredMultiLineText("ERROR\n" + text, 120, 3);
}