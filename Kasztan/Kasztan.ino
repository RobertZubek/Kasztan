#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <U8g2lib.h>
#include <string.h>
#include <ArduinoJson.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define dhtTYPE DHT11

#define dhtPIN 12 //D6
#define fotoPIN A0
#define redPIN 14//D5
#define greenPIN 15//D8
#define bluePIN 2//D4
#define switch1 13 //D7
#define switch2 16 //D0

const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=Krakow,pl&APPID="; //api for Cracow
const String key = "3d314141e44b26727482648506172985";

const char *ssid = "UPC913DBEC";
const char *password = "tC4jruczkpty";

static float temperature;
static float humidity;
static float light;

static float bmpTemperature;
static float bmpHumidity;
static float bmpPressure;
static float bmpAltitude;

static uint8_t redValue = 255;
static uint8_t greenValue = 255;
static uint8_t blueValue = 255;

static uint8_t sw1 = 1;
static uint8_t sw2 = 1;

static String ondisp;
static String ondisp2;
static String ondisp3;

struct Weather {
    String main;
    String description;
    float temperature;
    float feels_like;
    int pressure;
    int humidity;
    float wind_speed;
    int cloudiness;
};


DHT dht(dhtPIN, dhtTYPE);
WiFiClient client;
HTTPClient http1;
ESP8266WebServer server(80);   
Adafruit_BME280 bme;
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL, SDA);

void parseWeatherJson(const char* json, Weather &weather) {
    // Zwiększony rozmiar bufora, aby pomieścić większy JSON
    const size_t capacity = 1024;
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    weather.main = doc["weather"][0]["main"].as<String>();
    weather.description = doc["weather"][0]["description"].as<String>();
    weather.temperature = doc["main"]["temp"].as<float>();
    weather.feels_like = doc["main"]["feels_like"].as<float>();
    weather.pressure = doc["main"]["pressure"].as<int>();
    weather.humidity = doc["main"]["humidity"].as<int>();
    weather.wind_speed = doc["wind"]["speed"].as<float>();
    weather.cloudiness = doc["clouds"]["all"].as<int>();
}


void setup() {

  pinMode(redPIN, OUTPUT);
  pinMode(greenPIN, OUTPUT);
  pinMode(bluePIN, OUTPUT);
  pinMode(switch1, INPUT_PULLUP);
  pinMode(switch2, INPUT_PULLUP);

  Serial.begin(9600);
  dht.begin();
  u8g2.begin();
  connectToWiFi();
  server.on("/", handle_OnConnect);
  server.on("/increaseRed", HTTP_GET, handle_IncreaseRed);
  server.on("/decreaseRed", HTTP_GET, handle_DecreaseRed);
  server.on("/increaseGreen", HTTP_GET, handle_IncreaseGreen);
  server.on("/decreaseGreen", HTTP_GET, handle_DecreaseGreen);
  server.on("/increaseBlue", HTTP_GET, handle_IncreaseBlue);
  server.on("/decreaseBlue", HTTP_GET, handle_DecreaseBlue);
  server.on("/off", HTTP_GET, handle_off);
  server.onNotFound(handle_NotFound);

  server.begin();
  
  delay(300);
  if (!bme.begin(0x76)) { Serial.println("bme280 error");}
  else{Serial.println("bme280 ok");}
 
}

void loop() {
  Weather weather;
  Serial.println("IP Address: ");
  Serial.print(WiFi.localIP()[0]);
  Serial.print(".");
  Serial.print(WiFi.localIP()[1]);
  Serial.print(".");
  Serial.print(WiFi.localIP()[2]);
  Serial.print(".");
  Serial.print(WiFi.localIP()[3]);
  Serial.print("\n");
  server.handleClient();
  http1.begin(client, (endpoint + key).c_str());
  int httpCode = http1.GET();
  if (httpCode > 0) { 
 
        String payload = http1.getString();
        Serial.println(httpCode);
        delay(300);
        Serial.println(httpCode);
        delay(1000);
        Serial.println(payload);
        parseWeatherJson(payload.c_str(), weather);
        delay(1000);
      }
 
    else {
      Serial.println("Error on HTTP request");
    }
 
    http1.end(); 
    
  sw1=digitalRead(switch1);
  sw2=digitalRead(switch2);
    if(digitalRead(switch1)==HIGH){
      ondisp=weather.main+" Temp: "+String(weather.temperature-272.15)+" Cel.";
      ondisp2="Wind: "+String(weather.wind_speed)+"m/s Hum.: "+weather.humidity+"%";
      ondisp3="Press: "+String(weather.pressure)+"hPa Clouds: "+String(weather.cloudiness)+"%";
      u8g2.firstPage();
      do {
      u8g2.setFont(u8g2_font_micro_tr);
      u8g2.drawStr(0,10,ondisp.c_str());
      u8g2.drawStr(0,20,ondisp2.c_str());
      u8g2.drawStr(0,30,ondisp3.c_str());
      } while ( u8g2.nextPage() ); 
      }
    if(digitalRead(switch1)==LOW){
      ondisp=" Temp: "+String(dht.readTemperature())+" Cel.";
      ondisp2="Hum.: "+String(dht.readHumidity())+"% Light: "+String(analogRead(fotoPIN));
      ondisp3="IP: "+String(WiFi.localIP()[0])+"."+String(WiFi.localIP()[1])+"."+String(WiFi.localIP()[2])+"."+String(WiFi.localIP()[3]);
      u8g2.firstPage();
      do {
      u8g2.setFont(u8g2_font_micro_tr);
      u8g2.drawStr(0,10,ondisp.c_str());
      u8g2.drawStr(0,20,ondisp2.c_str());
      u8g2.drawStr(0,30,ondisp3.c_str());
      } while ( u8g2.nextPage() ); 
      }
 
  
}

void connectToWiFi() {
    Serial.println();
    Serial.println();
    WiFi.begin(ssid, password);
    int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    retries++;
    delay(500);
    //lcd.print(".");
  }
  if (retries > 14) {
     /* lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("WiFi connection FAILED"));*/
  }
  if (WiFi.status() == WL_CONNECTED) {
      /*lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("WiFi connected!"));
      delay(300);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("IP address: ");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
      lcd.clear();
      lcd.setCursor(0, 0);
      */
  }
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
  ptr += "<p>DHT11 Temperature: ";
  ptr += temperature;
  ptr += "&deg;C</p>";
  ptr += "<p>DHT11 Humidity: ";
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
  ptr += "<p>by Robert Zubek</p>";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

