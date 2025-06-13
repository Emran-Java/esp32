// Smart Cage Project using ESP32 and FreeRTOS
// Author: Bionic Orbit

#include <Arduino.h>
//#include <WiFi.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
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

#include <math.h>

//TaskHandle_t scanInputDataSendToFirebaseTaskHandle = NULL;
TaskHandle_t distanceFunTaskHandle = NULL, mq2FunTaskHandle = NULL, lightFunTaskHandle = NULL;
// TaskHandle_t gasFunTaskHandle = NULL;
// TaskHandle_t relayFunTaskHandle = NULL;
// TaskHandle_t readDbStageFunTaskHandle = NULL;

const long gmtOffset_sec = 6 * 3600;  // GMT+6
const int daylightOffset_sec = 0;

//#define WIFI_SSID "EMRAN"
//#define WIFI_PASSWORD "44332211"

#define WIFI_SSID "EMRAN"
#define WIFI_PASSWORD "44332211"

#define API_KEY "AIzaSyBU3igkoS3L7YgymYQE9Wb9qID95UJ7yjY"
#define DATABASE_URL "https://power-plug-iot-default-rtdb.asia-southeast1.firebasedatabase.app/"

String DEVICE_ID = "0";
String ROW_ID = "/" + DEVICE_ID;
String ROOT = "/pet_cage" + ROW_ID;
String STREAM_PATH = ROOT + "/modules";

FirebaseData fbdo;
FirebaseData stream;

FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

unsigned long sendDataPrevMillis = 0;
const long sendDataIntervalMillis = 1000;

unsigned long firebaseDataBaseScanDelay = 1000;  // default

// for input GPIO Pin Definitions
#define TRIG_PIN 5
#define ECHO_PIN 18
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

#define DHT_PIN 27
#define MQ2_PIN 34
#define TEMP_SENSOR_PIN 33
#define LIGHT_SENSOR_PIN 25
//-------------------------

// output
#define SERVO_DOOR 13
#define SERVO_FOOD_1 14
#define SERVO_FOOD_2 15

#define RELAY_AIR_EXIT_FAN 19
#define RELAY_INDOOR_FAN 21
#define RELAY_WATER_PUMP_1 22
#define RELAY_WATER_PUMP_2 23

#define RELAY_LIGHT_1 12
#define RELAY_ROOM_HEATER 2
//-------------------------


long duration;
float distanceCm = 0.0;
//float distanceInch;
int gasData = 0;

String isWaterPump_1_On = "0", isWaterPump_2_On = "0";
String isIndoreFan_On = "0", isAireExit_On = "0";
String isLight1_on = "0", isRoomHeater_on = "0";
bool isListenDbchange = true;

bool isChabgeServo1 = false, isChangeServo2 = false, isChangeServoDoor = false;
String servoDoorLockerStage = "0", schedulDoorServo = "5";
String servoFeedLocker1Stage = "0", schedulServo1 = "2";
String servoFeedLocker2Stage = "0", schedulServo2 = "2";
;  // 0 = close; 90 = open
int servoDoorClose = 0, servoFood1Close = 180, servoFood2Close = 30;




DHT dht(DHT_PIN, DHT11);
Servo servoDoor, servoFood1, servoFood2;
//SemaphoreHandle_t firebaseMutex;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(TEMP_SENSOR_PIN);
// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

#define FIREBASE_PATH_INPUT_TO_DB_DELAY ROOT + "/config/firebaseScanDelay"

#define FIREBASE_PATH_LDR ROOT + "/sensors/0/stage"
#define FIREBASE_PATH_MQ2_GAS ROOT + "/sensors/1/stage"
#define FIREBASE_PATH_DTH11 ROOT + "/sensors/2/stage"
#define FIREBASE_PATH_WATER_LEVEL ROOT + "/sensors/3/stage"
#define FIREBASE_PATH_TEMPETATURE ROOT + "/sensors/4/stage"

#define FIREBASE_PATH_AIRE_EXIT_FAN ROOT + "/modules/0/stage"
#define FIREBASE_PATH_INDOOR_FAN ROOT + "/modules/1/stage"
#define FIREBASE_PATH_PUMP_1 ROOT + "/modules/5/stage"
#define FIREBASE_PATH_PUMP_2 ROOT + "/modules/6/stage"
#define FIREBASE_PATH_LIGHT_1 ROOT + "/modules/2/stage"
#define FIREBASE_PATH_ROOM_HEATER ROOT + "/modules/10/stage"

