 #include <Servo.h>

 Servo myservo;  // create servo object to control a servo
 
void setup() {
  myservo.attach(9);
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  
  if(Serial.available() > 0){       //wait for jump command from PC
   Serial.read();

  myservo.write(130);              //move the servo horn to press the button  
  delay(230);
  myservo.write(90);               //return to release the button
  delay(150);
  while(Serial.available() > 0)  Serial.read();           //to remove any bending commands
    
  }
  
}
