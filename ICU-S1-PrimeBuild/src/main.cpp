#include <Arduino.h>
#include <ProjectState.h>
#include "ScreenController.h"
#include "ServoController.h"
#include "LedController.h"

// --- Global pointers to our component controllers
ScreenController *screenController = nullptr;
ServoController *servoController = nullptr;
LedController *ledController = nullptr;

// --- Timings for the Demonstration Cycle (in milliseconds) ---
const unsigned long SCAN_DURATION_1 = 20000;  // 20 seconds
const unsigned long NAP_DURATION = 8000;      // 8 seconds
const unsigned long WAKE_FROM_NAP_DURATION = 5000; // 5 seconds for wake-up animation
const unsigned long SCAN_DURATION_2 = 15000;  // 15 seconds
const unsigned long DETECT_DURATION = 7000;   // 7 seconds
const unsigned long SCAN_DURATION_3 = 20000;  // 20 seconds
const unsigned long SLEEP_DURATION = 15000;   // 15 seconds (long pause before looping)

// This new state machine manages the overall demonstration sequence
enum class DemoState {
  IDLE, // An initial state before the demo begins
  DEMO_SCAN_1,
  DEMO_NAP,
  DEMO_WAKE_FROM_NAP,
  DEMO_SCAN_2,
  DEMO_DETECT,
  DEMO_SCAN_3,
  DEMO_SLEEP
};
DemoState currentDemoState = DemoState::IDLE;

// Global timer for managing the demo sequence
unsigned long demoStateStartTime = 0;

// --- State Management (FROM YOUR WORKING CODE) ---
SystemState currentState = SystemState::WAKE_UP;
enum class WakeUpStep {
  INITIALIZE_LEDS, 
  INITIALIZE_SCREEN,
  AWAIT_SCREEN_READY,
  INITIALIZE_SERVOS,  
  COMPLETE
};
WakeUpStep currentWakeUpStep = WakeUpStep::INITIALIZE_LEDS;

// Helper function to set the state on all components at once
void setGlobalState(SystemState newState) {
  currentState = newState;
  screenController->setState(newState);
  servoController->setState(newState);
  ledController->setState(newState);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("FeatherS3 Robot - Main Control Program Initializing...");

  screenController = new ScreenController();
  servoController = new ServoController();
  ledController = new LedController();
  
  randomSeed(analogRead(A3));
  
  Serial.println("Setup complete. Entering main loop to begin staggered startup.");
}

