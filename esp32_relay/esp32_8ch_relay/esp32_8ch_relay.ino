#define NUM_RELAYS 8

// Define GPIO pins connected to the relay module
const int relayPins[NUM_RELAYS] = {2, 4, 5, 18, 19, 21, 22, 23};

// Create a task handle for the relay control task
TaskHandle_t relayTaskHandle = NULL;

void setup() {
  Serial.begin(115200);

  // Set all relay pins as OUTPUT and turn them OFF initially
  for (int i = 0; i < NUM_RELAYS; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // Assume active LOW relay (HIGH = OFF)
  }

  // Create the relay task and assign to task handle
  xTaskCreatePinnedToCore(
    relayTask,              // Task function
    "RelayControlTask",     // Task name
    2048,                   // Stack size
    NULL,                   // Parameter
    1,                      // Priority
    &relayTaskHandle,       // Task handle (for suspend/resume/etc.)
    1                       // Core 1
  );
}

void loop() {
  // FreeRTOS task handles the relay logic
  // If needed, you can suspend/resume the task using the handle:
  // vTaskSuspend(relayTaskHandle);
  // vTaskResume(relayTaskHandle);
}

void relayTask(void *parameter) {
  while (true) {
    for (int i = 0; i < NUM_RELAYS; i++) {
      // Turn off all relays
      for (int j = 0; j < NUM_RELAYS; j++) {
        digitalWrite(relayPins[j], HIGH); // OFF
      }

      // Turn on current relay
      digitalWrite(relayPins[i], LOW); // ON
      //digitalWrite(4, LOW); // ON
      Serial.printf("Relay %d ON\n", i + 1);

      // Wait 1 second
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  }
}
