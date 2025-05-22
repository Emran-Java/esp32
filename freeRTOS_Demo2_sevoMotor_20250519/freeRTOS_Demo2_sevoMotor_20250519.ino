//#include <Servo.h>
#include <ESP32Servo.h>

#define SERVO_PIN 13   //D13
#define SERVO_PIN2 14  //D14
#define SERVO_PIN3 15  //D15

// Task handle
TaskHandle_t servoTaskHandle = NULL;

// Task handle2
TaskHandle_t servoTaskHandle2 = NULL;

// Task handle3
TaskHandle_t servoTaskHandle3 = NULL;


Servo servo1, servo2, servo3;  

void servoTask(void *pvParameters) {
  // Retrieve the core number the task is running on
  int core = xPortGetCoreID();
  while (1) {
    
    Serial.printf("Servo 1 ON - Running on core %d\n", core);
        
    for (int posDegrees = 0; posDegrees <= 180; posDegrees++) {
      servo1.write(posDegrees);
     // Serial.println(posDegrees);
      vTaskDelay(5 / portTICK_PERIOD_MS);
    }

    for (int posDegrees = 180; posDegrees >= 0; posDegrees--) {
      servo1.write(posDegrees);
     //Serial.println(posDegrees);
      vTaskDelay(5 / portTICK_PERIOD_MS);
    }
  }
}


void servoTask2(void *pvParameters) {
  // Retrieve the core number the task is running on
  int core = xPortGetCoreID();
  while (1) {
    
    Serial.printf("Servo 2 ON - Running on core %d\n", core);
        
    for (int posDegrees = 0; posDegrees <= 180; posDegrees++) {
      servo2.write(posDegrees);
      //Serial.println(posDegrees);
      vTaskDelay(25 / portTICK_PERIOD_MS);
    }

    for (int posDegrees = 180; posDegrees >= 0; posDegrees--) {
      servo2.write(posDegrees);
      //Serial.println(posDegrees);
      vTaskDelay(25 / portTICK_PERIOD_MS);
    }
  }
}


void servoTask3(void *pvParameters) {
  // Retrieve the core number the task is running on
  int core = xPortGetCoreID();
  while (1) {
    
    Serial.printf("Servo 3 ON - Running on core %d\n", core);
        
    for (int posDegrees = 0; posDegrees <= 180; posDegrees++) {
      servo3.write(posDegrees);
      //Serial.println(posDegrees);
      vTaskDelay(60 / portTICK_PERIOD_MS);
    }

    for (int posDegrees = 180; posDegrees >= 0; posDegrees--) {
      servo3.write(posDegrees);
      //Serial.println(posDegrees);
      vTaskDelay(60 / portTICK_PERIOD_MS);
    }
  }
}


void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  servo1.attach(SERVO_PIN);
  servo2.attach(SERVO_PIN2);
  servo3.attach(SERVO_PIN3);

 //servo1.write(90);
 //servo2.write(90);
 //servo3.write(90);

  // Create the LED task
  xTaskCreatePinnedToCore(
    servoTask,       // Task function
    "SERVO Task",    // Task name
    2048,            // Stack size
    NULL,            // Task parameters
    1,               // Task priority
    &servoTaskHandle,  // Task handle
    0                // Core number to run the task on (0 or 1)
  );

   // Create the SERVO task2
  xTaskCreatePinnedToCore(
    servoTask2,       // Task function
    "SERVO Task2",    // Task name
    2048,            // Stack size
    NULL,            // Task parameters
    1,               // Task priority
    &servoTaskHandle2,  // Task handle
    0                // Core number to run the task on (0 or 1)
  );

  // Create the SERVO task3
  xTaskCreatePinnedToCore(
    servoTask3,       // Task function
    "SERVO Task3",    // Task name
    2048,            // Stack size
    NULL,            // Task parameters
    1,               // Task priority
    &servoTaskHandle3,  // Task handle
    0                // Core number to run the task on (0 or 1)
  );

}

void loop() {
  // put your main code here, to run repeatedly:
}
