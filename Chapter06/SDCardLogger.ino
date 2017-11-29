#include <SPI.h>
#include <SD.h>
#include <DHT.h>

const int chipSelect = D8;
#define DHTTYPE DHT22
#define DHTPIN  4

#define DEV_TYPE  "dht"
DHT dht(DHTPIN, DHTTYPE, 22); // 11 works fine for ESP8266
float humidity, temp_f;  // Values read from sensor

void gettemperature() 
{
  int runs=0;
  do {
       delay(2000);
       temp_f = dht.readTemperature(false);     
       humidity = dht.readHumidity();          

       if(runs > 0)
           Serial.println("##Failed to read from DHT sensor! ###");
       runs++;
    }
    while(isnan(temp_f) && isnan(humidity));
}
void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  gettemperature();
}

void loop()
{
  // make a string for assembling the data to log:
  String dataString = "";


  gettemperature();

  dataString += String(temp_f);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) 
  {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

  delay(3000);
}