void loop() {
  // --- Component Update Section (FROM YOUR WORKING CODE) ---
  if (ledController && ledController->isInitialized()) {
    ledController->update();
  }
  if (screenController && screenController->isInitialized()) {
    screenController->update();
  }
  if (servoController && servoController->isInitialized()) {
    servoController->update();
  }

  // --- Main State Machine Logic ---
  switch (currentState) {
    // --- THIS IS YOUR PROVEN STARTUP SEQUENCE, UNCHANGED ---
    case SystemState::WAKE_UP:
      switch (currentWakeUpStep) {

        case WakeUpStep::INITIALIZE_LEDS:
          if (ledController->begin()) {
            ledController->setState(SystemState::WAKE_UP);
            currentWakeUpStep = WakeUpStep::INITIALIZE_SCREEN;
          } else {
            currentState = SystemState::ERROR;
          }
          break;

        case WakeUpStep::INITIALIZE_SCREEN:
          if (screenController->begin()) {
            screenController->setState(SystemState::WAKE_UP);
            currentWakeUpStep = WakeUpStep::AWAIT_SCREEN_READY;
          } else { 
            currentState = SystemState::ERROR;
          }
          break;

        case WakeUpStep::AWAIT_SCREEN_READY:
          if (screenController->isWakeUpComplete()) {
            currentWakeUpStep = WakeUpStep::INITIALIZE_SERVOS;
          }
          break;

        case WakeUpStep::INITIALIZE_SERVOS:
          if (servoController->begin()) {
            Serial.println("All components online. Performing awakening sequence...");
            servoController->setState(SystemState::WAKE_UP); 
            delay(1000); // Give components time to animate
            Serial.println("Awakening complete.");
            currentWakeUpStep = WakeUpStep::COMPLETE;
            
          } else { 
            currentState = SystemState::ERROR;          
          }
          break;

        case WakeUpStep::COMPLETE:
          Serial.println("WAKE_UP state is fully complete. Transitioning to SCANNING.");
          // --- HANDOFF TO THE DEMO CYCLE ---
          currentDemoState = DemoState::DEMO_SCAN_1; // Start the demo
          demoStateStartTime = millis();             // Start the timer
          setGlobalState(SystemState::SCANNING);     // Set the first state

          break;
      }
      break;

    // --- THIS IS THE NEW DEMONSTRATION LOGIC ---
    // It only runs AFTER the WAKE_UP sequence is finished.
    case SystemState::SCANNING:
    case SystemState::NAPPING:
    case SystemState::DETECTION:
    case SystemState::FULL_ASLEEP:
      {
        unsigned long now = millis();
        switch (currentDemoState) {
          case DemoState::DEMO_SCAN_1:
            if (now - demoStateStartTime > SCAN_DURATION_1) {
              Serial.println("DEMO: Scan 1 complete. Napping...");
              currentDemoState = DemoState::DEMO_DETECT;
              demoStateStartTime = now;
              setGlobalState(SystemState::DETECTION);
            }
            break;
          // case DemoState::DEMO_NAP:
          //   if (now - demoStateStartTime > NAP_DURATION) {
          //     Serial.println("DEMO: Nap complete. Waking up...");
          //     currentDemoState = DemoState::DEMO_WAKE_FROM_NAP;
          //     demoStateStartTime = now;
          //     setGlobalState(SystemState::WAKE_UP);
          //   }
          //   break;
          
          // case DemoState::DEMO_WAKE_FROM_NAP:
          //   if (now - demoStateStartTime > WAKE_FROM_NAP_DURATION) {
          //     Serial.println("DEMO: Wake up complete. Scanning again...");
          //     currentDemoState = DemoState::DEMO_SCAN_2;
          //     demoStateStartTime = now;
          //     setGlobalState(SystemState::SCANNING);
          //   }
          //   break;

          
          // case DemoState::DEMO_SCAN_2:
          //   if (now - demoStateStartTime > SCAN_DURATION_2) {
          //     Serial.println("DEMO: Simulating detection!");
          //     currentDemoState = DemoState::DEMO_DETECT;
          //     demoStateStartTime = now;
          //     setGlobalState(SystemState::DETECTION);
          //   }
          //   break;

            //Shortcut the napping state here to go back to scanning
          
          case DemoState::DEMO_DETECT:
            if (now - demoStateStartTime > DETECT_DURATION) {
              Serial.println("DEMO: Detection hold finished. Resuming scan...");
              currentDemoState = DemoState::DEMO_SCAN_3;
              demoStateStartTime = now;
              setGlobalState(SystemState::SCANNING);
            }
            break;
          case DemoState::DEMO_SCAN_3:
            if (now - demoStateStartTime > SCAN_DURATION_3) {
              Serial.println("DEMO: Final scan complete. Going to sleep.");
              currentDemoState = DemoState::DEMO_SLEEP;
              demoStateStartTime = now;
              setGlobalState(SystemState::FULL_ASLEEP);
            }
            break;
          case DemoState::DEMO_SLEEP:
            if (now - demoStateStartTime > SLEEP_DURATION) {
              Serial.println("DEMO: Sleep period over. Restarting cycle.");
              // To restart the whole cycle, we reset the startup state machine and go to WAKE_UP
              currentWakeUpStep = WakeUpStep::INITIALIZE_LEDS; // Reset startup sequence
              setGlobalState(SystemState::WAKE_UP);
            }
            break;
        }
      }
      break;
    
    case SystemState::ERROR:
      break;
  }
}