#define FIREBASE_PATH_SERVO_DOOR ROOT + "/modules/14/stage"
#define FIREBASE_PATH_SERVO_DOOR_SCHEDULE ROOT + "/modules/14/schedule"
#define FIREBASE_PATH_SERVO_FEED_1 ROOT + "/modules/12/stage"
#define FIREBASE_PATH_SERVO_FEED_1_SCHEDULE ROOT + "/modules/12/schedule"
#define FIREBASE_PATH_SERVO_FEED_2 ROOT + "/modules/13/stage"
#define FIREBASE_PATH_SERVO_FEED_2_SCHEDULE ROOT + "/modules/13/schedule"

void streamCallback(FirebaseStream data) {
  Serial.println("Stream Data Received");
  Serial.println("Path: " + data.dataPath());
  Serial.println("Data: " + data.stringData());

  readDbStageFun();
  relayFun();
  servoFun();
}

void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("Stream timed out, reconnecting...");
  }
}

void readDbStageFun() {
  //while (true) {
  Serial.println("------ readDbStageFun ------");

  if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_ROOM_HEATER)) {
    isRoomHeater_on = fbdo.stringData();
    Serial.println("------ isRoomHeater_on :" + isRoomHeater_on + " ------");
  }

  if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_LIGHT_1)) {
    isLight1_on = fbdo.stringData();
    Serial.println("------ isLight1_on :" + isLight1_on + " ------");
  }

  if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_AIRE_EXIT_FAN)) {
    isAireExit_On = fbdo.stringData();
    Serial.println("------ isAireExit_On :" + isAireExit_On + " ------");
  }

  if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_INDOOR_FAN)) {
    isIndoreFan_On = fbdo.stringData();
    Serial.println("------ isIndoreFan_On :" + isIndoreFan_On + " ------");
  }
  Serial.println("------ _______________ ------");



  //isWaterPump_1_On = readFirebase(FIREBASE_PATH_PUMP_1);

  if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_PUMP_1)) {

    isWaterPump_1_On = fbdo.stringData();
    Serial.println("------ isWaterPump_1_On :" + isWaterPump_1_On + " ------");
  }

  if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_PUMP_2)) {
    isWaterPump_2_On = fbdo.stringData();
    Serial.println("------ isWaterPump_2_On :" + isWaterPump_2_On + " ------");
  }
  Serial.println("------ _______________ ------");

  //scan feed store Servo
  if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_SERVO_FEED_1)) {
    servoFeedLocker1Stage = fbdo.stringData();
    isChabgeServo1 = true;
    if (Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_SERVO_FEED_1_SCHEDULE)) {
      schedulServo1 = fbdo.stringData();
    }
    Serial.println(" ~~~~~~ servoFeedLocker1Stage :" + servoFeedLocker1Stage + " ~~~~~~");
  }

  if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_SERVO_FEED_2)) {
    servoFeedLocker2Stage = fbdo.stringData();
    isChangeServo2 = true;
    if (Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_SERVO_FEED_2_SCHEDULE)) {
      schedulServo2 = fbdo.stringData();
    }
    Serial.println(" ~~~~~~ servoFeedLocker2Stage :" + servoFeedLocker2Stage + " ~~~~~~");
  }

  // scan door servo
  if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_SERVO_DOOR)) {
    servoDoorLockerStage = fbdo.stringData();
    isChangeServoDoor = true;
    if (Firebase.RTDB.getString(&fbdo, FIREBASE_PATH_SERVO_DOOR_SCHEDULE)) {
      schedulDoorServo = fbdo.stringData();
    }
    Serial.println(" ~~~~~~ servoDoorLockerStage :" + servoDoorLockerStage + " ~~~~~~");
  }

  //isWaterPump_2_On = readFirebase(FIREBASE_PATH_PUMP_2);
  //vTaskDelay(firebaseDataBaseScanDelay / portTICK_PERIOD_MS);
  //}
}

