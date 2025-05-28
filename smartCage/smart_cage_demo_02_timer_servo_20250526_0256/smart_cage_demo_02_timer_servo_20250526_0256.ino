// Smart Cage - ESP32 Code with FreeRTOS and Firebase Integration
// Author: Bionic Orbit

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>
#include <Servo.h>

#define WIFI_SSID "EMRAN"
#define WIFI_PASSWORD "44332211"
#define API_KEY "AIzaSyBU3igkoS3L7YgymYQE9Wb9qID95UJ7yjY"
#define DATABASE_URL "https://power-plug-iot-default-rtdb.asia-southeast1.firebasedatabase.app/"

String DEVICE_ID = "0";
String ROW_ID = "/" + DEVICE_ID;
String ROOT = "/pet_cage" + ROW_ID;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

// GPIO Definitions
#define ULTRASONIC_TRIG 5
#define ULTRASONIC_ECHO 18
#define SERVO_DOOR 13
#define SERVO_FOOD_1 14
#define SERVO_FOOD_2 15
#define DHTPIN 27
#define MQ2_PIN 4
#define RELAY_AIR_EXIT 19
#define RELAY_INDOOR_FAN 21
#define RELAY_WATER_PUMP_1 22
#define RELAY_WATER_PUMP_2 23

DHT dht(DHTPIN, DHT11);
Servo servoDoor, servoFood1, servoFood2;

unsigned long sendDataPrevMillis = 0;
const long sendDataIntervalMillis = 1000;

// Utility Functions
void writeFirebase(String path, String value) {
  Firebase.RTDB.setString(&fbdo, path, value);
}

void writeFirebaseInt(String path, int value) {
  Firebase.RTDB.setInt(&fbdo, path, value);
}

String readFirebase(String path) {
  if (Firebase.RTDB.getString(&fbdo, path)) {
    return fbdo.stringData();
  }
  return "";
}

void setupWiFiAndFirebase() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    signupOK = true;
  } else {
    Serial.printf("Signup Error: %s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// FreeRTOS Tasks
void doorControlTask(void *pvParameters) {
  while (true) {
    String val = readFirebase(ROOT + "/modules/14/stage");
    int smokeVal = analogRead(MQ2_PIN);

    if (val == "1" || smokeVal > 800) {
      servoDoor.write(90);
      writeFirebaseInt(ROOT + "/modules/14/stage", 1);
    } else {
      servoDoor.write(0);
    }
    delay(5000);
  }
}

void servoFood1Task(void *pvParameters) {
  while (true) {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    if ((timeinfo->tm_hour == 9 || timeinfo->tm_hour == 17) && timeinfo->tm_min == 0) {
      servoFood1.write(45);
      writeFirebaseInt(ROOT + "/modules/12/stage", 1);
      delay(2000);
      servoFood1.write(0);
      writeFirebaseInt(ROOT + "/modules/12/stage", 0);
    }
    delay(60000);
  }
}

void servoFood2Task(void *pvParameters) {
  while (true) {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    if ((timeinfo->tm_hour == 11 && timeinfo->tm_min == 58) || (timeinfo->tm_hour == 19 && timeinfo->tm_min == 30)) {
      servoFood2.write(45);
      writeFirebaseInt(ROOT + "/modules/13/stage", 1);
      delay(2000);
      servoFood2.write(0);
      writeFirebaseInt(ROOT + "/modules/13/stage", 0);
    }
    delay(60000);
  }
}

void dhtTask(void *pvParameters) {
  while (true) {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    String val = String(hum) + "%," + String(temp) + "c";
    writeFirebase(ROOT + "/modules/18/stage", val);

    if (temp < 24) {
      delay(10000);
      digitalWrite(RELAY_AIR_EXIT, LOW);
      digitalWrite(RELAY_INDOOR_FAN, HIGH);
      writeFirebaseInt(ROOT + "/modules/0/stage", 0);
      writeFirebaseInt(ROOT + "/modules/1/stage", 1);
    } else if (temp > 27) {
      delay(10000);
      digitalWrite(RELAY_AIR_EXIT, HIGH);
      digitalWrite(RELAY_INDOOR_FAN, HIGH);
      writeFirebaseInt(ROOT + "/modules/0/stage", 1);
      writeFirebaseInt(ROOT + "/modules/1/stage", 1);
    }
    delay(10000);
  }
}

void mq2Task(void *pvParameters) {
  while (true) {
    int val = analogRead(MQ2_PIN);
    writeFirebaseInt(ROOT + "/modules/17/stage", val);

    if (val > 800) {
      digitalWrite(RELAY_AIR_EXIT, HIGH);
      writeFirebaseInt(ROOT + "/modules/0/stage", 1);
    } else {
      digitalWrite(RELAY_AIR_EXIT, LOW);
      writeFirebaseInt(ROOT + "/modules/0/stage", 0);
    }
    delay(5000);
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFiAndFirebase();

  dht.begin();
  servoDoor.attach(SERVO_DOOR);
  servoFood1.attach(SERVO_FOOD_1);
  servoFood2.attach(SERVO_FOOD_2);

  pinMode(MQ2_PIN, INPUT);
  pinMode(RELAY_AIR_EXIT, OUTPUT);
  pinMode(RELAY_INDOOR_FAN, OUTPUT);

  xTaskCreatePinnedToCore(doorControlTask, "DoorTask", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(servoFood1Task, "Food1Task", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(servoFood2Task, "Food2Task", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(dhtTask, "DHTTask", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(mq2Task, "MQ2Task", 4000, NULL, 1, NULL, 1);
}

void loop() {
  // Nothing in loop, handled by FreeRTOS
}
