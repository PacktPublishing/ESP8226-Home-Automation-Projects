#include "FS.h"

#include <ESP8266WiFi.h>  
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>    

boolean init_esp = false;

void setup() 
{
  Serial.begin(115200); delay(10);
  
  if(init_esp)
  {
    SPIFFS.format();
    WiFi.disconnect();
  }
  
  WiFiManager wifiManager;
  
  wifiManager.setConfigPortalTimeout(240);
  
  if (!wifiManager.autoConnect("ESP_AP", "password")) 
  {
    Serial.println(F("Failed to connect. Reset and try again..."));
    delay(3000);
    //reset and try again
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println(F("Connected to Wifi."));
  Serial.print(F("My IP: "));
  Serial.println(WiFi.localIP());
}

void loop() {
  //add your code for loop()
}
