/**************************************************
 * Catalin Batrinu bcatalin@gmail.com 2016
 * 
 * 
 * VERSION: 1.0.44 4/10/2017
 * 
***************************************************/

#include "mydatatypes.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <Timer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "FS.h"

//new NTPClient
#include <TimeLib.h>
#include <NtpClientLib.h>

//voice
#include <WiFiUdp.h>
#include <functional>
#include "switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"


//===================================to be deleted
int clear_g = 0;
//================================================

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event

UpnpBroadcastResponder upnpBroadcastResponder;
Switch *office = NULL;

extern "C" {
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "spi_flash.h"
}

//===== GPIO
#define PLUG_PIN 12
SYSCFG sysCfg;
SECTOR3_DATA sector3Data;

#define INFO Serial.printf
#include "SntpInit.h"
#include "Settings.h"

//--------------------------------------------------  ---------------------------- -----------------------------

//int send_mqtt_status = 0; //send or not a MQTT status message
#define DEV_TYPE  "plug"

int uptime=0;
uint8_t mqtt_fail = 0;

//============NEW BUTTON
unsigned long timerxs = 0, timersec = 0;

bool trigger_cron_upd = false;
int state = 0;

const byte interruptPin = 4;
volatile byte interruptCounter = 0;
int numberOfInterrupts = 0;

void ICACHE_RAM_ATTR handleInterrupt() {
  interruptCounter++;
}

#define MQTT_CLIENT_ID         "ESP_%06X"

#define DEBUG true
//#define Serial if(DEBUG) Serial
#define DEBUG_OUTPUT Serial

//==== WEB CONFIG ======
const char *ssid = "ESPap";
const char *passphrase = "12345678";
ESP8266WebServer server(80);
bool apMode = false;
String st; //html part of code
String content; //page content
int statusCode; //404, 200 etc

//===== EXTERN STUFF
extern SYSCFG sysCfg;

//===== OTHER
char dev_name[50];
bool setweb = false;
Timer every_minute_timer;
bool every_minute_timer_flag = false;
bool send_update = false;
bool ota_update = false;
bool writeCronData = false;

//TBR:
int msg_count = 1;

//===== MQTT
// Callback function header for receiving MQTT messages from broker
void rx_mqtt_callback(char* topic, byte* payload, unsigned int total);

char json_buffer_cron[1024];
//WiFiClientSecure espClient;
WiFiClient espClient;
PubSubClient mqttClient;
const char* userver  = "";
const char* utopic   = "";
int uport    = 1883;

//---------------------every device subscribe topics --
#define sub_status_update "/status/update"
#define sub_identity      "/status/identity"

//---------------------PLUG specific-------------------
#define sub_plug  "/plug/command"

/******************************************************
 *              Publishing on this topics
 ******************************************************/
#define publish_minute_state "/plug/status"
#define device_status_topic  "/device/status"

/********************************************************************
* FunctionName: stopPlug
* Description : Stop a plug. Put its GPIO to LOW
* Parameters  : None
* Returns     : None                                                     
*********************************************************************/
void ICACHE_FLASH_ATTR stopPlug()
{
  digitalWrite(PLUG_PIN, LOW);
}



//=============== TIMERS ========================
/********************************************************************
* FunctionName: everyMinuteTimer
* Description : Get the time very minute and check the cron for a match
* Parameters  : None
* Returns     : None                                                     
*********************************************************************/
void everyMinuteTimer()
{
  every_minute_timer_flag = true;
}
/********************************************************************
* FunctionName: everyMinuteTimerFn
* Description : 
* Parameters  : None
* Returns     : None                                                     
*********************************************************************/
void everyMinuteTimerFn()
{
  uptime++;
  sendOneMQTTUpdate();           
  INFO("everyMinuteTimerFn HEAP: %d msg_count:%d WiFi.connected:%d MQTT.connected:%d apMode:%d\n", 
                  ESP.getFreeHeap(), 
                  ++msg_count, 
                  WiFi.status(), 
                  mqttClient.connected(), 
                  apMode );
}

