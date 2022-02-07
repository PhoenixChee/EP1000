#include <DHT.h>

#define DHTPIN 3 
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  dht.begin();
  Serial.begin(9600);
}

void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  delay(1000);
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
}
