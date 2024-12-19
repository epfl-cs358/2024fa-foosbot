#include "CustomStepperControl.h"

// Constructor
CustomStepperControl::CustomStepperControl(int yDir, int yStep, int xDir, int xStep, int enPin, int sensor1Y, int sensor2Y, int sensor1X, int sensor2X)
  : stepperY(1, yStep, yDir),
    stepperX(1, xStep, xDir)
{
  Y_DIR = yDir;
  Y_STP = yStep;
  X_DIR = xDir;
  X_STP = xStep;
  EN = enPin;
  this->sensor1Y = sensor1Y;
  this->sensor2Y = sensor2Y;
  this->sensor1X = sensor1X;
  this->sensor2X = sensor2X;
}

void CustomStepperControl::setBeginning() {
  Serial.println("setBeginning() called.");
  int big_distance = 2000; //max distance in motorstep for translation
  moveSide(stepperY, sensor1Y, sensor2Y, big_distance);
  moveSide(stepperX, sensor1X, sensor2X, big_distance);
  stepperY.setCurrentPosition(0);
  stepperX.setCurrentPosition(0);
  Serial.println(stepperY.currentPosition());
  Serial.println(stepperX.currentPosition());
  moveSide(stepperY, sensor1Y, sensor2Y, -big_distance);
  moveSide(stepperX, sensor1X, sensor2X, -big_distance);
  int max_translation = stepperY.currentPosition();
  moveSide(stepperY, sensor1Y, sensor2Y, -max_translation/2 * 5);
  moveSide(stepperX, sensor1X, sensor2X, -max_translation/2 * 5);
  // Move into -z direction


  //   // for both stops when the sensor is triggered
  // int middlePosition = -(40/2 * 5)/2 ;  //translation_width/2 * fieldXToMotorUnits

  // rotateByAngle(wemosSerial1, 0);
  // rotateByAngle(wemosSerial2, 0);

  // moveSide(stepperY,sensor1Y, sensor2Y, middlePosition);
  // stepperY.setCurrentPosition(middlePosition); // now this is 0 position

  // moveSide(stepperX,sensor1X, sensor2X,middlePosition);
  // stepperX.setCurrentPosition(middlePosition); // now this is 0 position
}

// MOVE <value>
// <0 values to go far from the side motor
// >0 values to come close to the side motor
// for 7cm mov range of player, 350 unit is good
// original 22 cm
void CustomStepperControl::moveSide(AccelStepper &stepper, int sensor1, int sensor2, int value) {
  //int y =  stepper.currentPosition();
  stepper.move(value);          // Set the final position
  //for coordinate value thats been going far from the side motor >0, controlled by the pin 11
  if(value>0){
    while (digitalRead(sensor1) == LOW && digitalRead(sensor2) == LOW && stepper.distanceToGo() != 0) {
      Serial.println(digitalRead(sensor1));
      Serial.println(digitalRead(sensor2));
      stepper.run();                  // Continuously move toward the target
    }
    stepper.stop();
    stepper.runSpeedToPosition();
  }
  else {
    while (digitalRead(sensor1) == LOW && digitalRead(sensor2) == LOW && stepper.distanceToGo() != 0) {
      Serial.println(digitalRead(sensor1));
      Serial.println(digitalRead(sensor2));
      stepper.run();                  // Continuously move toward the target
    }
    stepper.stop();
    stepper.runSpeedToPosition();
  }
  Serial.print("Moved by ");
  Serial.println(value);
}


void CustomStepperControl::returnToInitialPositionSide() {
  stepperY.moveTo(0);
  stepperX.moveTo(0);
  stepperY.runToPosition();
  stepperX.runToPosition();
}

// void CustomStepperControl::rotateByAngle(SoftwareSerial &serialPort, float angle) {
//   float rad_angle = angle * 3.14 / 180;
//   String command = "MOVE " + String(rad_angle);
//   serialPort.println(command);
// }

void CustomStepperControl::executeInterpreter(String command) {
  char cmd[20];
  int value;
  sscanf(command.c_str(), "%s %d", cmd, &value);

  if (strcmp(cmd, "MOVE1") == 0) {
    moveSide(stepperY, sensor1Y, sensor2Y, value);
  } else if (strcmp(cmd, "MOVE2") == 0) {
    moveSide(stepperX, sensor1X, sensor2X, value);
  // } else if (strcmp(cmd, "ROTATE1") == 0) {
  //   rotateByAngle(wemosSerial1, (float)value);
  // } else if (strcmp(cmd, "ROTATE2") == 0) {
  //   rotateByAngle(wemosSerial2, (float)value);
  } else if (strcmp(cmd, "INITIALX") == 0) {
    returnToInitialPositionSide();
  } else if (strcmp(cmd, "BEGIN") == 0) {
    setBeginning();
  } else {
    Serial.println("Invalid Command!");
  }
}

void CustomStepperControl::setupSteppers() {
  Serial.begin(9600);

  stepperY.setMaxSpeed(500.0);
  stepperY.setAcceleration(4000.0);
  stepperX.setMaxSpeed(500.0);
  stepperX.setAcceleration(4000.0);

  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);

  // pinMode(sensor1Y, INPUT);
  // pinMode(sensor2Y, INPUT);
  // pinMode(sensor1X, INPUT);
  // pinMode(sensor2X, INPUT);
}
