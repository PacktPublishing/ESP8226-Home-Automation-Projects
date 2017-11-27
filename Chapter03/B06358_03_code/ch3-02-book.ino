#include <FS.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_PASS";
const char* mqtt_server = "YOUR_MQTT_SERVER";
const char* mqtt_user = "joe";
const char* mqtt_passwd = "joe1234";
const int mqtt_port = 1888;

#define RELAY_PIN  12
#define DHTTYPE DHT22
#define DHTPIN  4
#define GREEN_LED 15
#define RED_LED 13 

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE, 11); 
long lastMsg = 0;
float offset_temp = 0.4;
float desired_temp = 22.0;
float humidity, temp_f;  // Values read from sensor


void gettemperature() 
{
  int runs=0;
  do {
       //delay(2000);
       temp_f = dht.readTemperature(false);     
       humidity = dht.readHumidity();          

       if(runs > 0)
       {
           Serial.println("##Failed to read from DHT sensor! ###");
           //return;
       }
//       Serial.println(String(temp_f ).c_str());
//       Serial.println(String(humidity ).c_str());
       runs++;
    }
    while(isnan(temp_f) && isnan(humidity));
}


void setup() {
  pinMode(RELAY_PIN, OUTPUT);     
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  if(SPIFFS.begin())
  {
    Serial.println(F("File system was mounted."));
    //check to see if we have a desired temperature other then default one
    File f = SPIFFS.open("/config_temp.txt", "r");
    if (f) 
    {
      //now read the file content      
      String s=f.readStringUntil('\n');      
      Serial.println(s);
      desired_temp = s.toFloat();
      //closing the file now
      f.close();  
    } 
    else
      Serial.println(F("Failed to open my_file.txt"));
  }

}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    WiFi.begin(ssid, password);
    
    Serial.print(".");
    delay(5000);
  }
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  char rxj[20];
  int i;
  for(i=0;i<length;i++)
  {
    rxj[i] = payload[i];
  }  

  File my_file = SPIFFS.open("/config_temp.txt", "w+");
  if (!my_file) {
      Serial.println("file open failed");
  }
  Serial.println(F("Writing to config_temp.txt "));
  //print something to my_file.txt
  my_file.println(String(rxj).c_str());
  //close now the file
  my_file.close();
  desired_temp = String(rxj).toFloat();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    if (client.connect("ESP8266Client", mqtt_user, mqtt_passwd)) 
    {
      Serial.println(F("connected"));
      client.subscribe("thermostat/set");
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() 
{
  
  gettemperature();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    if((float)desired_temp - offset_temp >= (float)temp_f)
    {
      //Serial.println(F("Start heating..."));
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
    }
    else if((float)desired_temp + offset_temp <= (float)temp_f)
    {
      //Serial.println(F("Stop heating..."));
      digitalWrite(RELAY_PIN, LOW); 
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW); 
    }
    client.publish("thermostat/get", String(temp_f).c_str());
  }
}

