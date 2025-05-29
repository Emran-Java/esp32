#define NUM_RELAYS 8

// Define GPIO pins connected to the relay module
const int relayPins[NUM_RELAYS] = {2, 4, 5, 18, 19, 21, 22, 23};

// Task handle for the relay task
TaskHandle_t relayTaskHandle = NULL;

void setup() {
  Serial.begin(115200);
  delay(1000);  // Allow time for Serial to open

  // Initialize relay GPIOs
  for (int i = 0; i < NUM_RELAYS; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);  // Relay OFF (assuming active LOW)
  }

  // Print heap status
  Serial.printf("Free heap before task: %u bytes\n", esp_get_free_heap_size());

  // Create relay control task (using a good starting stack size)
  xTaskCreatePinnedToCore(
    relayTask,            // Task function
    "RelayControlTask",   // Name
    4096,                 // Stack size in words (4096*4 = 16 KB)
    NULL,                 // Parameter
    1,                    // Priority
    &relayTaskHandle,     // Task handle
    1                     // Core
  );

  if (relayTaskHandle != NULL) {
    Serial.println("Relay task created successfully.");
  } else {
    Serial.println("Failed to create relay task!");
  }
}

void loop() {
  // Monitor stack usage of relay task every 10 seconds
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 10000) {
    lastPrint = millis();

    UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(relayTaskHandle);
    Serial.printf("Relay Task Stack HighWaterMark: %u words (%u bytes)\n",
                  highWaterMark, highWaterMark * 4);
    Serial.printf("Free heap: %u bytes\n", esp_get_free_heap_size());
  }

  // You could suspend/resume task here if needed:
  // vTaskSuspend(relayTaskHandle);
  // vTaskResume(relayTaskHandle);
}

// FreeRTOS task for relay control
void relayTask(void *parameter) {
  while (true) {
    for (int i = 0; i < NUM_RELAYS; i++) {
      // Turn all relays OFF
      for (int j = 0; j < NUM_RELAYS; j++) {
        digitalWrite(relayPins[j], HIGH);  // OFF
      }

      // Turn current relay ON
      digitalWrite(relayPins[i], LOW);  // ON
      Serial.printf("Relay %d ON\n", i + 1);

      vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait 1 second
    }
  }
}
