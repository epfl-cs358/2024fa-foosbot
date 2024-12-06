#include "AccelStepper.h"
#include <SoftwareSerial.h>
#define EN 8
#define Y_DIR 6
#define Y_STP 3
// 1000 units is for the full range side to side
const float stepsPerMM = 400.0; // Adjust based on your setup ==> nb of steps required to move 1mm
SoftwareSerial wemosSerial(-1, 10); 

// AccelStepper set up 
AccelStepper stepperY(1,Y_STP, Y_DIR);
long TravelY;

const float stepsPerRevolution = 3200.0; 
const float degreesPerStep = 360.0 / stepsPerRevolution; 
int acceleration = 5;   

//BEGIN value
//go to the side till it hits the sensor 
void setBeginning(){
  // max is 1000 units
  int full_distance = 1000;
  moveSide(full_distance); // Move in the positive direction

  // If sensor on pin 12 was triggered, go back to the middle
  if (digitalRead(12) == LOW) {
    // Calculate the middle position
    int middlePosition = full_distance/2 ;

    // Backtrack to the middle
    moveSide(-500); // Move back by 500 units (middle of the range)
    // Update the current position to middle
    stepperY.setCurrentPosition(middlePosition); // now this is 0 position
    Serial.println("Sensor triggered! Position set to the middle.");
  } 
  stepperY.setCurrentPosition(0); // Reset the logical position to 0 at the middle
  Serial.println("Middle position set as 0 point.");


}
// MOVE <value>
// <0 values to go far from the side motor
// >0 values to come close to the side motor 
// for 7cm mov range of player, 350 unit is good
// original 17 cm movement range
void moveSide(int value){
  int y =  stepperY.currentPosition(); 
  stepperY.move(value);             // Set the final position 
  //for coordinate value thats been going far from the side motor >0, controlled by the pin 11
  if(value>0){
    while (stepperY.distanceToGo() != 0 && digitalRead(11) == LOW) {
      stepperY.run();                  // Continuously move toward the target
    }
    stepperY.stop();
  }
  else {
    while (stepperY.distanceToGo() != 0 && digitalRead(12) == LOW) {
      stepperY.run();                  // Continuously move toward the target
    }
    stepperY.stop();
  }
  Serial.print("Moved Y by ");
  Serial.println(value);
}

//INITIALY 
void returnToInitialPositionSide() {
  stepperY.moveTo(0);            // Command to move to position 0
  stepperY.runToPosition();      // Execute the movement
  Serial.print("Returned to initial   X position ");

}

//ROTATE <angle> 
// <0 value moves forward 
// >0 moves bavkwards
// angle = +-12 == 180 deg rotation
void rotateByAngle(float angle) {
  float rad_angle = angle * 3.14/180;
  // MOVE <angle>
  String command = "MOVE " + String(rad_angle);
  wemosSerial.println(command);
  Serial.print("Rotated by ");
  Serial.println(angle);
}
//

void executeInterpreter(String command) {
  char cmd[20];
  int value;
  sscanf(command.c_str(), "%s %d", cmd, &value);

  if (strcmp(cmd, "MOVE") == 0) {
    moveSide(value);
  } else if (strcmp(cmd, "ROTATE") == 0) {
    rotateByAngle((float)value); // Rotate by angle
  } else if (strcmp(cmd, "INITIALX") == 0) {
    returnToInitialPositionSide();
  } else if(strcmp(cmd, "INITIALY")== 0){
    rotateByAngle(0);
  }else if(strcmp(cmd, "BEGIN")==0){
    setBeginning();
  } else {
    Serial.println("Invalid Command!");
  }
}
// full set up if i was in one corner 1000 unit mo

void setup()
{
  Serial.begin(9600); 
  wemosSerial.begin(9600);
  stepperY.setMaxSpeed(5000.0); // set max speed of the stepper , slower to get better accuracy
  stepperY.setAcceleration(5000.0); //set acceleration of the stepper 
  pinMode(EN, OUTPUT); 
  digitalWrite(EN, LOW); // Enable motor driver
  //stepperY.setCurrentPosition(0);  // initialize the current position im at to be 0 

  pinMode(11, INPUT); // sensor far from the  sideway motor, responsible for the pos unit move control 
  pinMode(12, INPUT); // sensor close to the sideway motor, responsible for the pos unit move control  

  //test for side motion
  //moveSide(-350);
  //returnToInitialPositionSide();
  // test for rotary motion
  //rotateByAngle(-12);
  //rotaryInitialPositionRotation();
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // Read command from serial
    command.trim();                                // Remove whitespace
    executeInterpreter(command);                      // Execute parsed command
  }
  
}
