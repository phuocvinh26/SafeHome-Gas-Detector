#include "globals.h"
#include <DHT.h>

#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

float currentTemp = 0.0;
float currentHum = 0.0;
int currentLight = 0;
bool isSmokeDetected = false;
bool isErrorState = false;

unsigned long lastSensorRead = 0;
const unsigned long SENSOR_READ_INTERVAL = 2000;
int dhtErrorCount = 0;

void initSensors() {
    pinMode(MQ2_PIN, INPUT);
    pinMode(LDR_PIN, INPUT);
    dht.begin();
}

void readSensors() {
    unsigned long currentMillis = millis();
    // Non-blocking read every 2 seconds (DHT22 limit)
    if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
        lastSensorRead = currentMillis;

        float t = dht.readTemperature();
        float h = dht.readHumidity();
        
        if (isnan(t) || isnan(h)) {
            dhtErrorCount++;
            if (dhtErrorCount >= 3) { // Require 3 consecutive failures to enter error state
                isErrorState = true;
            }
        } else {
            dhtErrorCount = 0;
            isErrorState = false;
            currentTemp = t;
            currentHum = h;
        }

        // Map analog light sensor reading (0-4095 on ESP32) to 0-10 scale
        currentLight = map(analogRead(LDR_PIN), 0, 4095, 0, 10);
        if (currentLight < 0) currentLight = 0;
        if (currentLight > 10) currentLight = 10;

        // MQ-2: LOW means smoke detected. Only update if not preheating.
        if (!isPreheating) {
            isSmokeDetected = (digitalRead(MQ2_PIN) == LOW);
        } else {
            isSmokeDetected = false;
        }
    }
}