void servoFun() {
  //------------------ servo 1 ------------------
  // 180 = close door(FeedLocker1); 90 = open door
  if (servoFeedLocker1Stage.length() < 0) {
    servoFeedLocker1Stage = "180";
  }

  //closs to open
  if (isChabgeServo1) {
    isChabgeServo1 = false;
    servoFood1.write(servoFeedLocker1Stage.toInt());
    //isListenDbchange = false;
    writeFirebase(FIREBASE_PATH_SERVO_FEED_1, String(servoFood1Close));
    vTaskDelay((schedulServo1.toInt() * 500) / portTICK_PERIOD_MS);
    //isListenDbchange = true;
    servoFood1.write(servoFood1Close);
  }
  //------------------------------------------------

  //------------------ servo 2 ------------------
  // 0 = close door(FeedLocker1); 90 = open door
  if (servoFeedLocker2Stage.length() < 0) {
    servoFeedLocker2Stage = "0";
  }
  //closs to open
  if (isChangeServo2) {
    isChangeServo2 = false;
    servoFood2.write(servoFeedLocker2Stage.toInt());
    //isListenDbchange = false;
    writeFirebase(FIREBASE_PATH_SERVO_FEED_2, String(servoFood2Close));
    vTaskDelay((schedulServo2.toInt() * 1000) / portTICK_PERIOD_MS);
    //isListenDbchange = true;
    servoFood2.write(servoFood2Close);
  }
  //------------------------------------------------

  //------------------ servo door ------------------
  if (servoDoorLockerStage.length() < 0) {
    servoDoorLockerStage = "0";  //set default 0 as close
  }
  //door closs to open
  if (isChangeServoDoor) {
    isChangeServoDoor = false;
    servoDoor.write(servoDoorLockerStage.toInt());
    Serial.println("|> Servi Door " + servoDoorLockerStage + ", Shedule: " + schedulDoorServo + "<|");
    writeFirebase(FIREBASE_PATH_SERVO_DOOR, String(servoDoorClose));
    vTaskDelay((schedulDoorServo.toInt() * 1000) / portTICK_PERIOD_MS);
    servoFood2.write(servoDoorClose);
  }
  //------------------------------------------------
}

void relayFun() {
  //while (true) {
  Serial.println("Room Heater Relay " + isRoomHeater_on);
  if (isRoomHeater_on == "1") {
    //Serial.println("Room Heater Relay ON");
    digitalWrite(RELAY_ROOM_HEATER, LOW);  // ON
  } else {
    digitalWrite(RELAY_ROOM_HEATER, HIGH);  // OFF
  }

  Serial.println("Indore Light Relay " + isLight1_on);
  if (isLight1_on == "1") {
    //Serial.println("Indor Light Relay ON");
    digitalWrite(RELAY_LIGHT_1, LOW);  // ON
  } else {
    digitalWrite(RELAY_LIGHT_1, HIGH);  // OFF
  }

  Serial.println("Indore fane Relay " + isIndoreFan_On);
  if (isIndoreFan_On == "1") {
    //Serial.println("Indor fan Relay ON");
    digitalWrite(RELAY_INDOOR_FAN, LOW);  // ON
  } else {
    digitalWrite(RELAY_INDOOR_FAN, HIGH);  // OFF
  }

  Serial.println("AireExit Relay " + isAireExit_On);
  if (isAireExit_On == "1") {
    //Serial.println("Aire exit Relay ON");
    digitalWrite(RELAY_AIR_EXIT_FAN, LOW);  // ON
  } else {
    digitalWrite(RELAY_AIR_EXIT_FAN, HIGH);  // OFF
  }

  Serial.println("Water pump 1 Relay " + isWaterPump_1_On);
  if (isWaterPump_1_On == "1") {
    //Serial.println("Water pump 1 Relay ON");
    digitalWrite(RELAY_WATER_PUMP_1, LOW);  // ON
    servoFood2.write(servoFood2Close);
    servoFood1.write(servoFood1Close);
  } else {
    digitalWrite(RELAY_WATER_PUMP_1, HIGH);  // OFF
  }

  Serial.println("Water pump 2 Relay " + isWaterPump_2_On);
  if (isWaterPump_2_On == "1") {
    //Serial.println("Water pump 2 Relay ON");
    digitalWrite(RELAY_WATER_PUMP_2, LOW);  // ON
    servoFood2.write(servoFood2Close);
    servoFood1.write(servoFood1Close);
  } else {
    digitalWrite(RELAY_WATER_PUMP_2, HIGH);  // OFF
  }
  vTaskDelay(500 / portTICK_PERIOD_MS);  // Wait 1.5 seconds
  //}
}


//##### lifgt sensor #####
void lightSensor() {
  int analogValue = 0;

  double avg = 0.0;
  for (int i = 0; i < 10; i++) {
    analogValue = analogRead(LIGHT_SENSOR_PIN);
    avg = avg + analogValue;
    delay(200);
  }
  analogValue = avg / 10.0;
  Serial.print("Analog Value = ");
  Serial.print(analogValue);  // the raw analog reading
  Serial.print(" " + getLightingCondition(analogValue));
  Serial.println(" lux: " + String(getLux(analogValue)));
}

