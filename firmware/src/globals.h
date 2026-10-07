#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// --- Pin Definitions ---
#define DHT_PIN 4
#define MQ2_PIN 5
#define LDR_PIN 34
#define LED_GREEN_PIN 12
#define LED_YELLOW_PIN 13
#define LED_RED_PIN 14
#define BUZZER_PIN 27
#define BUTTON_PIN 26
#define SERVO_PIN 23

// I2C OLED
#define SDA_PIN 21
#define SCL_PIN 22

// --- Global Constants ---
#define PREHEAT_TIME_MS 30000

// --- Global Variables ---
extern float temp_warning;
extern float temp_danger;
extern float hum_warning;
extern float hum_danger;

extern float currentTemp;
extern float currentHum;
extern int currentLight;
extern bool isSmokeDetected;

extern int statusLevel; 
extern bool isValveOpen;
extern bool isPreheating;
extern bool isOffline;
extern bool isErrorState;
extern bool isBuzzerMuted;

// Functions to implement in other files
void initSensors();
void readSensors();
void initActuators();
void handleActuators();
void closeValve();
void openValve();

#endif // GLOBALS_H
