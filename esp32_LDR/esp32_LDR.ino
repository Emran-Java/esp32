#include <math.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager

#define LIGHT_SENSOR_PIN 35 // ESP32 pin 35 (ADC0)

void setup() {
  Serial.begin(115200);
  // set the ADC attenuation to 11 dB (up to ~3.3V input)
  analogSetAttenuation(ADC_11db);
/*
  WiFiManager wm;
  bool res;
  //res = wm.autoConnect();  // auto generated AP name from chipid
  res = wm.autoConnect("bo_wifi_8");  // anonymous ap
*/
  /*if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } else {
    //if you get here you have connected to the WiFi
    //Serial.println("connected...with ronok :)");
    Serial.println("\nConnected to Wi-Fi");
  }*/
  
}

void loop() {
  int analogValue = 0;

  double avg = 0.0;
  for(int i=0;i<10;i++){
    analogValue = analogRead(LIGHT_SENSOR_PIN);
    avg = avg+analogValue;
    delay(200);
  }
  analogValue = avg/10.0;
  Serial.print("Analog Value = ");
  Serial.print(analogValue);   // the raw analog reading
  Serial.print(" "+getLightingCondition(analogValue));
  Serial.println(" lux: "+String(getLux(analogValue))); 
}

String getLightingCondition(int analogValue) {
  if (analogValue >= 3500) {
    return "Very bright (Bright sunlight)";
  } else if (analogValue >= 2000) {
    return "Bright (Bright indoor lighting)";
  } else if (analogValue >= 1000) {
    return "Moderate (Indoor dim lighting)";
  } else if (analogValue >= 300) {
    return "Dim (Low ambient light)";
  } else if (analogValue >= 50) {
    return "Dark (Almost dark)";
  } else {
    return "Very dark / Night";
  }
}

//To convert the analog reading from an LDR (Light Dependent Resistor) to illuminance in lux (lx)
float getLux(int adcVal){
  
  float voltage = adcVal * (3.3 / 4095.0);  // Convert to volts, 3.3 is input voltage

  float Rldr = (3.3 * 10000.0 / voltage) - 10000.0;  // Resistance in ohms, we use 10K reg
  float lux = 500 / pow(Rldr / 1000.0, 1.4);  // Estimate Lux
  return lux;
}