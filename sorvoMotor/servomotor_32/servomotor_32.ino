//#include <Servo.h>
#include <ESP32Servo.h>

Servo servo;

// void setup() {
//   servo.attach(2); //D4
//   servo.write(0);
//   delay(2000);
// }

// void loop() {
//   servo.write(90);
//   delay(1000);
//   servo.write(0);
//   delay(1000);
// }

static const int servoPin = 13; //D13

Servo servo1;

void setup() {

  Serial.begin(115200);
  servo1.attach(servoPin);
}

void loop() {
  for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
    servo1.write(posDegrees);
    Serial.println(posDegrees);
    delay(20);
  }

  for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
    servo1.write(posDegrees);
    Serial.println(posDegrees);
    delay(20);
  }
}