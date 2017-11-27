
#include "Wire.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <FS.h>                   
#include <ESP8266WiFi.h>         
#include "SocketIOClient.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          
#include <ArduinoJson.h>          
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <SPI.h>

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(121212);

char acc_server[40];
char acc_port[6] = "1234";

#define ACC_CLIENT_ID         "ACC_%06X"
#define SIGNAL_PIN 13 //D7
#define INFO Serial.printf
char dev_name[50];

int clean_g = 0; // set to 1 to clear the wifiSettings and SFIFFS 

//flag for saving data
bool shouldSaveConfig = false;

SocketIOClient client;
StaticJsonBuffer<300> jsonBuffer; 
extern String RID;
extern String Rname;
//extern String Rcontent;
unsigned long previousMillis = 0;
long interval = 100;
unsigned long lastreply = 0;
unsigned long lastsend = 0;


//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

/*******************************************************************
 *         S E T U P
 *******************************************************************/

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200); delay(10);
  Serial.println();
  pinMode(A0, INPUT);
  pinMode(SIGNAL_PIN, OUTPUT); digitalWrite(SIGNAL_PIN, LOW);

 /* Initialise the sensor */
 if(!accel.begin())
 {
 /* There was a problem detecting the ADXL345 ... check your connections */
 Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
 while(1);
 }
 
 /* Set the range to whatever is appropriate for your project */
 accel.setRange(ADXL345_RANGE_16_G);
 // accel.setRange(ADXL345_RANGE_8_G);
 // accel.setRange(ADXL345_RANGE_4_G);
 // accel.setRange(ADXL345_RANGE_2_G);


  if(clean_g)
   SPIFFS.format();

  //read configuration from FS json
  Serial.println(F("mounting FS..."));

  if (SPIFFS.begin()) 
  {
    Serial.println(F("mounted file system"));
    if (SPIFFS.exists("/config.json")) 
    {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println(F("\nparsed json"));

          strcpy(acc_server, json["acc_server"]);
          strcpy(acc_port,   json["acc_port"]);                 
        } else {
          Serial.println(F("failed to load json config"));
        }
      }
    }   
  } else {
    Serial.println(F("failed to mount FS"));
  }

  WiFiManagerParameter custom_acc_server("server", "RAM IP", acc_server, 40);
  WiFiManagerParameter custom_acc_port("port", "RAM port", acc_port, 5);
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  //add all your parameters here
  wifiManager.addParameter(&custom_acc_server);
  wifiManager.addParameter(&custom_acc_port);

  if(clean_g)
   wifiManager.resetSettings();
  
  sprintf(dev_name, ACC_CLIENT_ID, ESP.getChipId());
  INFO("..DEV:%s \n",dev_name);
  
  if ( !wifiManager.autoConnect(dev_name) )  
    {
      Serial.println(F("failed to connect and hit timeout"));
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  //read updated parameters
  strcpy(acc_server, custom_acc_server.getValue());
  strcpy(acc_port, custom_acc_port.getValue());
//  strcpy(machine_id, custom_machine_id.getValue() );

  //save the custom parameters to FS. Only at init time..
  if (shouldSaveConfig) 
  {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["acc_server"] = acc_server;
    json["acc_port"]   = acc_port;
   
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    
    configFile.close();
  }

  Serial.print(F("\nlocal ip:"));
  Serial.println(WiFi.localIP());
  INFO("ACC acc_server: %s\n", acc_server);
  INFO("ACC acc_server_port: %s\n", acc_port);

  if (!client.connect(acc_server, atoi(acc_port) )) 
  {     
    Serial.println(F("connection failed"));
    return;
  }
  if (client.connected()) 
  {
    Serial.println("client.connected. Send connection msg to app.js");
    client.sendJSON("connection", "{\"acc_id\":\"" + String(dev_name) + "\" }" );
  }
 
} //=================> END SETUP


/*******************************************************************
 *         M A I N   L O O P
 *******************************************************************/
void loop() 
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval)
  {
    previousMillis = currentMillis;

    sensors_event_t event; 
    accel.getEvent(&event);
    Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print(" ");
    Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print(" ");
    Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print(" ");Serial.println("m/s^2 ");

    String acc_data;
    StaticJsonBuffer<100> jsonDeviceStatus;
    JsonObject& jsondeviceStatus = jsonDeviceStatus.createObject();
    jsondeviceStatus["device_name"] = dev_name;
    //readAccel();  
    jsondeviceStatus["x"]         = event.acceleration.x;//x;
    jsondeviceStatus["y"]         = event.acceleration.y;//y;
    jsondeviceStatus["z"]         = event.acceleration.z;//z;

    jsondeviceStatus.printTo(acc_data);
    client.sendJSON("JSON", acc_data); 
  } 

  // =============================== RX messages from WEB PAGE
  //look for any message receive from WEB page
  if(client.monitor())
  {
    lastreply = millis(); 
    if(strcmp(String(RID).c_str(), "welcome") == 0)
    {
      client.sendJSON("connection", "{\"acc_id\":\"" + String(dev_name) + "\" }" );      
    }
    //get the message type
    if(RID != "")
    {
     Serial.print(F("Message: ")); Serial.println(RID); 

     if(strcmp(String(RID).c_str(), "calibration") == 0)
     {
        //calibrate_sensor();
     }

     if(strcmp(String(RID).c_str(), "resetModule") == 0)
     {
       //reset the module 
       delay(1000);
       ESP.reset();
     }     
     if(strcmp(String(RID).c_str(), "initModule") == 0)
     {
       WiFi.disconnect(true);
       //ESP.reset();
       delay(1000);
       *((int*)0) = 0;
     }      
   }
  }

  if (!client.connected()) 
  {
     //digitalWrite(SIGNAL_PIN, LOW);
     Serial.println("LOOP: Client not connected, try to reconnect");
     client.connect(acc_server, atoi(acc_port) );     
     while(!client.connected())
     {
       client.connect(acc_server,atoi(acc_port));
       delay(1000);
     }
     //client.sendJSON("connection", "{\"acc_id\":\"" + String(dev_name) + "\",\"machine_id\":\""+ String(machine_id) +"\"}");
     client.sendJSON("connection", "{\"acc_id\":\"" + String(dev_name) + "\" }" );
  }
  else
  {
     //digitalWrite(SIGNAL_PIN, HIGH);
  }
  //heartbeat.update();
}
