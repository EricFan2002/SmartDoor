static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "DHT.h"
#include <sstream>
#include <string>
#include "Adafruit_CCS811.h"
#include "Wire.h"
#include <Button2.h>
#include "LiquidCrystal_I2C.h"

Adafruit_CCS811 ccs;
Button2 flashbtn(0);
Button2 fnbtn(D2);

float temp1 = 0;
float hum1 = 0;

int MODE = 0;

// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

/*Put your SSID & Password*/
const char* ssid = "eric24";  // Enter SSID here
const char* password = "58112403";  //Enter Password here

ESP8266WebServer server(80);

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// DHT Sensor
uint8_t DHTPin = D1; 
               
// Initialize DHT sensor.
DHT dht(DHTPin, DHT11);                

float Temperature;
float Humidity;

int tvoc;
int co2;


 
void setup() {
  Serial.begin(115200);
  pinMode(D4, OUTPUT);
  digitalWrite(D4,0);
  /*pinMode(0, INPUT_PULLUP);

  pinMode(D6, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  Wire.begin(D6, D5);

Serial.begin(115200);
  delay(100);
  
  while(!ccs.begin()){
    Serial.println("1:Failed to start sensor! Please check your wiring.");
    delay(1000);
  }
  while(!ccs.available());
  if(ccs.available()){
    if(!ccs.readData()){
      Serial.print("CO2: ");
      Serial.println(ccs.geteCO2());
      Serial.print("TVOC: ");
      Serial.println(ccs.getTVOC());
      Serial.print("Temp ");
      Serial.println((int)((ccs.calculateTemperature()-32)/1.8));
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
  }


  
  
  
  pinMode(DHTPin, INPUT);
  pinMode(D2, INPUT);
  dht.begin();              

  Serial.println("Connecting to ");
  Serial.println(ssid);
*/
  //connect to your local wi-fi network
  WiFi.begin("3292.4G", "329233666");

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

/*
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
*/

  server.on("/pass", handle_ok);
  server.on("/t2", handle_main);

server.begin();
  
}

long lasttt = 0;



float dist = 0;
float dish = 0;

void loop() {
 
  
  server.handleClient();
  if (WiFi.status() != WL_CONNECTED) Serial.println("ERR,RSTing"),ESP.restart();
  //ArduinoOTA.handle();
}


void handle_ok(){

  server.send(200,"ok door is opened!"); 
  Serial.println("req open");
  digitalWrite(D4, 1);
  delay(1000);
  digitalWrite(D4,0);
  
  
}


void handle_main(){
  server.send(200,"I am online!q"); 
   Serial.println("req online");
}
