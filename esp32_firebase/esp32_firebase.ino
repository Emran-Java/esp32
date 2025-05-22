//this demo project for test Firebase Realtime DB from ESP32
//FireBase link: https://console.firebase.google.com/u/0/project/power-plug-iot/database/power-plug-iot-default-rtdb/data
//user Id: emranhossain.rip@gmail.com
//

#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"

//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

//power-plug-iot
#define API_KEY "AIzaSyBU3igkoS3L7YgymYQE9Wb9qID95UJ7yjY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://power-plug-iot-default-rtdb.asia-southeast1.firebasedatabase.app/"
String DEVICE_ID = "0";//7=8
String ROW_ID = "/"+DEVICE_ID;
String ROOT = "/pet_cage" + ROW_ID;
String LOAD_01="/lod/0/lod";// /clnt2/0/lod/0/lod
String LOAD_02="/lod/1/lod";// /clnt2/0/lod/1/lod

// Define Firebase Data object.
FirebaseData fbdo;
FirebaseData fbdo2;

// Define firebase authentication.
FirebaseAuth auth;

// Definee firebase configuration.
FirebaseConfig config;

// Boolean variable for sign in status.
bool signupOK = false;

//======================================== Millis variable to send/store data to firebase database.
unsigned long sendDataPrevMillis = 0;
//const long sendDataIntervalMillis = 10000; //--> Sends/stores data to firebase database every 10 seconds.
const long sendDataIntervalMillis = 1000;  //--> Sends/stores data to firebase database every 10 seconds.
//========================================

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // ---
  WiFiManager wm;

  bool res;
  //res = wm.autoConnect();  // auto generated AP name from chipid
  res = wm.autoConnect("bo_esp_wifi");  // anonymous ap 


  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...with ronok :)");
  }
  //---------------------------------------------

    //setup firebase
  // Assign the api key (required).
  config.api_key = API_KEY;

  // Assign the RTDB URL (required).
  config.database_url = DATABASE_URL;

  // Sign up.
  Serial.println();
  Serial.println("---------------Sign up");
  Serial.print("Sign up new user... ");
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Serial.println("---------------");
    // Assign the callback function for the long running token generation task.
  config.token_status_callback = tokenStatusCallback;  //--> see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  //--------------

}

void loop() {
  // put your main code here, to run repeatedly:
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > sendDataIntervalMillis || sendDataPrevMillis == 0)) {
  sendDataPrevMillis = millis();
    bool isActive = false;

    if (Firebase.RTDB.getString(&fbdo, ROOT + "/isActive")) {
      if (fbdo.dataType() == "String" || fbdo.dataType() == "string") {
    
        String temp_val = fbdo.stringData();
         Serial.println("temp_val"+temp_val);
        if (temp_val == "0") {
          Firebase.RTDB.setString(&fbdo, ROOT + "/isActive","1");
          isActive = false;
        } else {
          Firebase.RTDB.setString(&fbdo, ROOT + "/isActive","0");
          isActive = true;
        }
        delay(5000);
      }
    }

  }//enf firebase if


}// end loop
