// defines pins
#define stepPinMotor1 6
#define dirPinMotor1 5
#define stepPinMotor2 8
#define dirPinMotor2 7
// #define stepPinMotor3 38
// #define dirPinMotor3 42

#define MS1 8
#define MS2 9
#define MS3 10
 
void setup() {
  // Sets the two pins as Outputs
  Serial.begin(9600);
  pinMode(stepPinMotor1,OUTPUT); 
  pinMode(dirPinMotor1,OUTPUT);

  pinMode(stepPinMotor2,OUTPUT); 
  pinMode(dirPinMotor2,OUTPUT);

  // pinMode(stepPinMotor3,OUTPUT); 
  // pinMode(dirPinMotor3,OUTPUT);

  // pinMode(MS1,OUTPUT);
  // pinMode(MS2,OUTPUT);
  // pinMode(MS3,OUTPUT);

  // digitalWrite(MS1,LOW);
  // digitalWrite(MS2,LOW);
  // digitalWrite(MS3,LOW);
  digitalWrite(dirPinMotor1,HIGH); 
  digitalWrite(dirPinMotor2,LOW);
  // digitalWrite(dirPinMotor3,HIGH);
}
void loop() {// Enables the motor to move in a particular direction
  // Makes 200 pulses for making one full cycle rotation
  Serial.println("Going");
  for(int x = 0; x < 5000; x++) {
    digitalWrite(stepPinMotor1,HIGH); 
    digitalWrite(stepPinMotor2,HIGH); 
    // digitalWrite(stepPinMotor3,HIGH); 
    delayMicroseconds(700);    // by changing this time delay between the steps we can change the rotation speed
    digitalWrite(stepPinMotor1,LOW); 
    digitalWrite(stepPinMotor2,LOW); 
    // digitalWrite(stepPinMotor3,LOW); 
    delay(15); 
  }
  //delay(1000); // One second delay
  Serial.println("Changing");
  
  digitalWrite(dirPinMotor1,LOW); //Changes the rotations direction
  digitalWrite(dirPinMotor2,HIGH); 
  // // Makes 400 pulses for making two full cycle rotation
  for(int x = 0; x < 5000; x++) {
    digitalWrite(stepPinMotor1,HIGH);
    digitalWrite(stepPinMotor2,HIGH);
    delayMicroseconds(700);
    digitalWrite(stepPinMotor1,LOW);
    digitalWrite(stepPinMotor2,LOW);
    delay(15);
  }
  // delay(1000);

  digitalWrite(dirPinMotor1,HIGH);
  digitalWrite(dirPinMotor2,LOW);
}