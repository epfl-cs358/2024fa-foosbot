#ifndef PARALLEL_INTERPRETER_H
#define PARALLEL_INTERPRETER_H

#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Arduino.h>

class ParallelIntepreter {
  public:
    ParallelIntepreter(int yDir, int yStep, int zDir, int zStep, int aDir, int aStep, int xDir, int xStep, int enPin, int sY_front, int sY_back, int sZ_front, int sZ_back);
    
    int  intLength(int value);
    void setTarget(const char* cmd, int value);
    void rangeEstimation();
    void executeInterpreter(String command);
    void moveMotorsWithSensors();
    void setupSteppers();


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
    int fieldSize;
};

#endif //PARALLEL_INTERPRETER_H