/********************************************************************
* FunctionName: rx_mqtt_callback 
* Description : 
* Parameters  : None
* Returns     : None                                                     
*********************************************************************/
void startMQTTService()
{
    /*********************************************
     *  SET UP THE MQTT, CONNECT AND SUBSCRIBE 
     *********************************************/
    mqttClient.setClient(espClient); //????   
    mqttClient.setServer( sysCfg.mqtt_host, 1883);//atoi(sysCfg.mqtt_port) );
    mqttClient.setCallback(rx_mqtt_callback);

    sprintf(dev_name, MQTT_CLIENT_ID, ESP.getChipId());
    Serial.printf("Connecting to MQTT: %s p:%s u:%s p:%s\n", sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.mqtt_user, sysCfg.mqtt_pass);
    while (!mqttClient.connected()) 
    {
//      Serial.println(F("Check first Wifi, maybe is down due to Wifi issues, then check MQTT."));
//      mqtt_fail++;
//      if ( WiFi.status() != WL_CONNECTED )
//      {
//        Serial.println(F("MQTT is down but also Wifi. Let's fix first Wifi connecction!"));
//        return; //===>
//      }
//      Serial.println(F("Wifi is up. Maybe the MQTT is down..."));
//
//      if(mqtt_fail == 10)
//      {
//        //give up for now will retry later
//        return;
//      }
      INFO("Attempting MQTT connection..DEV:%s USERVER:%s PORT:%s USER:%s PASS:%s\n",dev_name,sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.mqtt_user, sysCfg.mqtt_pass);       
      mqttClient.connect(dev_name, sysCfg.mqtt_user, sysCfg.mqtt_pass);
      if (mqttClient.connected())  
      {                                 
        Serial.println(F("connected to MQTT server\n")); 
        //digitalWrite(15, LOW);
        mqtt_fail = 0;
      /*********************************************************************
       *  MQTT IS CONNECTED NOW SUBSCRIBE TO TOPICS
       *********************************************************************/

       //generic topics to subscribe on
         char status_update[80];
         strcpy(status_update,"/");
         strcat(status_update,sysCfg.mqtt_topic);
         strcat(status_update,sub_status_update);
         mqttClient.subscribe(status_update);
         //mqttClient.loop();
         
         char identity[80];
         strcpy(identity,"/");
         strcat(identity,sysCfg.mqtt_topic);
         strcat(identity,sub_identity);
         mqttClient.subscribe(identity);
         //mqttClient.loop(); //sub_plug
         
         char subscrib_plug[80];
         strcpy(subscrib_plug,"/");
         strcat(subscrib_plug,sysCfg.mqtt_topic);
         strcat(subscrib_plug,sub_plug);
         mqttClient.subscribe(subscrib_plug);
         //mqttClient.loop(); 

         Serial.println(F("****** SUBSCRIBED TO TOPICS ********************"));
         Serial.println(status_update);
         Serial.println(identity);
         Serial.println(subscrib_plug);
         Serial.println(F("************************************************"));
         return;//==============>    
      } 
//      else 
//      {
//        Serial.println(mqttClient.state());
//        if ( WiFi.status() != WL_CONNECTED )
//        {
//             WiFi.disconnect();
//           Serial.println(F("WiFi is not connected. try a reconnect try:"));
//           WiFi.begin(sysCfg.sta_ssid, sysCfg.sta_pwd); //try to reconnect
//           delay(5000);
//        }
//        else
//           Serial.println(F("WiFi is connected but MQTT is DOWN"));   
//      }
//      mqttClient.disconnect();
//      delay(5000);
  }//end while MQTT connected.  
}


void sendMQTTUpdate()
{

}



/********************************************************************
* FunctionName: my_atoi 
* Description : Ascii to integer
* Parameters  : ascii
* Returns     : integer                                                     
*********************************************************************/
int  my_atoi(char *p) {
    int k = 0;
    while (*p) {
        k = (k<<3)+(k<<1)+(*p)-'0';
        p++;
     }
     return k;
}

/********************************************************************
* FunctionName: rx_mqtt_callback 
* Description : 
* Parameters  : None
* Returns     : None                                                     
*********************************************************************/
/******************************************************************
 *  M A I N   R E C E I V E   F U N C T I O N   F O R   M Q T T
 ******************************************************************/
