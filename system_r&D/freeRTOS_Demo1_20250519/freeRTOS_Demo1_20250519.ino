// Define the onboard LED pin for ESP32 (typically GPIO 2)
#define LED_PIN 2
#define LED_PIN2 1
#define LED_PIN3 3

// Task handle
TaskHandle_t ledTaskHandle = NULL;

// Task handle2
TaskHandle_t ledTaskHandle2 = NULL;

// Task handle3
TaskHandle_t ledTaskHandle3 = NULL;

void ledTask(void *pvParameters) {
  // Retrieve the core number the task is running on
    int core = xPortGetCoreID();
    for (;;) {

      // Toggle the LED
        digitalWrite(LED_PIN, HIGH);
        Serial.printf("LED ON - Running on core %d\n", core);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        digitalWrite(LED_PIN, LOW);
        Serial.printf("LED OFF - Running on core %d\n", core);
       vTaskDelay(1000 / portTICK_PERIOD_MS);
      
    }
}


void ledTask2(void *pvParameters) {
  // Retrieve the core number the task is running on
    int core = xPortGetCoreID();
    for (;;) {

      // Toggle the LED
        digitalWrite(LED_PIN2, HIGH);
        Serial.printf("LED ON - Running on core %d\n", core);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        digitalWrite(LED_PIN2, LOW);
        Serial.printf("LED OFF - Running on core %d\n", core);
       vTaskDelay(2000 / portTICK_PERIOD_MS);
      
    }
}


void ledTask3(void *pvParameters) {
  // Retrieve the core number the task is running on
    int core = xPortGetCoreID();
    for (;;) {

      // Toggle the LED
        digitalWrite(LED_PIN3, HIGH);
        Serial.printf("LED ON - Running on core %d\n", core);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        digitalWrite(LED_PIN3, LOW);
        Serial.printf("LED OFF - Running on core %d\n", core);
       vTaskDelay(4000 / portTICK_PERIOD_MS);
      
    }
}


void setup() {
 // Initialize serial communication
    Serial.begin(115200);

    // Initialize the onboard LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(LED_PIN2, OUTPUT);
    digitalWrite(LED_PIN2, LOW);

    pinMode(LED_PIN3, OUTPUT);
    digitalWrite(LED_PIN3, LOW);

    // Create the LED task
    xTaskCreatePinnedToCore(
        ledTask,       // Task function
        "LED Task",    // Task name
        2048,          // Stack size
        NULL,          // Task parameters
        1,             // Task priority
        &ledTaskHandle,// Task handle
        0              // Core number to run the task on (0 or 1)
    );


    // Create the LED task 2
    xTaskCreatePinnedToCore(
        ledTask2,       // Task function
        "LED Task2",    // Task name
        2048,          // Stack size
        NULL,          // Task parameters
        1,             // Task priority
        &ledTaskHandle2,// Task handle
        0              // Core number to run the task on (0 or 1)
    );


    // Create the LED task 2
    xTaskCreatePinnedToCore(
        ledTask3,       // Task function
        "LED Task2",    // Task name
        2048,          // Stack size
        NULL,          // Task parameters
        1,             // Task priority
        &ledTaskHandle3,// Task handle
        0              // Core number to run the task on (0 or 1)
    ); 

}

void loop() {
  // put your main code here, to run repeatedly:

}
