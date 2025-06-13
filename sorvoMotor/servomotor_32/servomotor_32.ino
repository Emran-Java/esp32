//#include <Servo.h>
#include <ESP32Servo.h>

Servo servo1, servo2;

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

static const int servoPin = 14, servoPin2 = 15; //D14


void setup() {

  Serial.begin(115200);
  //servo1.attach(servoPin);
  servo2.attach(servoPin2);
  servo2.write(30);
}

void loop() {
  servo2.write(100);
     delay(2000);
   servo2.write(30);
   delay(2000);
    
  // for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
  //   servo1.write(posDegrees);
  //   Serial.println(posDegrees);
  //   //servo2.write(posDegrees);
  //   delay(20);
  // }

  // for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
  //   servo1.write(posDegrees);
  //   Serial.println(posDegrees);
  //   //servo2.write(posDegrees);
  //   delay(20);
  // }
}