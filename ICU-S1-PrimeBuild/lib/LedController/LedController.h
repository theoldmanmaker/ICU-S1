#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <ProjectState.h>

class LedController {
public:
  LedController();
  bool begin();
  void update();
  void setState(SystemState newState);
  bool isInitialized();

private:
  // This is the simple, blocking pulse sender. It is safe because of our startup sequence.
  void _sendCommand(int commandNum);
  
  bool _isInitialized;
};

#endif // LED_CONTROLLER_H