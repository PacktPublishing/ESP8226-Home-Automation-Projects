#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
 
#define PanasonicAddress      0x4004     
#define PanasonicPower        0x100BCBD  // Panasonic Power button

#define wifi_ssid "WLAN_19"
#define wifi_password "12345678"

#define mqtt_server "46.101.223.98"
#define mqtt_user "CATA"
#define mqtt_password "CATA"
#define mqtt_port 1888
#define IR_PIN 12

#define ir_topic "/62/ir/command"
#define device_status_topic "/62/device/status"
// Callback function header
void rx_mqtt_callback(char* topic, byte* payload, unsigned int length);

#define DEBUG false
#define Serial if(DEBUG)Serial
#define DEBUG_OUTPUT Serial

IRsend irsend(IR_PIN); //an IR led is connected to GPIO pin
WiFiClient espClient;
PubSubClient client(mqtt_server, mqtt_port, rx_mqtt_callback,espClient);

StaticJsonBuffer<512> jsonDeviceStatus;
JsonObject& jsondeviceStatus = jsonDeviceStatus.createObject();

char dev_name[50]; 
char json_buffer_status[512];
char my_ip_s[16];

void setup_wifi() 
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(dev_name, mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void rx_mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  //reserve space for incomming message
  StaticJsonBuffer<256> jsonRxMqttBuffer;
  int i = 0;
  char rxj[256];
  Serial.println(dev_name); Serial.print("Topic:");Serial.println(topic);
  for(i=0;i<length;i++)
  {
    rxj[i] = payload[i];
  }

  Serial.println(rxj);
  JsonObject& root = jsonRxMqttBuffer.parseObject(rxj);
  if (!root.success())
  {
    Serial.println(F("parseObject() failed"));
    return;
  }

  const char* device_name  = root["device_name"];
  const char* type         = root["type"];
  const char* value        = root["value"];

  Serial.println(device_name); 
  Serial.println(type); Serial.println(value);

  sendIR();
 
  return;
}

void sendIR()
{
    int i = 0;
    Serial.print("sendIR for 2 sec");
    for(i=0;i<20; i++)
    {      
      irsend.sendPanasonic(PanasonicAddress,PanasonicPower); // This should turn your TV on and off
      delay(100);
    }
}
void setup() 
{
  delay(1000);
  irsend.begin();
  Serial.begin(115200); 
  sprintf(dev_name, "ESP_%d", ESP.getChipId());
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.connect(dev_name, mqtt_user, mqtt_password);
  client.subscribe(ir_topic);
  if (!client.connected()) 
  {
    reconnect();
  } 
}

void loop() {
  client.loop();
}
