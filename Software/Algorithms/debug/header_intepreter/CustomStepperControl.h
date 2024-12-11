#ifndef CUSTOM_STEPPER_CONTROL_H
#define CUSTOM_STEPPER_CONTROL_H

#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Arduino.h>

class CustomStepperControl {
  public:
    CustomStepperControl(int yDir, int yStep, int xDir, int xStep, int enPin, int sensor1Y, int sensor2Y, int sensor1X, int sensor2X);
    
    void setBeginning();
    void moveSide(AccelStepper &stepper, int sensor1, int sensor2, int value);
    void returnToInitialPositionSide();
    void rotateByAngle(SoftwareSerial &serialPort, float angle);
    void executeInterpreter(String command);
    void setupSteppers();
  
  private:
    AccelStepper stepperY;
    AccelStepper stepperX;
    SoftwareSerial wemosSerial1;
    SoftwareSerial wemosSerial2;
    int EN;
    int Y_DIR, Y_STP, X_DIR, X_STP;
    int sensor1Y, sensor2Y, sensor1X, sensor2X;
    const float stepsPerMM = 400.0;
    const float stepsPerRevolution = 3200.0;
    const float degreesPerStep = 360.0 / stepsPerRevolution;
    int acceleration = 5;
};

#endif // CUSTOM_STEPPER_CONTROL_H
