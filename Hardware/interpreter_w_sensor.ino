#include "AccelStepper.h"
#include <SoftwareSerial.h>
#define EN 8
#define Y_DIR 6
#define Y_STP 3
#define Z_DIR 7
#define Z_STP 4
#define A_STP 13
#define A_DIR 12
#define X_DIR 5
#define X_STP 2


// Sensor Pins
#define sY_front 10 // Front endstop for Y-axis
#define sY_back 11  // Back endstop for Y-axis
#define sZ_front 9 // Front endstop for Z-axis
#define sZ_back A0  // Back endstop for Z-axis

// 1000 units is for the full range side to side
const float stepsPerMM = 400.0; // Adjust based on your setup ==> nb of steps required to move 1mm


// AccelStepper set up this is for the side to side of rod 1 (goalie)
AccelStepper stepperY(1,Y_STP, Y_DIR);
// other side to side road
AccelStepper stepperZ(1,Z_STP, Z_DIR); 
// rotation motor for stepperY
AccelStepper stepperX(1,X_STP, X_DIR);
// rotation motor for stepperX
AccelStepper stepperA(1,A_STP, A_DIR);


const float stepsPerRevolution =2000.0; 
const float degreesPerStep = 360.0 / stepsPerRevolution; 
int acceleration = 5;   

//BEGIN value
//go to the side till it hits the sensor 
void setBeginning(){
  // max is 1000 units
  int full_distance = 1000;
  moveSide(stepperX,sY_front, sY_back,full_distance); // Move in the positive direction, going backwards
  moveSide(stepperZ,sZ_front, sZ_back, full_distance);
  // for both stops when the sensor is triggered 
  int middlePosition = -full_distance/2 ;
  
  rotateByAngle(stepperX,0 ); 
  rotateByAngle(stepperA,0 );  // TODO: check if it works  ==> should be bc encoder understands current angle that it is on 
  moveSide(stepperY,sY_front, sY_back,middlePosition);
  stepperY.setCurrentPosition(middlePosition); // now this is 0 position

  moveSide(stepperZ,sZ_front, sZ_back,middlePosition);
  stepperX.setCurrentPosition(middlePosition); // now this is 0 position
  
}
// MOVE <value>
// <0 values to go far from the side motor
// >0 values to come close to the side motor 
// for 7cm mov range of player, 350 unit is good
// original 22 cm 
void moveSide(AccelStepper &stepper,int sensor1, int sensor2,  int value){
  int y =  stepper.currentPosition(); 
  stepper.move(value);          // Set the final position 
  //for coordinate value thats been going far from the side motor >0, controlled by the pin 11
  if(value>0){
    while (stepper.distanceToGo() != 0 &&  digitalRead(sensor1) == LOW ) {
      stepper.run();                  // Continuously move toward the target
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

void moveSide2(AccelStepper &stepper, int value){
  int y =  stepper.currentPosition(); 
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

//INITIALX
void returnToInitialPositionSide() {
  stepperY.moveTo(0);            // Command to move to position 0
  stepperZ.moveTo(0);
  stepperY.runToPosition();      // Execute the movement
  stepperZ.runToPosition();
  Serial.print("Returned to initial X position ");
}

//ROTATE <angle> 
// <0 value moves forward 
// >0 moves bavkwards
// angle = +-12 == 180 deg rotation
void rotateByAngle(AccelStepper &stepper, int angle) {
  moveSide2(stepper,angle);
}
//

void executeInterpreter(String command) {
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
  } else {
    Serial.println("Invalid Command!");
  }
}
// full set up if i was in one corner 1000 unit mo

void setup()
{
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

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // Read command from serial
    command.trim();                                // Remove whitespace
    executeInterpreter(command);                      // Execute parsed command
  }
  
}
