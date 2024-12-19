#ifndef PARALLEL_INTERPRETER_H
#define PARALLEL_INTERPRETER_H

#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Arduino.h>


class ParallelInterpreter {
  public:
    ParallelInterpreter(int yDir, int yStep,
                       int zDir, int zStep,
                       int aDir, int aStep,
                       int xDir, int xStep,
                       int enPin,
                       int sY_front, int sY_back,
                       int sZ_front, int sZ_back,
                       int fieldSize);

    /*
     * Sets up the stepper motors.
     */
    void setupSteppers();

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

  private:
    AccelStepper stepperY;
    AccelStepper stepperZ;
    AccelStepper stepperX;
    AccelStepper stepperA;

    int targetY, targetZ, targetX, targetA;
    int minY, maxY, minZ, maxZ;

    int EN;

    int Y_DIR, Y_STP, Z_DIR, Z_STP, A_DIR, A_STP, X_DIR, X_STP;
    int sY_front, sY_back, sZ_front, sZ_back;

    int FIELD_SIZE;

    /*
     * Computes the string length of an integer.
     *
     * @param value The integer which length to be computed
     *
     * @returns the length of `value`.
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
     * Estimates the ranges of the linear motions by moving motors to the
     * end-stop sensors.
     */
    void rangeEstimation();

};

#endif // PARALLEL_INTERPRETER_H