void rx_mqtt_callback(char* topic, byte* payload, unsigned int msg_length)
{
 // Serial.println(F("=====> RX MQTT packet - rx_mqtt_callback"));                  //=======>
 // Serial.println(msg_length);
 // Serial.println(topic);

 char status_update[80];
 strcpy(status_update,"/");
 strcat(status_update,sysCfg.mqtt_topic);
 strcat(status_update,sub_status_update);

 char identity[80];
 strcpy(identity,"/");
 strcat(identity,sysCfg.mqtt_topic);
 strcat(identity,sub_identity);
 
 char subscrib_plug[80];
 strcpy(subscrib_plug,"/");
 strcat(subscrib_plug,sysCfg.mqtt_topic);
 strcat(subscrib_plug,sub_plug);

 Serial.println(F("****** CHECK FOR TOPICS ********************"));
 Serial.println(status_update);
 Serial.println(identity);
 Serial.println(subscrib_plug);
 Serial.println(F("************************************************"));
 if ( (strcmp(topic, status_update)  == 0) || (strcmp(topic, identity)  == 0) )
 { 
    // TO DO: identity case should be treated here. If the user will press the Identity button
    // on the mobile app, the connected status led will start to blink 5 time per second.
    // this feature is good when you have multiple devices of the same type.

    //For now on any message received on the identity or status update the device will send an update
    send_update = true;
    return;    
  }

  int i = 0;
  char rxj[1024]; //TO DO: allocate jsut the size of the message
 
  for(i=0;i<msg_length;i++)
  {
    rxj[i] = payload[i];
  }
  Serial.println(rxj);
  StaticJsonBuffer<1024> jsonRxMqttBuffer;
  
  JsonObject& root = jsonRxMqttBuffer.parseObject(rxj);
  if (!root.success())
  {
      Serial.println(F("ERROR: on decoding. Return."));
      return;
  }

  /*******************************************************************
   *  RX command to start or stop the plug
   *******************************************************************/
  else if(strcmp(topic, subscrib_plug) == 0) //RX command
  {  
    Serial.print("Command topic: "); Serial.println(subscrib_plug);
    const char* rx_device_name  = root["device_name"];
    const char* type            = root["type"];
    if(strcmp(rx_device_name, dev_name) == 0)
    {
        int state = root["state"];
        digitalWrite( PLUG_PIN, state ); //GPIO 15 need a function to do conversion for me
        //send an update to reflect the current state for devices
        send_update = true;
    } 
  }  
} //end of rx_mqtt_callback function

/********************************************************************
* FunctionName: officeLightsOn 
* Description : 
* Parameters  : None
* Returns     : None                                                     
*********************************************************************/
void officeLightsOn() {
    Serial.print("Switch 1 turn on ...");
    digitalWrite(PLUG_PIN, HIGH);
}
/********************************************************************
* FunctionName: officeLightsOff 
* Description : 
* Parameters  : None
* Returns     : None                                                     
*********************************************************************/
void officeLightsOff() {
    Serial.print("Switch 1 turn off ...");
    digitalWrite(PLUG_PIN, LOW);
}

