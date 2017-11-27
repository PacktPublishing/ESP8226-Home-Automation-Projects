#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <PubSubClient.h>
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#define CLIENT_ID         "ESP_%06X"

//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_user[32];
char mqtt_pass[32];
char mqtt_topic[64];
char dev_name[50];

boolean clean_g = false;

WiFiClient espClient;
PubSubClient client(espClient);


void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  char rxj[512]; 
  for (int i = 0; i < length; i++) {
    rxj[i] = payload[i];
  } 
  Serial.println(rxj);
}

//flag for saving data
bool saveConfig = false;

//callback notifying us of the need to save config
void saveConfigToFileFn () {
  Serial.println("Should save config");
  saveConfig = true;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); delay(10);
  sprintf(dev_name, CLIENT_ID, ESP.getChipId());  

  if(clean_g)
  {
    Serial.println(F("\n\nWait...I am formating  the FLASH !!!"));
    SPIFFS.format();
    Serial.println(F("Done!"));
    WiFi.disconnect(true);     
  }

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
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
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_pass, json["mqtt_pass"]);
          strcpy(mqtt_topic, json["mqtt_topic"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }

  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 32);
  WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqtt_pass, 32);
  WiFiManagerParameter custom_mqtt_topic("topic", "mqtt topic", mqtt_topic, 64);

  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigToFileFn);
  
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);
  wifiManager.addParameter(&custom_mqtt_topic);

  if (!wifiManager.autoConnect("ESP_AP"))
  {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  Serial.println(F("WiFi is connected now..."));

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());
  strcpy(mqtt_topic, custom_mqtt_topic.getValue());

  if (saveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"]   = mqtt_port;
    json["mqtt_user"]   = mqtt_user;
    json["mqtt_pass"]   = mqtt_pass;
    json["mqtt_topic"]  = mqtt_topic;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println(F("failed to open config file for writing"));
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println(F("My IP address: "));
  Serial.println(WiFi.localIP());
  //connect to mqtt server
  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(mqtt_callback);
  if (client.connect(dev_name , mqtt_user, mqtt_pass)) 
  {
       Serial.println(F("Connected to MQTT broker"));
       Serial.println(F("Subscribe to your mqtt_topic now"));
  }

}

void loop() {
  // put your main code to runin loop
  client.loop();
}
