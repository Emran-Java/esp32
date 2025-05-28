// Smart Cage Project using ESP32 and FreeRTOS
// Author: [Your Name]

#include <WiFi.h>
#include <DHT.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "EMRAN"
#define WIFI_PASSWORD "44332211"

#define API_KEY "AIzaSyBU3igkoS3L7YgymYQE9Wb9qID95UJ7yjY"
#define DATABASE_URL "https://power-plug-iot-default-rtdb.asia-southeast1.firebasedatabase.app/"

String DEVICE_ID = "0";
String ROW_ID = "/" + DEVICE_ID;
String ROOT = "/pet_cage" + ROW_ID;

FirebaseData fbdo;
FirebaseData fbdo2;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

unsigned long sendDataPrevMillis = 0;
const long sendDataIntervalMillis = 1000;

// GPIO Pin Definitions
#define TRIG_PIN 5
#define ECHO_PIN 18
#define SERVO1_PIN 13
#define SERVO2_PIN 14
#define SERVO3_PIN 15
#define DHT_PIN 27
#define MQ2_PIN 4
#define TEMP_SENSOR_PIN 33

int RELAY_PINS[9] = {19, 21, 22, 23, 25, 26, 32, 12, 2};

DHT dht(DHT_PIN, DHT11);

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void firebaseSetup() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Serial.println("\n---------------Sign up");
  Serial.print("Sign up new user... ");
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void ultrasonicTask(void *pvParameters) {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  while (1) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance = duration * 0.034 / 2;
    Serial.print("Distance: "); Serial.println(distance);

    delay(5000);
  }
}

void dhtTask(void *pvParameters) {
  dht.begin();
  while (1) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    Serial.print("DHT Temp: "); Serial.print(t);
    Serial.print(" Humidity: "); Serial.println(h);
    delay(5000);
  }
}

void mq2Task(void *pvParameters) {
  pinMode(MQ2_PIN, INPUT);
  while (1) {
    int gas = analogRead(MQ2_PIN);
    Serial.print("Gas Level: "); Serial.println(gas);
    delay(5000);
  }
}

void tempSensorTask(void *pvParameters) {
  pinMode(TEMP_SENSOR_PIN, INPUT);
  while (1) {
    int val = analogRead(TEMP_SENSOR_PIN);
    Serial.print("Analog Temp Sensor: "); Serial.println(val);
    delay(5000);
  }
}

void relayTask(void *pvParameters) {
  for (int i = 0; i < 9; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
  }
  while (1) {
    for (int i = 0; i < 9; i++) {
      digitalWrite(RELAY_PINS[i], HIGH);
      delay(500);
      digitalWrite(RELAY_PINS[i], LOW);
    }
    delay(5000);
  }
}

void firebaseTask(void *pvParameters) {
  while (1) {
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > sendDataIntervalMillis || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      if (Firebase.RTDB.getString(&fbdo, ROOT + "/isActive")) {
        String temp_val = fbdo.stringData();
        Serial.println("temp_val: " + temp_val);
        if (temp_val == "0") {
          Firebase.RTDB.setString(&fbdo, ROOT + "/isActive", "1");
        } else {
          Firebase.RTDB.setString(&fbdo, ROOT + "/isActive", "0");
        }
        delay(5000);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  firebaseSetup();

  xTaskCreate(ultrasonicTask, "Ultrasonic", 2048, NULL, 1, NULL);
  xTaskCreate(dhtTask, "DHT", 2048, NULL, 1, NULL);
  xTaskCreate(mq2Task, "MQ2", 2048, NULL, 1, NULL);
  xTaskCreate(tempSensorTask, "TempSensor", 2048, NULL, 1, NULL);
  xTaskCreate(relayTask, "Relay", 4096, NULL, 1, NULL);
  xTaskCreate(firebaseTask, "Firebase", 8192, NULL, 1, NULL);
}

void loop() {
  // FreeRTOS handles tasks
}
