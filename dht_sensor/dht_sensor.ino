#include <DHT.h>
#define DHTPIN 27     // Digital pin connected to the DHT sensor
// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
/media/emran/karkhana/iOT_project/esp32/multiThredTest/multiThredTest.ino
DHT dht(DHTPIN, DHTTYPE);

void setup(){
  Serial.begin(115200);

  dht.begin();

}

void loop()
{

 float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    //return "--";
  }
  else {
    Serial.println(t);
    //return String(t);
  }


 float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    //return "--";
  }
  else {
    Serial.println(h);
    //return String(h);
  }

   delay(4000);
}