String getLightingCondition(int analogValue) {
  if (analogValue >= 3500) {
    return "Very bright (Bright sunlight)";
  } else if (analogValue >= 2000) {
    return "Bright (Bright indoor lighting)";
  } else if (analogValue >= 1000) {
    return "Moderate (Indoor dim lighting)";
  } else if (analogValue >= 300) {
    return "Dim (Low ambient light)";
  } else if (analogValue >= 50) {
    return "Dark (Almost dark)";
  } else {
    return "Very dark / Night";
  }
}

//To convert the analog reading from an LDR (Light Dependent Resistor) to illuminance in lux (lx)
float getLux(int adcVal) {

  float voltage = adcVal * (3.3 / 4095.0);  // Convert to volts, 3.3 is input voltage

  float Rldr = (3.3 * 10000.0 / voltage) - 10000.0;  // Resistance in ohms, we use 10K reg
  float lux = 500 / pow(Rldr / 1000.0, 1.4);         // Estimate Lux
  return lux;
}
//##### _______ end lifgt sensor _______ #####


/*
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
    //Serial.print("Distance: ");
    //Serial.println(distance);
    delay(5000);
  }
}
*/

void gasFun() {
  //while (true) {
  //int gasVal = analogRead(MQ2_PIN);  // Read gas value from MQ2
  // Protect Firebase access with mutex
  // if (xSemaphoreTake(firebaseMutex, portMAX_DELAY)) {
  //If readings are noisy:
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(MQ2_PIN);
    delay(100);
  }
  gasData = total / 10;
  //xSemaphoreGive(firebaseMutex);
  //}
  Serial.printf("Gas val: %d\n", gasData);
  vTaskDelay(1500 / portTICK_PERIOD_MS);  // Wait 1.5 seconds
  //}
}


void mq2Fun(void *pvParameters) {
  while (1) {
    gasFun();
  }
}

void lightFun(void *pvParameters) {
  while (1) {
    lightSensor();
  }
}

void distanceFun(void *pvParameters) {
  while (1) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(ECHO_PIN, HIGH);
    // Calculate the distance
    distanceCm = duration * SOUND_VELOCITY / 2;
    //Serial.println("Distance: " + String(distanceCm) + "cm");
    // Convert to inches
    //distanceInch = distanceCm * CM_TO_INCH;
    //writeFirebase(FIREBASE_PATH_WATER_LEVEL, String(distanceCm) + "cm");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void scanInputDataSendToFirebase(/*void *pvParameters*/) {
  //  while (1) {

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > sendDataIntervalMillis || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (readFirebase(ROOT + "/isActive")) {

      String isActive = fbdo.stringData();

      if (isActive = "1") {
        Serial.println(" === Start send sensor data to FB DB ===");
        Serial.println("");

        //DTH11
        float hum = dht.readHumidity();
        float temp = dht.readTemperature();
        String val = String(hum) + "%," + String(temp) + "c";
        Serial.println(" ('.') Environment: " + val);
        writeFirebase(FIREBASE_PATH_DTH11, val);

        //delay(5000);
        //-----------------

        //DS18B20
        sensors.requestTemperatures();
        //float temperatureC = sensors.getTempCByIndex(0);
        //float temperatureF = sensors.getTempFByIndex(0);

        String tmpVal = String(sensors.getTempCByIndex(0)) + "c";
        Serial.println("....Water Temp: " + tmpVal + "");
        writeFirebase(FIREBASE_PATH_TEMPETATURE, tmpVal);
        //-----------------

        //MQ2 Gas
        //int gasVal = analogRead(MQ2_PIN);
        Serial.println("Gas data: " + String(gasData));
        writeFirebase(FIREBASE_PATH_MQ2_GAS, String(gasData));
        //-----------------

        //Water level / Distance
        Serial.println("distance Cm: " + String(distanceCm));
        writeFirebase(FIREBASE_PATH_WATER_LEVEL, String(distanceCm) + "cm");
        //-----------------

        Serial.println("");
        vTaskDelay(firebaseDataBaseScanDelay / portTICK_PERIOD_MS);
      }
    }
  }
  //}
}
//-----------------------------------


