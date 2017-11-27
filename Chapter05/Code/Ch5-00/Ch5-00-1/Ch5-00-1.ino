
#define PIN_PIR  D7             
int pirState = LOW;             
int current_value = 0;                    
 
void setup() {
  pinMode(PIN_PIR, INPUT);     
  Serial.begin(115200);
}
 
void loop(){
  current_value = digitalRead(PIN_PIR);  
  if (current_value == HIGH) {            
    if (pirState == LOW) {      
      Serial.println(F("Sensor detected motion!"));
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } 
  else 
  {   
    if (pirState == HIGH){
      Serial.println(F("Motion ended.."));
      pirState = LOW;
    }
  }
}
