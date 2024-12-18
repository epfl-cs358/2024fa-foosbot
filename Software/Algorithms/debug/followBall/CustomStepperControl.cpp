#include "CustomStepperControl.h"

// Constructor
CustomStepperControl::CustomStepperControl(int yDir, int yStep, int zDir, int zStep, int aDir, int aStep, int xDir, int xStep, int enPin, int sY_front, int sY_back, int sZ_front, int sZ_back)
  : stepperY(1, yStep, yDir),
    stepperZ(1, zStep, zDir),
    stepperX(1, xStep, xDir),
    stepperA(1, aStep, aDir)
{
  Y_DIR = yDir;
  Y_STP = yStep;
  Z_DIR = zDir;
  Z_STP = zStep;
  A_DIR = aDir;
  A_STP = aStep;
  X_DIR = xDir;
  X_STP = xStep;
  EN = enPin;
  this->sY_front = sY_front;
  this->sY_back = sY_back;
  this->sZ_front = sZ_front;
  this->sZ_back = sZ_back;
}

void CustomStepperControl::setBeginning() {
  Serial.println("Begin");

  int max_distance = 1100; //max distance in motorstep for translation between the trigger -> 241 mm * 5 (for motor units)

  moveSide(stepperY, sY_front, sY_back, max_distance); // Move in the positive direction, going backwards
  stepperY.setCurrentPosition(0);                      // Set pos of stepper to 0
  moveSide(stepperY, sY_front, sY_back, -max_distance);// Move in the negative direction, going forward
  int middle = 1100/2;                                 //have the bar be centered on the middle player
  moveSide(stepperY, sY_front, sY_back, middle);

  moveSide(stepperZ, sZ_front, sZ_back, max_distance);
  stepperZ.setCurrentPosition(0);
  moveSide(stepperZ, sZ_front, sZ_back, -max_distance);
  moveSide(stepperZ, sZ_front, sZ_back, middle);
}

// MOVE <value>
// <0 values to go far from the side motor
// >0 values to come close to the side motor
// for 7cm mov range of player, 350 unit is good
// original 22 cm
void CustomStepperControl::moveSide(AccelStepper &stepper, int sensor1, int sensor2, double value) {
  //int y =  stepper.currentPosition();
  stepper.move(value);          // Set the final position
  //for coordinate value thats been going far from the side motor >0, controlled by the pin 11
  if(value>0){
    while (stepper.distanceToGo() != 0 &&  digitalRead(sensor1) == LOW ) {
      stepper.run();                 // Continuously move toward the target
    }
    stepper.stop();
  }
  else {
    while (stepper.distanceToGo() != 0 && digitalRead(sensor2) == LOW  ) {
      stepper.run();                  // Continuously move toward the target
    }
    stepper.stop();
  }
  Serial.print("Moved by ");
  Serial.println(value);
}

void CustomStepperControl::moveSide2(AccelStepper &stepper, double value){
  //int y =  stepper.currentPosition();
  stepper.move(value);          // Set the final position
  //for coordinate value thats been going far from the side motor >0, controlled by the pin 11
  if(value>0){
    while (stepper.distanceToGo() != 0  ) {
      stepper.run();                  // Continuously move toward the target
    }
    stepper.stop();
  }
  else {
    while (stepper.distanceToGo() != 0  ) {
      stepper.run();                  // Continuously move toward the target
    }
    stepper.stop();
  }
  Serial.print("Moved by ");
  Serial.println(value);
}


void CustomStepperControl::returnToInitialPositionSide() {
  int middle = -1100/2;
  stepperY.moveTo(middle);            // Command to move to position 0
  stepperZ.moveTo(middle);
  stepperY.runToPosition();      // Execute the movement
  stepperZ.runToPosition();
  Serial.print("Returned to initial X position ");
}

void CustomStepperControl::rotateByAngle(AccelStepper &stepper, double angle) {
  moveSide2(stepper,angle);
}

void CustomStepperControl::executeInterpreter(String command) {
  char cmd[20];
  int value;
  sscanf(command.c_str(), "%s %d", cmd, &value);

  if (strcmp(cmd, "MOVE1") == 0) {
    moveSide(stepperY, sY_front,sY_back, value);
  }else if(strcmp(cmd, "MOVE2")==0){
    moveSide(stepperZ, sZ_front, sZ_back,value);
  } else if (strcmp(cmd, "ROTATE1") == 0) {
    moveSide2(stepperX,value); // Rotate by angle single player pole
  }else if(strcmp(cmd, "ROTATE2") == 0){
    moveSide2(stepperA,value); // Rotate by angle double player pole
   }else if(strcmp(cmd, "BEGIN")==0){
    setBeginning();
  }else if(strcmp(cmd, "INITX")==0){
    returnToInitialPositionSide();
  } else {
    Serial.println("Invalid Command!");
  }
}

void CustomStepperControl::setupSteppers() {
  Serial.begin(9600);
  stepperY.setMaxSpeed(5000.0); // set max speed of the stepper , slower to get better accuracy
  stepperY.setAcceleration(5000.0); //set acceleration of the stepper
  stepperX.setMaxSpeed(5000.0); // set max speed of the stepper , slower to get better accuracy
  stepperX.setAcceleration(5000.0); //set acceleration of the stepper
  stepperZ.setMaxSpeed(5000.0); // set max speed of the stepper , slower to get better accuracy
  stepperZ.setAcceleration(5000.0); //set acceleration of the stepper
  stepperA.setMaxSpeed(5000.0); // set max speed of the stepper , slower to get better accuracy
  stepperA.setAcceleration(5000.0); //set acceleration of the stepper
  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW); // Enable motor driver
  //stepperY.setCurrentPosition(0);  // initialize the current position im at to be 0
}
