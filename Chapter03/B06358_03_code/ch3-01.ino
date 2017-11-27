
#include "FS.h"

const byte interruptPin = 4;
volatile byte interruptCounter = 0;

void handleInterrupt() {
  interruptCounter++;
}

void setup() {
  Serial.begin(115200); delay(10);
  //GPIO 4 format SPIFFS
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
  
  if(SPIFFS.begin())
  {
    Serial.println(F("File system was mounted."));
    // open file for writing
    File my_file = SPIFFS.open("/my_file.txt", "w+");
    if (!my_file) {
      Serial.println("file open failed");
    }
    Serial.println(F("Writing to SPIFFS "));
    //print something to my_file.txt
    my_file.println("SPIFFS is cool!");
    //close now the file
    my_file.close();

    // open file for reading. Now I can use other File object    
    File f = SPIFFS.open("/my_file.txt", "r");
    if (!f) 
    {
       Serial.println(F("Failed to open my_file.txt"));
    } 
    //now read the file content      
    String s=f.readStringUntil('\n');      
    Serial.println(s);
    //closing the file now
    f.close();  
  }
  else
  {
    Serial.println(F("Failed to mount SPIFFS. Restart"));
    ESP.restart();
  }

}

void loop() 
{
  if(interruptCounter>0)
  {
    interruptCounter--;
    Serial.println(F("Formating the file system... Please wait!"));
    SPIFFS.format();  
    Serial.println(F("Done formating the file system."));  
    ESP.restart();  
  }  
} 
