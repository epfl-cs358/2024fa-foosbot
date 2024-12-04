#include "AccelStepper.h"
#define EN 8
#define Y_DIR 6
#define Y_STP 3

const float stepsPerMM = 400.0; // Adjust based on your setup ==> nb of steps required to move 1mm

// AccelStepper set up 
AccelStepper stepperY(1,Y_STP, Y_DIR);
long TravelY;

const float stepsPerRevolution = 3200.0; 
const float degreesPerStep = 360.0 / stepsPerRevolution; 
int acceleration = 5;    

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
}

void returnToInitialPositionSide() {
  stepperY.moveTo(0);            // Command to move to position 0
  stepperY.runToPosition();      // Execute the movement
}

// <0 value moves forward 
// >0 moves bavkwards
// angle = +-12 == 180 deg rotation

void rotateByAngle(float angle) {
  long steps = angle / degreesPerStep; // Convert angle to steps
  stepperY.move(steps);                 // Move relative to current position
  stepperY.runToPosition();            
  stepperY.stop();
}

void rotaryInitialPositionRotation(){
  stepperY.moveTo(0);                    // Move to the absolute position
  stepperY.runToPosition();                        // Execute the move
}


void setup()
{
  stepperY.setMaxSpeed(7000.0); // set max speed of the stepper , slower to get better accuracy
  stepperY.setAcceleration(9000.0); //set acceleration of the stepper 
  pinMode(EN, OUTPUT); 
  digitalWrite(EN, LOW); // Enable motor driver
  stepperY.setCurrentPosition(0);  // initialize the current position im at to be 0 

  pinMode(11, INPUT); // sensor far from the  sideway motor, responsible for the pos unit move control 
  pinMode(12, INPUT); // sensor close to the sideway motor, responsible for the pos unit move control  

  //test for side motion
  //moveSide(-350);
  //returnToInitialPositionSide();
  // test for rotary motion
  rotateByAngle(-12);
  rotaryInitialPositionRotation();

 

}




void loop() {
  
}


