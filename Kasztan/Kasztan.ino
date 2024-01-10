#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define dhtPIN 2 //D4

#define dhtTYPE DHT11

const char *ssid = "UPC1263944";
const char *password = "AeaAbednck5p";

float temperature;
float humidity;

DHT dht(dhtPIN, dhtTYPE);
WiFiClient client;
ESP8266WebServer server(80);   

void setup() {
  Serial.begin(9600);
  dht.begin();
  connectToWiFi();
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

}

void loop() {
  server.handleClient();
  /*
  delay(1000);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.println("Temperature:");
  Serial.println(temperature);
  Serial.println("Humidity:");
  Serial.println(humidity);*/
}

void connectToWiFi() {
  //Connect to WiFi Network
    Serial.println();
    Serial.println();
    Serial.println("Connecting to WiFi");
    Serial.println("...");
    WiFi.begin(ssid, password);
    int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    retries++;
    delay(500);
    Serial.print(".");
  }
  if (retries > 14) {
      Serial.println(F("WiFi connection FAILED"));
  }
  if (WiFi.status() == WL_CONNECTED) {
      Serial.println(F("WiFi connected!"));
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
  }
      Serial.println(F("Setup ready"));
}

void handle_OnConnect() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  server.send(200, "text/html", SendHTML(temperature,humidity)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature,float humidity){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP8266 Weather Station</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>KASZTAN Weather Station</h1>\n";
  ptr +="<p>Temperature: ";
  ptr +=temperature;
  ptr +="&deg;C</p>";
  ptr +="<p>Humidity: ";
  ptr +=humidity;
  ptr +="%</p>";
  ptr +="m</p>";
  ptr+="<p>Julia to mega slodziak, slay!";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
