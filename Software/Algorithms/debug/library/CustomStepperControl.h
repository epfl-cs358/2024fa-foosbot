#ifndef CUSTOM_STEPPER_CONTROL_H
#define CUSTOM_STEPPER_CONTROL_H

#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Arduino.h>

class CustomStepperControl {
  public:
    CustomStepperControl(int yDir, int yStep, int zDir, int zStep, int aDir, int aStep, int xDir, int xStep, int enPin, int sY_front, int sY_back, int sZ_front, int sZ_back);

    void setBeginning();
    void moveSide(AccelStepper &stepper, int sensor1, int sensor2, double value);
    void moveSide2(AccelStepper &stepper, double value);
    void returnToInitialPositionSide();
    void rotateByAngle(AccelStepper &stepper, double angle);
    void executeInterpreter(String command);
    void setupSteppers();

  private:
    AccelStepper stepperY;
    AccelStepper stepperZ;
    AccelStepper stepperX;
    AccelStepper stepperA;
    int EN;
    int Y_DIR, Y_STP, Z_DIR, Z_STP, A_DIR, A_STP, X_DIR, X_STP;
    int sY_front, sY_back, sZ_front, sZ_back;
    const float stepsPerMM = 400.0;
    const float stepsPerRevolution = 3200.0;
    const float degreesPerStep = 360.0 / stepsPerRevolution;
    int acceleration = 5;
};

#endif // CUSTOM_STEPPER_CONTROL_H
