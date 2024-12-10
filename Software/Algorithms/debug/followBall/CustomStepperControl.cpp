#include "CustomStepperControl.h"

// Constructor
CustomStepperControl::CustomStepperControl(int yDir, int yStep, int xDir, int xStep, int enPin, int sensor1Y, int sensor2Y, int sensor1X, int sensor2X) 
  : stepperY(1, yStep, yDir), 
    stepperX(1, xStep, xDir), 
    wemosSerial1(-1, 2), 
    wemosSerial2(-1, 2) 
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
  int full_distance = 1000;
  moveSide(stepperY, sensor1Y, sensor2Y, full_distance);
  moveSide(stepperX, sensor1X, sensor2X, full_distance);
  rotateByAngle(wemosSerial1, 0); 
  rotateByAngle(wemosSerial2, 0); 

  if (digitalRead(sensor2Y) == LOW) {
    int middlePosition = full_distance / 2;
    moveSide(stepperY, sensor1Y, sensor2Y, -500);
    stepperY.setCurrentPosition(middlePosition);
  }

  if (digitalRead(sensor2X) == LOW) {
    int middlePosition = full_distance / 2;
    moveSide(stepperX, sensor1X, sensor2X, -500);
    stepperX.setCurrentPosition(middlePosition);
  }

  stepperY.setCurrentPosition(0);
}

void CustomStepperControl::moveSide(AccelStepper &stepper, int sensor1, int sensor2, int value) {
  stepper.move(value);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  stepper.stop();
}

void CustomStepperControl::returnToInitialPositionSide() {
  stepperY.moveTo(0);
  stepperX.moveTo(0);
  stepperY.runToPosition();
  stepperX.runToPosition();
}

void CustomStepperControl::rotateByAngle(SoftwareSerial &serialPort, float angle) {
  float rad_angle = angle * 3.14 / 180;
  String command = "MOVE " + String(rad_angle);
  serialPort.println(command);
}

void CustomStepperControl::executeInterpreter(String command) {
  char cmd[20];
  int value;
  sscanf(command.c_str(), "%s %d", cmd, &value);

  if (strcmp(cmd, "MOVE1") == 0) {
    moveSide(stepperY, sensor1Y, sensor2Y, value);
  } else if (strcmp(cmd, "MOVE2") == 0) {
    moveSide(stepperX, sensor1X, sensor2X, value);
  } else if (strcmp(cmd, "ROTATE1") == 0) {
    rotateByAngle(wemosSerial1, (float)value);
  } else if (strcmp(cmd, "ROTATE2") == 0) {
    rotateByAngle(wemosSerial2, (float)value);
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
  wemosSerial1.begin(9600);
  wemosSerial2.begin(9600);

  stepperY.setMaxSpeed(5000.0);
  stepperY.setAcceleration(5000.0);
  stepperX.setMaxSpeed(5000.0);
  stepperX.setAcceleration(5000.0);

  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);

  pinMode(sensor1Y, INPUT);
  pinMode(sensor2Y, INPUT);
  pinMode(sensor1X, INPUT);
  pinMode(sensor2X, INPUT);
}
