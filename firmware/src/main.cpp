#include "globals.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <Wire.h>
#include <dns.h>

#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Global Variables implementation
float temp_warning = 30.0;
float temp_danger = 35.0;
float hum_warning = 70.0;
float hum_danger = 80.0;

int statusLevel = 1;
bool isPreheating = true;
bool isOffline = true;
bool lastSyncedValveState = false;

unsigned long startMillis = 0;
unsigned long lastFirebaseUpdate = 0;
const unsigned long FIREBASE_INTERVAL = 5000;

unsigned long lastOledUpdate = 0;
const unsigned long OLED_INTERVAL = 500;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// The CORE logic from the "Room Monitoring Hub"
int evaluateStatusLevel() {
  if (currentTemp >= temp_danger || currentHum >= hum_danger) {
    return 3;
  } else if (currentTemp >= temp_warning || currentHum >= hum_warning) {
    return 2;
  }
  return 1;
}

void updateOLED() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastOledUpdate < OLED_INTERVAL)
    return;
  lastOledUpdate = currentMillis;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.printf("Temperature: %.1fC", currentTemp);

  display.setCursor(0, 10);
  display.printf("Humidity: %.1f%%", currentHum);

  display.setCursor(0, 20);
  display.printf("Wifi: %s", isOffline ? "Disconnected" : "Connected");

  display.setCursor(0, 30);
  display.printf("Light: %d", currentLight);

  display.setCursor(0, 40);
  display.printf("Gas: %s", isValveOpen ? "Open" : "Closed");

  display.display();
}

void initFirebase() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    isOffline = false;
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    // Đăng nhập ẩn danh (Anonymous Auth)
    if (Firebase.signUp(&config, &auth, "", "")) {
      Serial.println("Firebase Auth Successful");
    } else {
      Serial.printf("Firebase Auth Failed: %s\n",
                    config.signer.signupError.message.c_str());
    }

    // Gắn callback để handle token generation từ thư viện
    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
  } else {
    isOffline = true;
  }
}

void handleFirebase() {
  if (WiFi.status() != WL_CONNECTED) {
    isOffline = true;
    return;
  }
  isOffline = false;

  unsigned long currentMillis = millis();

  if (currentMillis - lastFirebaseUpdate >= FIREBASE_INTERVAL) {
    lastFirebaseUpdate = currentMillis;

    // 0. Uu tien dong bo trang thai nut cung len Firebase truoc!
    if (isValveOpen != lastSyncedValveState) {
      if (Firebase.RTDB.setBool(&fbdo, "/roomhub/valve_open", isValveOpen)) {
        lastSyncedValveState = isValveOpen;
      }
    } else {
      // 1. Chi doc Firebase neu nhu khong co nut cung nao vua duoc bam
      if (Firebase.RTDB.getBool(&fbdo, "/roomhub/valve_open")) {
        if (fbdo.dataType() == "boolean") {
          bool remoteValveOpen = fbdo.boolData();

          if (remoteValveOpen && !isValveOpen) {
            if (!isSmokeDetected) {
              openValve();
              lastSyncedValveState = true;
            } else {
              // Safety: Refuse to open if gas is detected, sync back to false
              Firebase.RTDB.setBool(&fbdo, "/roomhub/valve_open", false);
              lastSyncedValveState = false;
            }
          } else if (!remoteValveOpen && isValveOpen) {
            closeValve();
            lastSyncedValveState = false;
          } else {
            lastSyncedValveState = remoteValveOpen;
          }
        }
      }
    }

    // 2. Fetch remote settings (old architecture)
    if (Firebase.RTDB.getFloat(&fbdo, "/roomhub/settings/temp_warning"))
      temp_warning = fbdo.floatData();
    if (Firebase.RTDB.getFloat(&fbdo, "/roomhub/settings/temp_danger"))
      temp_danger = fbdo.floatData();
    if (Firebase.RTDB.getFloat(&fbdo, "/roomhub/settings/hum_warning"))
      hum_warning = fbdo.floatData();
    if (Firebase.RTDB.getFloat(&fbdo, "/roomhub/settings/hum_danger"))
      hum_danger = fbdo.floatData();

    // 3. Upload Latest JSON state
    FirebaseJson json;
    json.add("temperature", currentTemp);
    json.add("humidity", currentHum);
    json.add("smoke_detected", isSmokeDetected);
    json.add("light_level", currentLight);
    json.add("status_level", statusLevel);
    json.add("valve_open", isValveOpen);

    Firebase.RTDB.setJSON(&fbdo, "/roomhub/latest", &json);
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.display();

  initSensors();
  initActuators();
  initFirebase();

  startMillis = millis();
}

void loop() {
  if (isPreheating && (millis() - startMillis >= PREHEAT_TIME_MS)) {
    isPreheating = false;
  }

  readSensors();
  statusLevel = evaluateStatusLevel(); // Core logic call
  updateOLED();
  handleActuators();
  handleFirebase();
}