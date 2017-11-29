
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define PIN_12 12

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "iotcentral.eu";
const char* mqtt_username = "email@email.com";
const char* mqtt_password = "*******";
#define MQTT_CLIENT_ID         "ESP_%06X"
#define BASE_TOPIC "c5c05219" //get it from iotcentral.eu 

char dev_name[11];

WiFiClientSecure espClient;

PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() 
{
  pinMode(PIN_12, OUTPUT);
  digitalWrite(PIN_12, LOW);
  sprintf(dev_name, MQTT_CLIENT_ID, ESP.getChipId());
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 8883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // First connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected. My IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  digitalWrite(PIN_12,HIGH);
  delay(50);
  digitalWrite(PIN_12,LOW);
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");

  // Switch on the LED on PIN_12 if an ON message was received on any topic
  if ((char)payload[0] == 'ON') {
    digitalWrite(PIN_12, HIGH);   // Turn the LED on 
  } 
  else if ( (char)payload[0] == 'OFF')
  {
    digitalWrite(PIN_12, LOW);  
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Start MQTT connection...");

    if (client.connect(dev_name, mqtt_username, mqtt_password)) {
      Serial.print("connected to MQTT broker"); 
      Serial.println(mqtt_server);
      client.subscribe(BASE_TOPIC"/#");
    } else {
      Serial.print("Failed to connect. Error code: ");
      Serial.println(client.state());
      Serial.println(" try again in 5 seconds");
      // Disconnect and wait 5 seconds before retrying
      client.disconnect();
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    Serial.println("Reconnect to the broker....");
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "Sending message #%ld", value);
    Serial.print("Send to MQTT broker message: ");
    Serial.println(msg);
    client.publish(BASE_TOPIC"/outTopic", msg);
  }
}
