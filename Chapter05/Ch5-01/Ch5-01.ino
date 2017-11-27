#define BLYNK_PRINT Serial    
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>


char app_token[] = "TOKEN_FROM_EMAIL";

SimpleTimer timer;

char ssid[] = "WIFI_SSID";
char pass[] = "WIFI_PASS";

int state;
int counter=0;
int flag=1;

WidgetLED led1(V1);
WidgetLED pirLED(V2);

void timer_ev()
{
  counter = counter+1;
  
  if(counter==5)
  {
    pirLED.off();
    counter=0;
    flag = 1;
  }

  int pirStatus = digitalRead(D7);
  if (pirStatus) 
  {
    if (flag == 1)
    {
        Serial.println(F("Sensor detected motion!")); 
        Blynk.email("bcatalin@gmail.com","Subject: Security alert!", "Movement detected!");
        Blynk.notify("Security alert! Someone is in the house!");
        digitalWrite(D4, LOW);
        led1.on();
        pirLED.on();
        flag=2;
      
    }
  }
}




void setup()
{
  Serial.begin(115200);
  Blynk.begin(app_token, ssid, pass);
  pinMode(D4, OUTPUT);
  timer.setInterval(1000L, timer_ev);
}

BLYNK_WRITE(V0)
{
  state = param.asInt();
  if (state == 1){
    digitalWrite(D4, LOW);
    led1.on();
  }
  else {
    digitalWrite(D4, HIGH);
    led1.off();
  }
}


void loop()
{
  Blynk.run();
  timer.run(); 
}
