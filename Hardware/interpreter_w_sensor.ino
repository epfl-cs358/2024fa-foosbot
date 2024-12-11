#include "AccelStepper.h"
#include <SoftwareSerial.h>
#define EN 8
#define Y_DIR 6
#define Y_STP 3
#define Z_DIR 7
#define Z_STP 4
#define sY_front 9
#define sY_back 12
#define sX_front 2 //POS VALUE
#define sX_back 5 // NEG VALUE 

// 1000 units is for the full range side to side
const float stepsPerMM = 400.0; // Adjust based on your setup ==> nb of steps required to move 1mm
SoftwareSerial wemosSerial1(-1, 10);  // single player pole rotary
 // double player pole rotary
SoftwareSerial wemosSerial2(-1, 11);  // single player pole rotary

// AccelStepper set up this is for the side to side of rod 1 (goalie)
AccelStepper stepperY(1,Y_STP, Y_DIR);
// other side to side road
AccelStepper stepperX(1,Z_STP, Z_DIR);

long TravelY;

const float stepsPerRevolution = 3200.0; 
const float degreesPerStep = 360.0 / stepsPerRevolution; 
int acceleration = 5;   

//BEGIN value
//go to the side till it hits the sensor 
void setBeginning(){
  // max is 1000 units
  int full_distance = 1000;
  moveSide(stepperX,sY_front, sY_back,full_distance); // Move in the positive direction, going backwards
  moveSide(stepperY,sX_front, sX_back, full_distance);
  // for both stops when the sensor is triggered 
  int middlePosition = -full_distance/2 ;
  
  rotateByAngle(wemosSerial1,0 ); 
  rotateByAngle(wemosSerial2,0 );  // TODO: check if it works  ==> should be bc encoder understands current angle that it is on 
  moveSide(stepperY,sY_front, sY_back,middlePosition);
  stepperY.setCurrentPosition(middlePosition); // now this is 0 position

  moveSide(stepperX,sX_front, sX_back,middlePosition);
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

//INITIALX
void returnToInitialPositionSide() {
  stepperY.moveTo(0);            // Command to move to position 0
  stepperX.moveTo(0);
  stepperY.runToPosition();      // Execute the movement
  stepperX.runToPosition();
  Serial.print("Returned to initial X position ");
}

//ROTATE <angle> 
// <0 value moves forward 
// >0 moves bavkwards
// angle = +-12 == 180 deg rotation
void rotateByAngle(SoftwareSerial &serialPort, float angle) {
  float rad_angle = angle * 3.14/180;
  // MOVE <angle>
  String command = "MOVE " + String(rad_angle);
  serialPort.println(command);
  Serial.print("Rotated by ");
  Serial.println(angle);
}
//

void executeInterpreter(String command) {
  char cmd[20];
  int value;
  sscanf(command.c_str(), "%s %d", cmd, &value);

  if (strcmp(cmd, "MOVE1") == 0) {
    moveSide(stepperY, sY_front,sY_back, value);
  }else if(strcmp(cmd, "MOVE2")==0){
    moveSide(stepperX, sX_front, sX_back,value);
  } else if (strcmp(cmd, "ROTATE1") == 0) {
    rotateByAngle(wemosSerial1,(float)value); // Rotate by angle single player pole
  }else if(strcmp(cmd, "ROTATE2") == 0){
    rotateByAngle(wemosSerial2,(float)value); // Rotate by angle double player pole
  } else if (strcmp(cmd, "INITIALX") == 0) {
    returnToInitialPositionSide();
  } else if(strcmp(cmd, "INITIALY")== 0){
    rotateByAngle(wemosSerial1, 0);
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
  wemosSerial1.begin(9600);
  stepperY.setMaxSpeed(5000.0); // set max speed of the stepper , slower to get better accuracy
  stepperY.setAcceleration(5000.0); //set acceleration of the stepper 
  stepperX.setMaxSpeed(5000.0); // set max speed of the stepper , slower to get better accuracy
  stepperX.setAcceleration(5000.0); //set acceleration of the stepper 
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