/********************************************************************
* FunctionName: setup_wifi 
* Description : 
* Parameters  : None
* Returns     : None                                                     
*********************************************************************/
void setup_wifi() 
{
   delay(10);
   WiFi.mode(WIFI_STA);
   WiFi.begin(sysCfg.sta_ssid, sysCfg.sta_pwd);

   while (WiFi.status() != WL_CONNECTED) //
   {
     //WiFi.persistent(false);
     //WiFi.mode(WIFI_OFF);
     WiFi.disconnect();    //new============== for disconnect issues.     
     //WiFi.mode(WIFI_STA);
     WiFi.begin(sysCfg.sta_ssid, sysCfg.sta_pwd); //try to reconnect
   
     Serial.println(F("...try WiFi.begin again..."));
     delay(5000);
   }

  /***************************************************
   * GET THE IP AND MAC ADDRESS 
   ***************************************************/ 
   String my_ip  = WiFi.localIP().toString();
   String my_mac = WiFi.macAddress();
  
   /****************************************************
   * GET THE DATA FROM EXTERNAL SERVICE
   ****************************************************/
    WiFiClient clientData;
    const int httpPort = 80;
    const char *host = "iotcentral.eu";
    //---------------------make MD5
 //   MD5Builder md5;
 //   md5.begin();
 //   md5.add(sysCfg.mqtt_pass); //3417973cd67f37b077e56b82f0cc306f
 //   md5.calculate();

    //Other method for MD5
    unsigned char* hash=make_hash(sysCfg.mqtt_pass);
    char *md5str = make_digest(hash, 16);
    free(hash);
    //Serial.println(md5str);

    //-------construct the URL that will be sent to the SERVER to get the JSON with the broker connections
    String new_url = "/getme/iot?u="; 
    new_url.concat(sysCfg.mqtt_user);
    new_url.concat("&p="); new_url.concat(md5str); 
    new_url.concat("&t="); new_url.concat(DEV_TYPE);
    new_url.concat("&i="); new_url.concat(my_ip);
    new_url.concat("&m="); new_url.concat(my_mac);
    new_url.concat("&v="); new_url.concat(sysCfg.sw_version);
    Serial.println(new_url);
    //Give the Memory back to the System if you run the md5 Hash generation in a loop
    free(md5str);
  
    if (!clientData.connect(host, httpPort)) {
        return;
    } 

    // This will send the request to the server
    clientData.print(String("GET ") + new_url + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" + 
              "Connection: close\r\n\r\n");
    delay(10);

    //Wait up to 10 seconds for server to respond then read response
    int i=0;
    while((!clientData.available()) && (i<1000))
    {
      delay(10);
      Serial.println(F("connecting to iotcentral.eu to retrieve data ....."));
      i++;
    }
    String html_header;
    // Read all the lines of the reply from server and print them to Serial
    while(clientData.available()){
         html_header = clientData.readStringUntil('\r');
         //Serial.print(html_header);
    }

   /****************************************************
    * decode the data received from iotcentral.eu server
    ****************************************************/
    int start_json = html_header.indexOf('{');
    int stop_json = html_header.indexOf('}', start_json);
    String myJSON = html_header.substring(start_json, stop_json+1);
    StaticJsonBuffer<1024> jsonRxMqttBuffer;
    JsonObject& root = jsonRxMqttBuffer.parseObject(myJSON);
    if (!root.success())
    {
      //Serial.println("parseObject() failed");
      //ESP.restart();
      return;
    }
    userver  = root["userver"];
    uport    = root["uport"];
    utopic   = root["utopic"];
    
   
    strcpy(sysCfg.mqtt_host, root["userver"]);
    strcpy(sysCfg.mqtt_topic, root["utopic"]);  
     
    //Serial.println(sysCfg.mqtt_topic);  
       
                   //sysCfg.mqtt_port = itoa(uport);  
    itoa(uport, sysCfg.mqtt_port,10);
    Serial.println(sysCfg.mqtt_port);
    clientData.stop();  
    delay(20);
    startMQTTService();
    
  /********************************************************
   * START AN ONE MINUTE TIMER TO CALL everyMinuteTimer
   ********************************************************/
   every_minute_timer.every(60 * 1000 , everyMinuteTimer); // ORIG EVERY MINUTE
   
   delay(100);
   //start voice
   upnpBroadcastResponder.beginUdpMulticast();
   office = new Switch( String( sysCfg.alexa_name).c_str(), 80, officeLightsOn, officeLightsOff);
   upnpBroadcastResponder.addDevice(*office);

   // start NTP Cleint
   NTP.begin("pool.ntp.org", atoi(sysCfg.time_zone), true);
   NTP.setInterval(63);    
}




//========================== WEB ===================================

void createWebServer(int webtype)
{
  if ( webtype == 1 ) 
  {
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        //pattern='^[a-zA-Z0-9_]+$' la USerName
        content =
            "<!doctype html><html lang='en'><head><title>Device Configration</title>\<meta name='viewport' content='width=device-width, initial-scale=1' /></head>\
             <body><h2>New device config</h2>\
             <form method='GET' action='setting'>\
               <h3>Wi-Fi</h3>\
                SSID: <input type='text' name='ssid' placeholder='SSID'  required/>\
                <br/>\
                Password<input type='text' name='pass' placeholder='Password'  required/>\
                <br/>\
                <h3>Account details</h3>\
                <input type='email' name='mqtt_user' placeholder='User Name'   required />\
                <br/>\
                <input type='text' name='mqtt_pass' placeholder='Password'  required />\
                <br/>\
                <input type='text' name='alexa_name' placeholder='Alexa name'  required />\
                <br/>\
                <input type='text' name='time_zone' placeholder='Time zone'  required\
                     pattern='^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$'/>\
                <br/>\
                <input type='submit' value='Save config'/>\
             </form></body></html>";        
        
             server.send(200, "text/html", content);  
    });
    server.on("/setting", []() {
        String qsid            = server.arg("ssid");
        String qpass           = server.arg("pass");
        String mqtt_pass_web   = server.arg("mqtt_pass");
        String mqtt_user_web   = server.arg("mqtt_user");
        String alexa_name_web =  server.arg("alexa_name");
        String time_zone_web = server.arg("time_zone").c_str();
        
        if (qsid.length() > 0 && qpass.length() > 0) 
        {
          
          strcpy(sysCfg.sta_ssid, String(qsid).c_str());
          strcpy(sysCfg.sta_pwd, String(qpass).c_str());
          strcpy(sysCfg.mqtt_user, String(mqtt_user_web).c_str());
          strcpy(sysCfg.mqtt_pass, String(mqtt_pass_web).c_str());
          strcpy(sysCfg.alexa_name, String(alexa_name_web).c_str());
          
          strcpy(sysCfg.time_zone, String(time_zone_web).c_str());
          strcpy(sysCfg.sw_version, "1039");
          /*Settings.*/
          settings_save();            
          content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
          statusCode = 200;
          server.send(statusCode, "application/json", content);
          Serial.println(F("[settings.c] settings_save: Done saving! restart the ESP\r\n"));
          delay(300);
          ESP.restart();
          delay(2000);
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;          
          server.send(statusCode, "application/json", content);
        }
        
    });
  }
}


