#include <DHT.h>

#define dhtPIN 2 //D4

#define dhtTYPE DHT11

DHT dht(dhtPIN, dhtTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  delay(1000);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.println("Temperature:");
  Serial.println(temperature);
  Serial.println("Humidity:");
  Serial.println(humidity);
}