//***********************************
//--- Firebase related functions ---
void writeFirebase(String path, String value) {
  //if (xSemaphoreTake(firebaseMutex, portMAX_DELAY)) {
  if (Firebase.ready()) {
    Firebase.RTDB.setString(&fbdo, path, value);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  //xSemaphoreGive(firebaseMutex);
  //}
}

void writeFirebaseInt(String path, int value) {
  writeFirebase(path, String(value));
}

String readFirebase(String path) {
  String result = "";
  //if (xSemaphoreTake(firebaseMutex, portMAX_DELAY)) {
  if (Firebase.ready() && Firebase.RTDB.getString(&fbdo, path)) {
    result = fbdo.stringData();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  //xSemaphoreGive(firebaseMutex);
  //}
  return result;
}

void firebaseConfigTask(void *pvParameters) {
  while (true) {
    String delayVal = "1";  //readFirebase(ROOT + "/config/firebaseScanDelay");
    if (delayVal != "") {
      firebaseDataBaseScanDelay = delayVal.toInt() * 1000;
    }
    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
}
//------------------------------------


void setupWiFiAndFirebase() {
  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // while (WiFi.status() != WL_CONNECTED) {
  //   vTaskDelay(500 / portTICK_PERIOD_MS);
  //   Serial.print(".");
  // }

  WiFiManager wm;
  bool res;
  //res = wm.autoConnect();  // auto generated AP name from chipid
  res = wm.autoConnect("bo_wifi_8");  // anonymous ap

  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } else {
    //if you get here you have connected to the WiFi
    //Serial.println("connected...with ronok :)");
    Serial.println("\nConnected to Wi-Fi");
  }

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    signupOK = true;
  } else {
    Serial.printf("Signup Error: %s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  String delayVal = readFirebase(FIREBASE_PATH_INPUT_TO_DB_DELAY);
  Serial.println("----------delayVal: " + delayVal + "------------");
  if (delayVal != "") {
    firebaseDataBaseScanDelay = delayVal.toInt() * 1000;
  }


  if (!Firebase.RTDB.beginStream(&stream, STREAM_PATH)) {
    Serial.println("Stream begin error: " + stream.errorReason());
  }
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);


  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  while (time(nullptr) < 100000) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print("~");
  }
  Serial.println("\nTime synchronized");
  delay(2000);
}

String getCurrentTime() {

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

  analogReadResolution(12);  //ESP32 ADC resolution: 0–4095
  //helping setting for MQ2
  analogSetAttenuation(ADC_11db);  //for inpur Gas data, improve ADC range/accuracy


  // Start the DTH11 sensor
  dht.begin();
  // Start the DS18B20 sensor
  sensors.begin();

  //set Servo motors for food and door lock
  servoDoor.attach(SERVO_DOOR);
  servoFood1.attach(SERVO_FOOD_1);
  servoFood2.attach(SERVO_FOOD_2);

  servoDoor.write(servoDoorClose);
  servoFood1.write(servoFood1Close);
  servoFood2.write(servoFood2Close);

  pinMode(MQ2_PIN, INPUT);
  //pinMode(LIGHT_SENSOR_PIN, INPUT);

  //pinMode(TEMP_SENSOR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_AIR_EXIT_FAN, OUTPUT);
  pinMode(RELAY_INDOOR_FAN, OUTPUT);
  pinMode(RELAY_WATER_PUMP_1, OUTPUT);
  pinMode(RELAY_WATER_PUMP_2, OUTPUT);
  pinMode(RELAY_LIGHT_1, OUTPUT);
  pinMode(RELAY_ROOM_HEATER, OUTPUT);

  //set relay default value
  digitalWrite(RELAY_WATER_PUMP_1, HIGH);
  digitalWrite(RELAY_WATER_PUMP_2, HIGH);
  digitalWrite(RELAY_AIR_EXIT_FAN, HIGH);
  digitalWrite(RELAY_INDOOR_FAN, HIGH);
  digitalWrite(RELAY_LIGHT_1, HIGH);
  digitalWrite(RELAY_ROOM_HEATER, HIGH);

  //firebaseMutex = xSemaphoreCreateMutex();
  //xSemaphoreGive(firebaseMutex);  // Give it once so it's available

  setupWiFiAndFirebase();

  xTaskCreatePinnedToCore(distanceFun, "distanceFunTaskHandle", 2024, NULL, 1, &distanceFunTaskHandle, 0);
  xTaskCreatePinnedToCore(mq2Fun, "mq2FunTaskHandle", 2024, NULL, 1, &mq2FunTaskHandle, 1);
  xTaskCreatePinnedToCore(lightFun, "lightFunTaskHandle", 2024, NULL, 1, &lightFunTaskHandle, 0);
}

void loop() {
  //readDbStageFun();
  //relayFun();

  //if (isListenDbchange) {
    Firebase.RTDB.readStream(&stream);  // Keep polling
  //}
  delay(100);  // Or run this in a FreeRTOS task

  //lightSensor();
  //distanceFun();
  scanInputDataSendToFirebase();
  // gasFun();
}