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

unsigned long firebaseDataBaseScanDelay = 10000;  // default

// GPIO Pin Definitions
#define TRIG_PIN 5
#define ECHO_PIN 18
#define SERVO1_PIN 13
#define SERVO2_PIN 14
#define SERVO3_PIN 15
#define DHT_PIN 27
#define MQ2_PIN 4
#define TEMP_SENSOR_PIN 33

#define RELAY_AIR_EXIT 19
#define RELAY_INDOOR_FAN 21
#define RELAY_WATER_PUMP_1 22
#define RELAY_WATER_PUMP_2 23

servomotor 3
light 2
fan 2
heter 2



//int RELAY_PINS[9] = {19, 21, 22, 23, 25, 26, 32, 12, 2};

DHT dht(DHT_PIN, DHT11);

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}


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
    Serial.print("Distance: ");
    Serial.println(distance);

    delay(5000);
  }
}

void dhtTask(void *pvParameters) {
  dht.begin();
  while (1) {
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    Serial.print("DHT Temp: ");
    Serial.print(temp);
    Serial.print(" Humidity: ");
    Serial.println(hum);

    String val = String(hum) + "%," + String(temp) + "c";
    if (Firebase.ready()) {
      //Firebase.RTDB.setString(&fbdo, ROOT + "/modules/18/stage", val);
      //Firebase.RTDB.setString(&fbdo, ROOT + "/isActive", "1");
    }
    //writeFirebase(ROOT + "/modules/18/stage", val);

    delay(firebaseDataBaseScanDelay);
  }
}
// void dhtTask(void *pvParameters) {
//   while (true) {
//     float temp = dht.readTemperature();
//     float hum = dht.readHumidity();

//     String val = String(hum) + "%," + String(temp) + "c";
//     writeFirebase(ROOT + "/modules/18/stage", val);

//     if (temp < 24) {
//       delay(10000);
//       digitalWrite(RELAY_AIR_EXIT, LOW);
//       digitalWrite(RELAY_INDOOR_FAN, HIGH);
//       writeFirebase(ROOT + "/modules/0/stage", "0");
//       writeFirebase(ROOT + "/modules/1/stage", "1");
//     } else if (temp > 27) {
//       delay(10000);
//       digitalWrite(RELAY_AIR_EXIT, HIGH);
//       digitalWrite(RELAY_INDOOR_FAN, HIGH);
//       writeFirebase(ROOT + "/modules/0/stage", "1");
//       writeFirebase(ROOT + "/modules/1/stage", "1");
//     }
//     delay(firebaseDataBaseScanDelay);
//   }
// }

void mq2Task(void *pvParameters) {
  pinMode(MQ2_PIN, INPUT);
  while (1) {
    int gas = analogRead(MQ2_PIN);
    Serial.print("Gas Level: ");
    Serial.println(gas);
    delay(5000);
  }
}

void tempSensorTask(void *pvParameters) {
  pinMode(TEMP_SENSOR_PIN, INPUT);
  while (1) {
    int val = analogRead(TEMP_SENSOR_PIN);
    Serial.print("Analog Temp Sensor: ");
    Serial.println(val);
    delay(5000);
  }
}

// void relayTask(void *pvParameters) {
//   for (int i = 0; i < 9; i++) {
//     pinMode(RELAY_PINS[i], OUTPUT);
//   }
//   while (1) {
//     for (int i = 0; i < 9; i++) {
//       digitalWrite(RELAY_PINS[i], HIGH);
//       delay(500);
//       digitalWrite(RELAY_PINS[i], LOW);
//     }
//     delay(5000);
//   }
// }

void firebaseTask(void *pvParameters) {
  while (1) {
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > sendDataIntervalMillis || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      if (Firebase.RTDB.getString(&fbdo, ROOT + "/isActive")) {
        String temp_val = fbdo.stringData();

        float hum = dht.readHumidity();
        float temp = dht.readTemperature();
        Serial.print("DHT Temp: ");
        Serial.print(temp);
        Serial.print(" Humidity: ");
        Serial.println(hum);

        String val = String(hum) + "%," + String(temp) + "c";

        Serial.println("temp_val: " + temp_val);
        if (temp_val == "0") {
          Firebase.RTDB.setString(&fbdo, ROOT + "/isActive", "1");
          Firebase.RTDB.setString(&fbdo, ROOT + "/modules/18/stage", val);
        } else {
          Firebase.RTDB.setString(&fbdo, ROOT + "/isActive", "0");
          Firebase.RTDB.setString(&fbdo, ROOT + "/modules/18/stage", val);
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
  //xTaskCreate(dhtTask, "DHT", 2048, NULL, 1, NULL);
  xTaskCreate(mq2Task, "MQ2", 2048, NULL, 1, NULL);
  xTaskCreate(tempSensorTask, "TempSensor", 2048, NULL, 1, NULL);
  //xTaskCreate(relayTask, "Relay", 4096, NULL, 1, NULL);
  xTaskCreate(firebaseTask, "Firebase", 8192, NULL, 1, NULL);

  //xTaskCreate(dhtTask, "DHTTask", 4000, NULL, 1, NULL);
}

void loop() {
  // FreeRTOS handles tasks
}