void launchWeb(int webtype) {
  createWebServer(webtype);
  // Start the server
  server.begin();
}

void setupAP(void) 
{
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.softAP(ssid);//, passphrase, 6);
  Serial.println(F("softap"));
  launchWeb(1);
}


void setWebService()
{
   settings_load();
   if(settings_valid() == true)
   {
    //Connect to WiFi and then to the MQTT broker
    setup_wifi();
    apMode = false;  

     /****************************************************
     * SNTP
     ****************************************************/
     SNTP_Init(atoi(sysCfg.time_zone) );
     //delay(1000);
   }
   else
   {
    Serial.println(F("settings not valid start in AP mode"));
    apMode = true;
    setupAP();
    server.begin();
   }  
}


//========================== MQTT ==================================
void sendMQTTstatus()
{
   StaticJsonBuffer<1024> jsonRxMqttBuffer; //new on 1.0.43

   JsonObject& jsonStatus = jsonRxMqttBuffer.createObject();
   
   jsonStatus["device_name"] = dev_name; 
   jsonStatus["type"]        = DEV_TYPE;
   jsonStatus["state"]       = digitalRead( PLUG_PIN );
   
   os_memset(&json_buffer_cron, 0x00, sizeof(json_buffer_cron)); //new
   jsonStatus.printTo(json_buffer_cron, sizeof(json_buffer_cron));
   char pub_state_every_min[80];
   strcpy(pub_state_every_min, "/");
   strcat(pub_state_every_min, sysCfg.mqtt_topic);
   strcat(pub_state_every_min, publish_minute_state);
 
   Serial.println(pub_state_every_min);
   mqttClient.publish(pub_state_every_min,  json_buffer_cron, false);// , false);  
}


void sendMQTTDeviceDetails()
{
   StaticJsonBuffer<1024> jsonRxMqttBuffer; //new on 1.0.43   
   JsonObject& jsondeviceStatus = jsonRxMqttBuffer.createObject();
   char my_ip_s[16];
      
   IPAddress my_ip_addr = WiFi.localIP();  
   sprintf(my_ip_s, "%d.%d.%d.%d", my_ip_addr[0],my_ip_addr[1],my_ip_addr[2],my_ip_addr[3]);
   jsondeviceStatus["device_name"] = dev_name;
   jsondeviceStatus["type"]        = DEV_TYPE; 
   jsondeviceStatus["ipaddress"]   = my_ip_s;
   jsondeviceStatus["alexa"]       = sysCfg.alexa_name;
   jsondeviceStatus["bgn"]         = 3;
   jsondeviceStatus["sdk"]         = ESP.getSdkVersion();
   jsondeviceStatus["version"]     = sysCfg.sw_version;
   jsondeviceStatus["uptime"]      = NTP.getUptimeString();//uptime;//uptime;
   
   
   os_memset(&json_buffer_cron, 0x00, sizeof(json_buffer_cron)); //new  
   jsondeviceStatus.printTo(json_buffer_cron, sizeof(json_buffer_cron)); 
   char dev_status_topic[80];
   strcpy(dev_status_topic,"/");
   strcat(dev_status_topic,sysCfg.mqtt_topic);
   strcat(dev_status_topic,device_status_topic);
   
   //sprintf( dev_status_topic, "/%s/%s", sysCfg.mqtt_topic, device_status_topic);
   
   mqttClient.publish(dev_status_topic, json_buffer_cron, false); //to /62/device/status  
}

