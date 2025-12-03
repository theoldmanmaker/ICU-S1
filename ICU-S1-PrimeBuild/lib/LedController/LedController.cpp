#include "LedController.h"
#include <Arduino.h>

// =================================================================
// == LED RING CONFIGURATION                                     ==
// =================================================================
const int SIGNAL_PIN = 6;

// Command Definitions that MUST match the ATTiny85 sketch
#define CMD_WAKE_UP 1
#define CMD_SCANNING 2
#define CMD_DETECTION 3
#define CMD_NAPPING 4
#define CMD_FULL_ASLEEP 5
#define CMD_ERROR 6

int currentLEDCommand = -1;
// Pulse Timing Configuration
const int PULSE_WIDTH_MS = 20;
const int PULSE_SPACING_MS = 50;
// =================================================================

LedController::LedController()
{
  _isInitialized = false;
}

bool LedController::isInitialized()
{
  return _isInitialized;
}



// The simple, proven, blocking pulse sender from your original test sketch.
void LedController::_sendCommand(int commandNum)
{

  return;
  
  // --- START OF CRITICAL SECTION ---
  // Disable all interrupts to guarantee precise timing for the pulse train.
  noInterrupts();

  for (int i = 0; i < commandNum; i++)
  {
    digitalWrite(SIGNAL_PIN, LOW);
    delay(PULSE_WIDTH_MS);
    digitalWrite(SIGNAL_PIN, HIGH);
    if (i < commandNum - 1)
    {
      delay(PULSE_SPACING_MS);
    }
  }

  // --- END OF CRITICAL SECTION ---
  // Re-enable interrupts so the rest of the system can function normally.
  interrupts();
}

bool LedController::begin()
{
  Serial.println("LedController: Initializing GPIO 6.");
  pinMode(SIGNAL_PIN, OUTPUT);
  digitalWrite(SIGNAL_PIN, HIGH); // Set to idle state
  _isInitialized = true;
  return true;
}

void LedController::setState(SystemState newState)
{
  if (!_isInitialized)
    return;

  Serial.print("LedController: Received new state request -> ");
  Serial.println((int)newState);

  int commandNum = 0;

  // --- THIS IS THE CRITICAL, EXPLICIT MAPPING ---
  // It translates the 0-indexed SystemState enum to the 1-indexed command protocol.
  switch (newState)
  {
  case SystemState::WAKE_UP:
    commandNum = CMD_WAKE_UP;
    break; // 0 -> 1
  case SystemState::SCANNING:
    commandNum = CMD_SCANNING;
    break; // 1 -> 2
  case SystemState::DETECTION:
    commandNum = CMD_DETECTION;
    break; // 2 -> 3
  case SystemState::NAPPING:
    commandNum = CMD_NAPPING;
    break; // 3 -> 4
  case SystemState::FULL_ASLEEP:
    commandNum = CMD_FULL_ASLEEP;
    break; // 4 -> 5
  case SystemState::ERROR:
    commandNum = CMD_ERROR;
    break; // 5 -> 6
  }

  if (currentLEDCommand == commandNum)
  {
    Serial.print("Current LED Command: ");
    Serial.print(currentLEDCommand);
    Serial.println(" and current commandNum ");
    Serial.print(commandNum);

    return;
  }

  currentLEDCommand = commandNum;

  if (commandNum > 0)
  {
    Serial.print("LedController: Mapping to command number ");
    Serial.print(commandNum);
    Serial.println(" and sending pulses...");
    _sendCommand(commandNum);
    Serial.println("LedController: Command sent.");
  }
}

// update() is not needed for this simple controller, but is kept for consistency.
void LedController::update()
{
  if (!_isInitialized)
    return;
}
