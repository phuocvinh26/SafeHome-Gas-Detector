#include "esp32-hal-gpio.h"
#include "globals.h"
#include <ESP32Servo.h>

Servo gasValveServo;

int currentServoAngle = 90; // Default closed state
int targetServoAngle = 90;
unsigned long lastServoMoveTime = 0;
const unsigned long SERVO_STEP_DELAY =
    20; // 15ms per degree for smooth movement

bool isBuzzerMuted = false;
bool isValveOpen = false;

// Button state tracking for short/long press
unsigned long buttonPressTime = 0;
bool buttonStateLast = HIGH;
bool isButtonPressed = false;
bool hasTriggered1s = false;
bool hasTriggered5s = false;

// Blinking state
unsigned long lastBlinkTime = 0;
bool blinkState = false;
const unsigned long BLINK_INTERVAL = 500;

// Gas alarm sequencing
unsigned long gasAlarmStartTime = 0;
bool gasAlarmSequenceActive = false;
bool buzzerAndLedEnabled = false;

// Reject command sequencing
bool isRejectingCommand = false;
unsigned long rejectStartTime = 0;
int rejectBeepCount = 0;
unsigned long lastRejectBeepTime = 0;
bool rejectBeepState = false;

void initActuators() {
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT_OPEN_DRAIN);
  digitalWrite(BUZZER_PIN, HIGH); // Start OFF (Active LOW)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  gasValveServo.attach(SERVO_PIN);
  gasValveServo.write(currentServoAngle); // Start immediately at 90

  // Safety State: Close valve immediately on boot (90 degrees = CLOSED)
  closeValve();
}

void closeValve() {
  targetServoAngle = 90;
  isValveOpen = false;
}

void openValve() {
  targetServoAngle = 0;
  isValveOpen = true;
}

void handleServoMovement() {
  if (currentServoAngle != targetServoAngle) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastServoMoveTime >= SERVO_STEP_DELAY) {
      lastServoMoveTime = currentMillis;
      if (currentServoAngle < targetServoAngle) {
        currentServoAngle++;
      } else {
        currentServoAngle--;
      }
      gasValveServo.write(currentServoAngle);
    }
  }
}

