// Smart Cage Project using ESP32 and FreeRTOS
// Author: [Your Name]

#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <ESP32Servo.h>
#include <time.h>

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

#define SERVO_DOOR 13
#define SERVO_FOOD_1 14
#define SERVO_FOOD_2 15

#define DHT_PIN 27
#define MQ2_PIN 4
#define TEMP_SENSOR_PIN 33

#define RELAY_AIR_EXIT_FAN 19
#define RELAY_INDOOR_FAN 21
#define RELAY_WATER_PUMP_1 22
#define RELAY_WATER_PUMP_2 23

#define RELAY_SERVO_1_FOOD_STORE 25
#define RELAY_SERVO_2_FOOD_RE_STORE 26
#define RELAY_SERVO_3_DOOR 32

#define RELAY_LIGHT_1 12
#define RELAY_ROOM_HEATER 2


// light 2
// fan 2
// heter 2



//int RELAY_PINS[9] = {19, 21, 22, 23, 25, 26, 32, 12, 2};

DHT dht(DHT_PIN, DHT11);
Servo servoDoor, servoFood1, servoFood2;
SemaphoreHandle_t firebaseMutex;

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void writeFirebase(String path, String value) {
  if (xSemaphoreTake(firebaseMutex, portMAX_DELAY)) {
    if (Firebase.ready()) Firebase.RTDB.setString(&fbdo, path, value);
    xSemaphoreGive(firebaseMutex);
  }
}

void writeFirebaseInt(String path, int value) {
  writeFirebase(path, String(value));
}

String readFirebase(String path) {
  String result = "";
  if (xSemaphoreTake(firebaseMutex, portMAX_DELAY)) {
    if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, path)) {
      result = fbdo.stringData();
    }
    xSemaphoreGive(firebaseMutex);
  }
  return result;
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

    vTaskDelay(firebaseDataBaseScanDelay / portTICK_PERIOD_MS);
  }
}


void mq2Task(void *pvParameters) {
  pinMode(MQ2_PIN, INPUT);
  while (1) {
    int gas = analogRead(MQ2_PIN);
    Serial.print("Gas Level: ");
    Serial.println(gas);
    vTaskDelay(firebaseDataBaseScanDelay / portTICK_PERIOD_MS);
  }
}

void tempSensorTask(void *pvParameters) {
  pinMode(TEMP_SENSOR_PIN, INPUT);
  while (1) {
    int val = analogRead(TEMP_SENSOR_PIN);
    Serial.print("Analog Temp Sensor: ");
    Serial.println(val);
    vTaskDelay(firebaseDataBaseScanDelay / portTICK_PERIOD_MS);
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

     // if (readFirebase(ROOT + "/isActive")) {
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
         // Firebase.RTDB.setString(&fbdo, ROOT + "/isActive", "1");
          Firebase.RTDB.setString(&fbdo, ROOT + "/modules/18/stage", val);
         //writeFirebase(ROOT + "/modules/18/stage", val);
        } else {
         // Firebase.RTDB.setString(&fbdo, ROOT + "/isActive", "0");
          Firebase.RTDB.setString(&fbdo, ROOT + "/modules/18/stage", val);
         //writeFirebase(ROOT + "/modules/18/stage", val);
        }
        //delay(5000);
        vTaskDelay(firebaseDataBaseScanDelay / portTICK_PERIOD_MS);
      }
    }
  }
}


void firebaseConfigTask(void *pvParameters) {
  while (true) {
    String delayVal = readFirebase(ROOT + "/config/firebaseScanDelay");
    if (delayVal != "") {
      firebaseDataBaseScanDelay = delayVal.toInt() * 1000;
    }
    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
}

void setupWiFiAndFirebase() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
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

  String delayVal = readFirebase(ROOT + "/config/firebaseScanDelay");
  if (delayVal != "") {
    firebaseDataBaseScanDelay = delayVal.toInt() * 1000;
  }

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  while (time(nullptr) < 100000) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print("~");
  }
  Serial.println("\nTime synchronized");
}

void setup() {
  Serial.begin(115200);
  
  dht.begin();

  servoDoor.attach(SERVO_DOOR);
  servoFood1.attach(SERVO_FOOD_1);
  servoFood2.attach(SERVO_FOOD_2);

  pinMode(MQ2_PIN, INPUT);
  pinMode(RELAY_AIR_EXIT_FAN, OUTPUT);
  pinMode(RELAY_INDOOR_FAN, OUTPUT);

  //connectWiFi();
  //firebaseSetup();

  firebaseMutex = xSemaphoreCreateMutex();
  xSemaphoreGive(firebaseMutex);  // Give it once so it's available

  setupWiFiAndFirebase();


  xTaskCreate(ultrasonicTask, "Ultrasonic", 2048, NULL, 1, NULL);
  //xTaskCreate(dhtTask, "DHT", 2048, NULL, 1, NULL);
  xTaskCreate(mq2Task, "MQ2", 2048, NULL, 1, NULL);
  xTaskCreate(tempSensorTask, "TempSensor", 2048, NULL, 1, NULL);
  
  xTaskCreate(firebaseTask, "Firebase", 8192, NULL, 1, NULL);
//  xTaskCreate(firebaseTask2, "Firebase", 8192, NULL, 1, NULL);

  //xTaskCreate(dhtTask, "DHTTask", 4000, NULL, 1, NULL);
}

void loop() {
  // FreeRTOS handles tasks
}