void sendOneMQTTUpdate()
{
  /***********************************************************************
  * Not to have all the responses in the same time since some of them
  * are quite big ( cron_records ) do a random delay between 0 and 200 ms
  ************************************************************************/
  if((WiFi.status() == WL_CONNECTED) && (mqttClient.connected() == true))
  {
    sendMQTTstatus();
    mqttClient.loop();
    delay(0);
    sendMQTTDeviceDetails();
    mqttClient.loop();
    delay(0);
  }    
}

void processSyncEvent(NTPSyncEvent_t ntpEvent) {
  if (ntpEvent) 
  {
    //Serial.print("Time Sync error: ");
    if (ntpEvent == noResponse)
    {
     // Serial.println("NTP server not reachable");
    }
    else if (ntpEvent == invalidAddress)
    {
     // Serial.println("Invalid NTP server address");
    }
  }
  else {
    //Serial.print("Got NTP time: ");
    //Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
  }
}
//=========================== SETUP =================================
/********************************************************
 *          S E T U P
 ********************************************************/
void setup() 
{
  // put your setup code here, to run once:
   Serial.begin(115200); delay(10);

   /*****************************************************
    * Start SPI File system
    ****************************************************/
   if (!SPIFFS.begin()) {
      Serial.println("ERROR: Failed to mount file system");
      return;
   }

   if(clear_g)
   {
     WiFi.disconnect();
     Serial.print(F("\n\nFormating..."));
     SPIFFS.format();
     Serial.println(F("done."));
   }
   
   randomSeed(analogRead(A0));
   pinMode(12, OUTPUT); digitalWrite(12, LOW);//green 
   Serial.println(F("======== 1.0.37  ==============="));
    /******************************************************************
     * After a power outage all the devices will start in the same
     * time, so the broker will receive a lot of messages in the same
     * time. Do a small delay in here to have an uniform distribution 
     * on messages.
     ******************************************************************/
    delay(random(0,60));
    setWebService();    
    
}
/********************************************************
 *          L O O P
 ********************************************************/
int wifi_counter = 0;
void loop() 
{
//  if(!apMode)
//   while (WiFi.status() != WL_CONNECTED) 
//   {
//     wifi_counter++;
//     if(wifi_counter == 10)
//     {
//       Serial.println(F("Tried to recover wifi for 50s. Give up now. Try later"));       
//       break;
//     }
//     WiFi.persistent(false);
//     WiFi.mode(WIFI_OFF);
//     WiFi.disconnect(); 
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(sysCfg.sta_ssid, sysCfg.sta_pwd); //try to reconnect
//   
//     Serial.println(F("...try WiFi.begin again...loop")); 
//     delay(5000);
//   }
//  wifi_counter=0;
  if(!mqttClient.connected() && (apMode == false) )
  {
    startMQTTService();
  }
   
  if(apMode)
  {
    server.handleClient();
  }

  if(every_minute_timer_flag == true)
  {
    every_minute_timer_flag = false;
    everyMinuteTimerFn();
  }

  if(send_update == true)
  {
    send_update = false;
    sendOneMQTTUpdate();
  }
  every_minute_timer.update();
  mqttClient.loop();

  static int i = 0;
  static int last = 0;

  if (syncEventTriggered) {
    processSyncEvent(ntpEvent);
    syncEventTriggered = false;
  }

  if ((millis() - last) > 5100) {
    last = millis();
    Serial.print(i); Serial.print(" ");
    Serial.print(NTP.getTimeDateString()); Serial.print(" ");
    Serial.print(NTP.isSummerTime() ? "Summer Time. " : "Winter Time. ");
    Serial.print("WiFi is ");
    Serial.print(WiFi.isConnected() ? "connected" : "not connected"); Serial.print(". ");
    Serial.print("Uptime: ");
    Serial.print(NTP.getUptimeString()); Serial.print(" since ");
    Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

    i++;
  }
  if (WiFi.status() == WL_CONNECTED ) //1.0.41
  {
    upnpBroadcastResponder.serverLoop();    
    office->serverLoop();
  }
}


 





