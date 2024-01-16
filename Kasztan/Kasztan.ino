#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define dhtPIN 2 //D4
#define fotoPIN A0
#define redPIN 5//D3
#define greenPIN 4//D2
#define bluePIN 0//D1

const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=Krakow,pl&APPID=";
const String key = "13e6fd15ee63796bcf8f4809fb26832b";

#define dhtTYPE DHT11

const char *ssid = "UPC913DBEC";
const char *password = "tC4jruczkpty";

float temperature;
float humidity;
float light;

int redValue = 128;
int greenValue = 128;
int blueValue = 128;

DHT dht(dhtPIN, dhtTYPE);
WiFiClient client;
HTTPClient http1;
ESP8266WebServer server(80);   

void setup() {

  pinMode(redPIN, OUTPUT);
  pinMode(greenPIN, OUTPUT);
  pinMode(bluePIN, OUTPUT);


  Serial.begin(9600);
  dht.begin();
  connectToWiFi();
  server.on("/", handle_OnConnect);
  server.on("/increaseRed", HTTP_GET, handle_IncreaseRed);
  server.on("/decreaseRed", HTTP_GET, handle_DecreaseRed);
  server.on("/increaseGreen", HTTP_GET, handle_IncreaseGreen);
  server.on("/decreaseGreen", HTTP_GET, handle_DecreaseGreen);
  server.on("/increaseBlue", HTTP_GET, handle_IncreaseBlue);
  server.on("/decreaseBlue", HTTP_GET, handle_DecreaseBlue);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

}

void loop() {
  server.handleClient();
  http1.begin(client, (endpoint + key).c_str());
  int httpCode = http1.GET();
  if (httpCode > 0) { 
 
        String payload = http1.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      }
 
    else {
      Serial.println("Error on HTTP request");
    }
 
    http1.end(); 
  
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
  light = analogRead(fotoPIN);
  server.send(200, "text/html", SendHTML(temperature,humidity, light, redValue, greenValue, blueValue)); 
}

void handle_IncreaseRed()
{
  if (redValue > 11)
    redValue-=10;
  analogWrite(redPIN, redValue);
  server.send(200, "text/html", SendHTML(temperature, humidity, light, redValue, greenValue, blueValue));
}
void handle_IncreaseGreen()
{
  if (greenValue > 11)
    greenValue-=10;
  analogWrite(greenPIN, greenValue);
  server.send(200, "text/html", SendHTML(temperature, humidity, light, redValue, greenValue, blueValue));
}
void handle_IncreaseBlue()
{
  if (blueValue > 11)
    blueValue-=10;
  analogWrite(bluePIN, blueValue);
  server.send(200, "text/html", SendHTML(temperature, humidity, light, redValue, greenValue, blueValue));
}
void handle_DecreaseRed()
{
  if (redValue <244)
    redValue+=10;
  analogWrite(redPIN, redValue);
  server.send(200, "text/html", SendHTML(temperature, humidity, light, redValue, greenValue, blueValue));
}
void handle_DecreaseGreen()
{
  if (greenValue < 244)
    greenValue+=10;
  analogWrite(greenPIN, greenValue);
  server.send(200, "text/html", SendHTML(temperature, humidity, light, redValue, greenValue, blueValue));
}
void handle_DecreaseBlue()
{
  if (blueValue <244)
    blueValue+=10;
  analogWrite(bluePIN, blueValue);
  server.send(200, "text/html", SendHTML(temperature, humidity, light, redValue, greenValue, blueValue));
}
void handle_off()
{
  redValue=255;
  analogWrite(redPIN, redValue);
  greenValue=255;
  analogWrite(greenPIN, greenValue);
  blueValue=255;
  analogWrite(bluePIN, blueValue);
  server.send(200, "text/html", SendHTML(temperature, humidity, light, redValue, greenValue, blueValue));
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature,float humidity, float light, int red, int green, int blue){
   String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>ESP8266 Weather Station</title>\n";
  ptr += "<script>function increaseColor(color) {"
         "var xhr = new XMLHttpRequest();"
         "xhr.open('GET', '/increase' + color, true);"
         "xhr.send();}"
         "function decreaseColor(color) {"
         "var xhr = new XMLHttpRequest();"
         "xhr.open('GET', '/decrease' + color, true);"
         "xhr.send();}</script>\n";
  ptr += "<script>function turnOff(){"
         "var xhr = new XMLHttpRequest();"
         "xhr.open('GET', '/off', true);"
         "xhr.send();}</script>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>KASZTAN Weather Station</h1>\n";
  ptr += "<p>Temperature: ";
  ptr += temperature;
  ptr += "&deg;C</p>";
  ptr += "<p>Humidity: ";
  ptr += humidity;
  ptr += "%</p>";
  ptr += "<p>Light: ";
  ptr += light;
  ptr += "</p>";
  ptr += "<p>RGB LED:</p>";
  ptr += "<button onclick=\"increaseColor('Red')\">Increase Red</button>";
  ptr += "<button onclick=\"decreaseColor('Red')\">Decrease Red</button><br>";
  ptr += "<button onclick=\"increaseColor('Green')\">Increase Green</button>";
  ptr += "<button onclick=\"decreaseColor('Green')\">Decrease Green</button><br>";
  ptr += "<button onclick=\"increaseColor('Blue')\">Increase Blue</button>";
  ptr += "<button onclick=\"decreaseColor('Blue')\">Decrease Blue</button><br>";
  ptr += "<button onclick=\"turnOff()\">OFF</button><br>";
  ptr += "<p>RED: ";
  ptr += red;
  ptr += "<p>GREEN: ";
  ptr += green;
  ptr += "<p>BLUE: ";
  ptr += blue;
  ptr += "<p>kocham Julcie</p>";
  ptr += "<p>by Robert Zubek</p>";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
