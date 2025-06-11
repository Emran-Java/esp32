#define LIGHT_SENSOR_PIN 25
#define MQ2_PIN 34

void readLightTask(void *pvParameters) {
  while (true) {
    int lightVal = analogRead(LIGHT_SENSOR_PIN);
    Serial.print("Light Sensor Value (A0 - GPIO 25): ");
    Serial.println(lightVal);
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay 2 seconds
  }
}

void readGasTask(void *pvParameters) {
  while (true) {
    int gasVal = analogRead(MQ2_PIN);
    Serial.print("MQ2 Gas Sensor Value (A0 - GPIO 34): ");
    Serial.println(gasVal);
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay 2 seconds
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Allow time for Serial monitor to initialize

  analogReadResolution(12); // ESP32 ADC resolution: 0–4095
  analogSetAttenuation(ADC_11db); // More accurate full range (0–3.3V)

  xTaskCreatePinnedToCore(readLightTask, "LightSensorTask", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(readGasTask, "GasSensorTask", 2048, NULL, 1, NULL, 1);
}

void loop() {
  // Nothing here; tasks run in background
}
