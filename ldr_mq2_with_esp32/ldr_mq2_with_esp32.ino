#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager

#define LIGHT_SENSOR_PIN 35
#define MQ2_PIN 34

void readLightTask(void *pvParameters) {
  while (true) {
    int lightVal = analogRead(LIGHT_SENSOR_PIN);
    Serial.print("Light Sensor Value (A0 - GPIO 35): ");
    Serial.println(lightVal);
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay 2 seconds
  }
}

void readGasTask(void *pvParameters) {
  while (true) {
    int gasVal = analogRead(MQ2_PIN);
    Serial.print("MQ2 Gas Sensor Value (A0 - GPIO 34): ");
    String gasVal2 = String((int)analogRead(MQ2_PIN));
    Serial.println(gasVal2);
    Serial.println("");
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay 2 seconds
  }
}

void setup() {
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
    //Serial.println("connected...with ronok :)");
    Serial.println("\nConnected to Wi-Fi");
  }


  delay(1000);  // Allow time for Serial monitor to initialize



  analogReadResolution(12); // ESP32 ADC resolution: 0–4095
  analogSetAttenuation(ADC_11db); // More accurate full range (0–3.3V)

  xTaskCreatePinnedToCore(readLightTask, "LightSensorTask", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(readGasTask, "GasSensorTask", 2048, NULL, 1, NULL, 1);
}

void loop() {
  // Nothing here; tasks run in background
}
