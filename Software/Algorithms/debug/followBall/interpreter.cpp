#include "interpreter.h"

#define EN 8
#define Y_DIR 6
#define Y_STP 3
#define X_DIR 7
#define X_STP 4

// 1000 units is for the full range side to side

Interpreter::Interpreter(
        float           spw,
        SoftwareSerial* ws1,
        SoftwareSerial* ws2,
        AccelStepper*   sx,
        AccelStepper*   sy,
        float           spr,
        float           dps,
        int             a
        )
  : stepsPerWM(spw)
  , wemosSerial1(ws1)
  , wemosSerial2(ws2)
  , stepperX(sx)
  , stepperY(sy)
  , stepsPerRevolution(spr)
  , degreesPerStep(dps)
  , acceleration(a)
{
  wemosSerial1->begin(9600);
  wemosSerial2->begin(9600);

  // Slower to get better accuracy
  stepperY->setMaxSpeed(5000.0);
  stepperX->setMaxSpeed(5000.0);

  stepperX->setAcceleration(5000.0);
  stepperY->setAcceleration(5000.0);
}

void Interpreter::setBeginning(){
  int full_distance = 1000;

  // Move in the positive direction
  moveSide(*stepperY,11,12,full_distance);
  moveSide(*stepperX,2,13, full_distance);
  rotateByAngle(*wemosSerial1,0);
  rotateByAngle(*wemosSerial2,0);

  // If sensor on pin 12 was triggered, go back to the middle
  if (digitalRead(12) == LOW) {
    // Calculate the middle position
    int middlePosition = full_distance/2 ;

    // Backtrack to the middle
    // Move back by 500 units (middle of the range)
    moveSide(*stepperY,11,12,-500);
    // Update the current position to middle
    // now this is 0 position
    stepperY->setCurrentPosition(middlePosition);
    Serial.println("Sensor triggered! Position set to the middle.");
  }

  if (digitalRead(13) == LOW) {
    // Calculate the middle position
    int middlePosition = full_distance/2 ;

    // Backtrack to the middle
    // Move back by 500 units (middle of the range)
    moveSide(*stepperY,2,13,-500);
    // Update the current position to middle
    // now this is 0 position
    stepperX->setCurrentPosition(middlePosition);
    Serial.println("Sensor triggered! Position set to the middle.");
  }

  // Reset the logical position to 0 at the middle
  stepperY->setCurrentPosition(0);
  Serial.println("Middle position set as 0 point.");
}

//BEGIN value
//go to the side till it hits the sensor
// MOVE <value>
// <0 values to go far from the side motor
// >0 values to come close to the side motor
// for 7cm mov range of player, 350 unit is good
// original 17 cm movement range
void Interpreter::moveSide(AccelStepper &stepper,int sensor1, int sensor2,  int value){
  int y =  stepper.currentPosition();
  stepper.move(value);          // Set the final position
  //for coordinate value thats been going far from the side motor >0, controlled by the pin 11
  if(value>0){
    while (stepper.distanceToGo() != 0 ) {
      stepper.run();                  // Continuously move toward the target
    }
    stepper.stop();
  }
  else {
    while (stepper.distanceToGo() != 0 ) {
      stepper.run();                  // Continuously move toward the target
    }
    stepper.stop();
  }
  Serial.print("Moved Y by ");
  Serial.println(value);
}

//INITIALX
void Interpreter::returnToInitialPositionSide() {
  stepperY->moveTo(0);            // Command to move to position 0
  stepperX->moveTo(0);
  stepperY->runToPosition();      // Execute the movement
  stepperX->runToPosition();
  Serial.print("Returned to initial X position ");
}

//ROTATE <angle>
// <0 value moves forward
// >0 moves bavkwards
// angle = +-12 == 180 deg rotation
void Interpreter::rotateByAngle(SoftwareSerial &serialPort, float angle) {
  float rad_angle = angle * 3.14/180;
  // MOVE <angle>
  String command = "MOVE " + String(rad_angle);
  serialPort.println(command);
  Serial.print("Rotated by ");
  Serial.println(angle);
}

void Interpreter::executeInterpreter(String command) {
  char cmd[20];
  int value;
  sscanf(command.c_str(), "%s %d", cmd, &value);

  if (strcmp(cmd, "MOVE1") == 0) {
    moveSide(*stepperY, 11,12, value);
  }else if(strcmp(cmd, "MOVE2")==0){
    moveSide(*stepperX,2, 0,value);
  } else if (strcmp(cmd, "ROTATE1") == 0) {
    rotateByAngle(*wemosSerial1,(float)value); // Rotate by angle single player pole
  }else if(strcmp(cmd, "ROTATE2") == 0){
    rotateByAngle(*wemosSerial2,(float)value); // Rotate by angle double player pole
  } else if (strcmp(cmd, "INITIALX") == 0) {
    returnToInitialPositionSide();
  } else if(strcmp(cmd, "INITIALY")== 0){
    rotateByAngle(*wemosSerial1, 0);
    rotateByAngle(*wemosSerial2, 0);
  }else if(strcmp(cmd, "BEGIN")==0){
    setBeginning();
  } else {
    Serial.println("Invalid Command!");
  }
}
