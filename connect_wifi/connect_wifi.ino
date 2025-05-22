#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  WiFiManager wm;

  bool res;
  //res = wm.autoConnect();  // auto generated AP name from chipid
  res = wm.autoConnect("bo_wifi_8");  // anonymous ap 


  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...with ronok :)");
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
