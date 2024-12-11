#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <AccelStepper.h>
#include <SoftwareSerial.h>

class Interpreter {
  public:
      Interpreter(
              float spw,
              SoftwareSerial ws1,
              SoftwareSerial ws2,
              AccelStepper   sx,
              AccelStepper   sy,
              float          spr,
              float          dps,
              int            a
    );
    void setBeginning();
    void moveSide(
            AccelStepper &stepper,
            int sensor1,
            int sensor2,
            int value
    );
    void returnToInitialPositionSide();
    void rotateByAngle(SoftwareSerial &serialPort, float angle);
    void executeInterpreter(String command);

  private:
    float stepsPerWM;

    SoftwareSerial* wemosSerial1;  // single player pole rotary
    SoftwareSerial* wemosSerial2;  // double player pole rotary

    AccelStepper* stepperY;
    AccelStepper* stepperX;

    float stepsPerRevolution;
    float degreesPerStep;
    int   acceleration;
};

#endif /* ifndef INTERPRETER_H */
