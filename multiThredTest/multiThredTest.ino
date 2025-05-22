#include <DHT.h>
#include <Servo.h>

#define DHTPIN 4           // DHT connected to GPIO4
#define DHTTYPE DHT11      // Change to DHT22 if needed
#define MQ2PIN 34          // MQ2 analog output connected to GPIO34
#define SERVOPIN 15        // Servo connected to GPIO15

DHT dht(DHTPIN, DHTTYPE);
Servo servoMotor;

// Servo state
int angle = 0;
bool goingUp = true;

// Task handles (optional, for tracking)
TaskHandle_t TaskDHT, TaskMQ2, TaskServo;

void setup() {
  Serial.begin(115200);

  dht.begin();
  servoMotor.attach(SERVOPIN);

  // Create tasks
  xTaskCreatePinnedToCore(readDHTTask, "Read DHT", 2048, NULL, 1, &TaskDHT, 0);    // Core 0
  xTaskCreatePinnedToCore(readMQ2Task, "Read MQ2", 2048, NULL, 1, &TaskMQ2, 0);    // Core 0
  xTaskCreatePinnedToCore(servoTask,   "Servo Move", 2048, NULL, 1, &TaskServo, 1); // Core 1
}

void loop() {
  // Nothing in loop — all done in tasks
}

// Task: Read DHT sensor every 3 seconds
void readDHTTask(void *pvParameters) {
  while (1) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      Serial.printf("DHT → Temp: %.1f °C, Humidity: %.1f %%\n", t, h);
    } else {
      Serial.println("DHT → Failed to read");
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);  // 3 seconds
  }
}

// Task: Read MQ2 every 1 second
void readMQ2Task(void *pvParameters) {
  while (1) {
    int mq2Value = analogRead(MQ2PIN);
    Serial.printf("MQ2 → Gas value: %d\n", mq2Value);
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1 second
  }
}

// Task: Move servo continuously
void servoTask(void *pvParameters) {
  while (1) {
    if (goingUp) {
      angle++;
      if (angle >= 180) goingUp = false;
    } else {
      angle--;
      if (angle <= 0) goingUp = true;
    }
    servoMotor.write(angle);
    vTaskDelay(20 / portTICK_PERIOD_MS);  // Adjust speed of movement
  }
}
