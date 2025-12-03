#ifndef PROJECT_STATE_H
#define PROJECT_STATE_H

// This enum is the single source of truth for the entire project's state.
// Any component that needs to know the current state (Screen, LEDs, Servos)
// will include this file.
enum class SystemState {
  WAKE_UP,
  SCANNING,
  DETECTION,
  NAPPING,
  FULL_ASLEEP,
  ERROR
};

#endif // PROJECT_STATE_H