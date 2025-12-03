#ifndef SCREEN_CONTROLLER_H
#define SCREEN_CONTROLLER_H

#include <Arduino_GFX_Library.h>
#include "ProjectState.h"

enum class DisplaySubState {
  SHOWING_TEXT,
  SHOWING_ANIMATION
};

class ScreenController {
public:
  ScreenController();
  bool begin();
  void update();
  void setState(SystemState newState);
  bool isWakeUpComplete();
  
  // New public method to check if begin() has been run successfully
  bool isInitialized();

private:
  void runUpdate();

  Arduino_DataBus* _bus;
  Arduino_GFX* _gfx;
  
  // New private flag to track initialization status
  bool _isInitialized;

  SystemState _currentState;
  bool _isWakeUpCompleteFlag;
  unsigned long _stateEnterTime;

  DisplaySubState _scanningSubState;
  unsigned long _scanningLastSubStateChangeTime;
  int _scanningMessageIndex;

  DisplaySubState _detectionSubState;
  unsigned long _detectionLastSubStateChangeTime;
  int _detectionMessageIndex;

  void _updateWakeUp();
  void _updateScanning();
  void _updateDetection();
  void _updateNapping();
  void _updateFullSleep();
  void _updateError(const String& text);
  void _drawCenteredMultiLineText(const String& text, int y_center, uint8_t size);
};

#endif // SCREEN_CONTROLLER_H