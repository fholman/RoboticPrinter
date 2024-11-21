// defines pins
#define stepPin 4
#define dirPin 3

#define MS1 8
#define MS2 9
#define MS3 10
 
void setup() {
  // Sets the two pins as Outputs
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);

  pinMode(MS1,OUTPUT);
  pinMode(MS2,OUTPUT);
  pinMode(MS3,OUTPUT);

  digitalWrite(MS1,HIGH);
  digitalWrite(MS2,LOW);
  digitalWrite(MS3,LOW);
  digitalWrite(dirPin,HIGH); 
}
void loop() {// Enables the motor to move in a particular direction
  // Makes 200 pulses for making one full cycle rotation
  for(int x = 0; x < 10; x++) {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(700);    // by changing this time delay between the steps we can change the rotation speed
    digitalWrite(stepPin,LOW); 
    delay(200); 
  }
  //delay(1000); // One second delay
  
  // digitalWrite(dirPin,LOW); //Changes the rotations direction
  // // Makes 400 pulses for making two full cycle rotation
  // for(int x = 0; x < 10; x++) {
  //   digitalWrite(stepPin,HIGH);
  //   delayMicroseconds(700);
  //   digitalWrite(stepPin,LOW);
  //   delay(250);
  // }
  // delay(1000);
}