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

//******
// GPIO where the DS18B20 is connected to
#include <OneWire.h>
#include <DallasTemperature.h>
//------

const long gmtOffset_sec = 6 * 3600; // GMT+6
const int daylightOffset_sec = 0;

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
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701
long duration;
float distanceCm;
//float distanceInch;

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


DHT dht(DHT_PIN, DHT11);
Servo servoDoor, servoFood1, servoFood2;
SemaphoreHandle_t firebaseMutex;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(TEMP_SENSOR_PIN);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);


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

void scanInputDataSendToFirebase(/*void *pvParameters*/) {
  while (1) {
    // get distance using Ultrasonic sonsor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY/2;
  // Convert to inches
  //distanceInch = distanceCm * CM_TO_INCH;

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > sendDataIntervalMillis || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      if (Firebase.RTDB.getString(&fbdo, ROOT + "/isActive")) {

        String isActive = fbdo.stringData();

        //DTH11
        float hum = dht.readHumidity();
        float temp = dht.readTemperature();
        
        String val = String(hum) + "%," + String(temp) + "C";

        Serial.println("Environment: " + val);
        if(isActive="1"){
          writeFirebase(ROOT + "/modules/18/stage", val);
        }
        //delay(5000);
        //-----------------
        
        //DS18B20
          sensors.requestTemperatures(); 
          //float temperatureC = sensors.getTempCByIndex(0);
          //float temperatureF = sensors.getTempFByIndex(0);
          
          String tmpVal = String(sensors.getTempCByIndex(0))+"c";
          Serial.println("Water Temp: " + tmpVal);
          if(isActive="1"){
            writeFirebase(ROOT + "/modules/20/stage", tmpVal);
          }
          //-----------------

          //MQ2 Gas
          delay(500);
          int gasVal = analogRead(MQ2_PIN);
          Serial.println("Gas val: " + gasVal);
          if(isActive="1"){
            writeFirebase(ROOT + "/modules/17/stage", String(gasVal));
          }
          //-----------------

          //Water level / Distance
          Serial.println("distanceCm: " + String(distanceCm));
          if(isActive="1"){
            writeFirebase(ROOT + "/modules/19/stage", String(distanceCm)+"cm");
          }
          
          //-----------------
  
        vTaskDelay(firebaseDataBaseScanDelay / portTICK_PERIOD_MS);
      }
    }
  }
}


void firebaseTask2(void *pvParameters) {
  while (1) {
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > sendDataIntervalMillis || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      if (Firebase.RTDB.getString(&fbdo, ROOT + "/isActive")) {

     // if (readFirebase(ROOT + "/isActive")) {
        String isActivie = fbdo.stringData();

        float hum = dht.readHumidity();
        float temp = dht.readTemperature();
       
        String val = String(hum) + "%," + String(temp) + "c---c";

        Serial.println("temp_val: " + val);
        if (isActivie == "0") {
         // Firebase.RTDB.setString(&fbdo, ROOT + "/isActive", "1");
          //Firebase.RTDB.setString(&fbdo, ROOT + "/modules/18/stage", val);
         writeFirebase(ROOT + "/modules/18/stage", val);
        } else {
         // Firebase.RTDB.setString(&fbdo, ROOT + "/isActive", "0");
          //Firebase.RTDB.setString(&fbdo, ROOT + "/modules/18/stage", val);
         writeFirebase(ROOT + "/modules/18/stage", val);
        }
        //delay(5000);
        vTaskDelay(firebaseDataBaseScanDelay / portTICK_PERIOD_MS);
      }
    }
  }
}

//-----------------------------------


//***********************************
//--- Firebase related functions ---
void writeFirebase(String path, String value) {
  if (xSemaphoreTake(firebaseMutex, portMAX_DELAY)) {
    if (Firebase.ready()) {
      Firebase.RTDB.setString(&fbdo, path, value);
      delay(2000);
    }
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
      delay(1000);
    }
    xSemaphoreGive(firebaseMutex);
  }
  return result;
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
//------------------------------------



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
  Serial.println("----------delayVal: " + delayVal+"------------");
  if (delayVal != "") {
    firebaseDataBaseScanDelay = delayVal.toInt() * 1000;
  }


  configTime( gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  while (time(nullptr) < 100000) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print("~");
  }
  Serial.println("\nTime synchronized");
  delay(2000);
}


String getCurrentTime(){

  // Get current time
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);

  char buffer[20];
  sprintf(buffer, "%02d:%02d:%02d:%03d",
          timeinfo->tm_hour,
          timeinfo->tm_min,
          timeinfo->tm_sec);

  return String(buffer);
}
//------------------------------------


//***********************************
//--- System functions ---
void setup() {
  Serial.begin(115200);

  // Start the DTH11 sensor
  dht.begin();
  // Start the DS18B20 sensor
  sensors.begin();

  servoDoor.attach(SERVO_DOOR);
  servoFood1.attach(SERVO_FOOD_1);
  servoFood2.attach(SERVO_FOOD_2);

  //pinMode(MQ2_PIN, INPUT);
  //pinMode(TEMP_SENSOR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_AIR_EXIT_FAN, OUTPUT);
  pinMode(RELAY_INDOOR_FAN, OUTPUT);

  
  firebaseMutex = xSemaphoreCreateMutex();
  xSemaphoreGive(firebaseMutex);  // Give it once so it's available

  setupWiFiAndFirebase();

  scanInputDataSendToFirebase();
  
  //xTaskCreatePinnedToCore(dhtTask, "DHTTask", 3072, NULL, 1, NULL, 0);
  //xTaskCreatePinnedToCore(firebaseTask, "DHTTask", 3072, NULL, 1, NULL, 0);
  //xTaskCreate(firebaseTask2, "DHTTask", 3072, NULL, 1, NULL);

}

void loop() {
   
}
