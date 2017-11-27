#include <ESP8266WiFi.h>
#include <PubSubClient.h>


const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_SERVER_IP";
const char* mqtt_user = "joe";
const char* mqtt_passwd = "joe1234";
const int mqtt_port = 1883;
#define OUTDOOR_LIGHT  12

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

void setup() {
  pinMode(OUTDOOR_LIGHT, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    WiFi.begin(ssid, password);
    
    Serial.print(".");
    delay(5000);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
    digitalWrite(OUTDOOR_LIGHT, LOW);   // Turn the LED off 
  } else {
    digitalWrite(OUTDOOR_LIGHT, HIGH);  // Turn the LED on
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_passwd)) 
    {
      Serial.println("connected");
      client.subscribe("outdoor/light");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    String light_state;

    if(digitalRead(OUTDOOR_LIGHT) == HIGH)
      light_state = "ON";
    else
      light_state = "OFF";

  
    Serial.print("Publish message: ");
    Serial.println(light_state);
    client.publish("outdoor/light/status", light_state.c_str());
  }
}

