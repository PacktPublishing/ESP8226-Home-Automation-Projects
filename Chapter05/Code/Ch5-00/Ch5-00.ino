
#define PIN_PIR  D7  //GPIO13
#define REALY_PIN D6 //GPIO12           
int pirState = LOW;             
int current_value = 0;                    
 
void setup() {
  pinMode(PIN_PIR, INPUT); 
  pinMode(RELAY_PIN, OUTPUT);    
  Serial.begin(115200);
}
 
void loop(){
  current_value = digitalRead(PIN_PIR);  
  if (current_value == HIGH) {            
    if (pirState == LOW) {      
      Serial.println(F("Sensor detected motion!"));
      // We only want to print on the output change, not state
      pirState = HIGH;
      digitalWrite(RELAY_PIN,HIGH);
    }
  } 
  else 
  {   
    if (pirState == HIGH){
      Serial.println(F("Motion ended.."));
      pirState = LOW;
      digitalWrite(RELAY_PIN,LOW);
    }
  }
}
