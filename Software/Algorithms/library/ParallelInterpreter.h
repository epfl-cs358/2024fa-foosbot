#ifndef PARALLEL_INTERPRETER_H
#define PARALLEL_INTERPRETER_H

#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Arduino.h>


class ParallelIntepreter {
  public:
    ParallelIntepreter(int yDir, int yStep,
                       int zDir, int zStep,
                       int aDir, int aStep,
                       int xDir, int xStep,
                       int enPin,
                       int sY_front, int sY_back,
                       int sZ_front, int sZ_back,
                       int fieldSize);

    /*
     * TODO: Doc
     *
     * @param value 
     *
     * @returns 
     */
    int  intLength(int value);

    /*
     * Set stepper's target if every safety check has been verified.
     *
     * @param cmd   Command to be executed
     * @param value Value of movement
     */
    void setTarget(const char* cmd, int value);

    /*
     * TODO: Doc
     */
    void rangeEstimation();

    /*
     * Parses the given input and executes the resulting command.
     *
     * @param command Command to be executed
     */
    void executeInterpreter(String command);

    /*
     * Controls motors without blocking and checks for sensor input for
     * safety check.
     */
    void moveMotorsWithSensors();

    /*
     * TODO: Doc
     */
    void setupSteppers();

  private:
    AccelStepper stepperY;
    AccelStepper stepperZ;
    AccelStepper stepperX;
    AccelStepper stepperA;
    int EN;
    int Y_DIR, Y_STP, Z_DIR, Z_STP, A_DIR, A_STP, X_DIR, X_STP;
    int sY_front, sY_back, sZ_front, sZ_back;
    int FIELD_SIZE;
};

#endif //PARALLEL_INTERPRETER_H