void handleLEDsAndBuzzer() {
  unsigned long currentMillis = millis();

  // Update generic blink state
  if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = currentMillis;
    blinkState = !blinkState;
  }

  // 1. Handle Command Rejected Sequence (highest priority display override)
  if (isRejectingCommand) {
    if (rejectBeepCount >= 6 ||
        currentMillis - rejectStartTime > 2000) { // 3 full cycles = 6 toggles
      isRejectingCommand = false;
      digitalWrite(BUZZER_PIN, HIGH); // Buzzer off
    } else {
      if (currentMillis - lastRejectBeepTime >= 150) {
        lastRejectBeepTime = currentMillis;
        rejectBeepState = !rejectBeepState;
        rejectBeepCount++;
        digitalWrite(LED_RED_PIN, rejectBeepState ? HIGH : LOW);
        digitalWrite(BUZZER_PIN, rejectBeepState ? LOW : HIGH); // Active LOW
      }
      digitalWrite(LED_GREEN_PIN, LOW);
      digitalWrite(LED_YELLOW_PIN, LOW);
      return; // Skip normal LED processing while rejecting
    }
  }

  // 2. Handle Error State or Preheating
  if (isErrorState || isPreheating) {
    // Green LED blinks slowly, everything else OFF
    digitalWrite(LED_GREEN_PIN, blinkState);
    digitalWrite(LED_YELLOW_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
    // digitalWrite(BUZZER_PIN, HIGH);
    return;
  }

  // 3. Handle Gas Alarm Logic
  if (isSmokeDetected) {
    digitalWrite(LED_GREEN_PIN, LOW);

    if (!gasAlarmSequenceActive) {
      // Initiate alarm sequence
      gasAlarmSequenceActive = true;
      gasAlarmStartTime = currentMillis;
      buzzerAndLedEnabled = false;
      closeValve(); // Step 1: Immediately close valve
    }

    // Wait 200ms before activating sirens/LEDs to prevent voltage drop
    if (gasAlarmSequenceActive && (currentMillis - gasAlarmStartTime >= 200)) {
      buzzerAndLedEnabled = true;
    }

    if (buzzerAndLedEnabled) {
      if (isBuzzerMuted) {
        digitalWrite(BUZZER_PIN, HIGH);  // Mute
        digitalWrite(LED_RED_PIN, HIGH); // Solid RED
        digitalWrite(LED_YELLOW_PIN, LOW);
      } else {
        digitalWrite(BUZZER_PIN, LOW); // Active LOW
        // Alternating RED and YELLOW
        digitalWrite(LED_RED_PIN, blinkState);
        digitalWrite(LED_YELLOW_PIN, !blinkState);
      }
    } else {
      // Still in the 200ms wait period
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_RED_PIN, LOW);
      digitalWrite(LED_YELLOW_PIN, LOW);
    }
  } else {
    // Smoke cleared, normal operations
    gasAlarmSequenceActive = false;
    buzzerAndLedEnabled = false;
    isBuzzerMuted = false;
    digitalWrite(BUZZER_PIN, HIGH);

    // Status Level display (derived from Temp/Hum)
    if (statusLevel == 3) {
      digitalWrite(LED_GREEN_PIN, LOW);
      digitalWrite(LED_YELLOW_PIN, LOW);
      digitalWrite(LED_RED_PIN, blinkState);
    } else if (statusLevel == 2) {
      digitalWrite(LED_GREEN_PIN, LOW);
      digitalWrite(LED_YELLOW_PIN, isValveOpen ? HIGH : blinkState);
      digitalWrite(LED_RED_PIN, LOW);
    } else {
      // Level 1: Normal. Green Solid if open, Green Blinking if closed
      digitalWrite(LED_GREEN_PIN, isValveOpen ? HIGH : blinkState);
      digitalWrite(LED_YELLOW_PIN, LOW);
      digitalWrite(LED_RED_PIN, LOW);
    }
  }
}

void processButton() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  unsigned long currentMillis = millis();

  if (currentButtonState == LOW && buttonStateLast == HIGH) {
    // Button just pressed
    buttonPressTime = currentMillis;
    isButtonPressed = true;
    hasTriggered1s = false;
    hasTriggered5s = false;
  } else if (currentButtonState == HIGH && buttonStateLast == LOW &&
             isButtonPressed) {
    // Button released
    isButtonPressed = false;
    hasTriggered1s = false;
    hasTriggered5s = false;
  } else if (currentButtonState == LOW && isButtonPressed) {
    // Button is being held down
    unsigned long pressDuration = currentMillis - buttonPressTime;

    if (pressDuration >= 5000 && !hasTriggered5s) {
      hasTriggered5s = true;
      // Long Press (5 seconds)

      if (isSmokeDetected) {
        // Command Rejected!
        isRejectingCommand = true;
        rejectStartTime = currentMillis;
        lastRejectBeepTime = currentMillis;
        rejectBeepCount = 0;
        rejectBeepState = false;
      } else {
        // Normal state: Toggle valve
        if (isValveOpen) {
          closeValve();
        } else {
          openValve();
        }
      }
    } else if (pressDuration >= 1000 && pressDuration < 5000 &&
               !hasTriggered1s) {
      hasTriggered1s = true;
      // 1 Second Hold -> Mute
      if (isSmokeDetected) {
        isBuzzerMuted = true;
      }
    }
  }
  buttonStateLast = currentButtonState;
}

void handleActuators() {
  handleServoMovement(); // Process smooth, non-blocking servo movement
  processButton();
  handleLEDsAndBuzzer();
}
