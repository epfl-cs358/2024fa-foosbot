#include "ParallelInterpreter.h"

ParallelInterpreter::ParallelInterpreter(int yDir, int yStep,
                                         int zDir, int zStep,
                                         int aDir, int aStep,
                                         int xDir, int xStep,
                                         int enPin,
                                         int sY_front, int sY_back,
                                         int sZ_front, int sZ_back,
                                         int FIELD_SIZE)
    : stepperY(1, yStep, yDir), stepperZ(1, zStep, zDir),
      stepperX(1, xStep, xDir), stepperA(1, aStep, aDir)
{

  Y_DIR = yDir ;
  Y_STP = yStep;
  Z_DIR = zDir ;
  Z_STP = zStep;
  A_DIR = aDir ;
  A_STP = aStep;
  X_DIR = xDir ;
  X_STP = xStep;
  EN    = enPin;

  this->sY_front = sY_front;
  this->sY_back  = sY_back;
  this->sZ_front = sZ_front;
  this->sZ_back  = sZ_back;

  FIELD_SIZE = FIELD_SIZE;
}

int ParallelInterpreter::intLength(int value) {

  // Special case for 0
  if (value == 0) return 1;

  // Start with 2 if negative (1 for '-' and 1 for digits), or 1 for positive
  int length = (value < 0) ? 2 : 1;

  // Count the number of digits
  length += floor(log10(abs(value)));

  return length;
}

void ParallelInterpreter::setTarget(const char* cmd, int value){
  if (strcmp(cmd, "MOVE1") == 0) {

    targetY += value;
    targetY  =
      targetY > maxY ?
        maxY :
      targetY < minY ?
        minY :
      targetY;

    stepperY.moveTo(targetY);

  } else if (strcmp(cmd, "MOVE2") == 0) {

    targetZ += value;
    targetZ  =
      targetZ > maxZ ?
        maxZ :
      targetZ < minZ ?
        minZ :
      targetZ;

    stepperZ.moveTo(targetZ);

  } else if (strcmp(cmd, "ROTATE1") == 0) {

    targetX += value;
    stepperX.moveTo(targetX);

  } else if (strcmp(cmd, "ROTATE2") == 0) {

    targetA += value;
    stepperA.moveTo(targetA);

  } else if (strcmp(cmd, "BEGIN") == 0) {
    rangeEstimation();

  } else {
    Serial.print("Invalid Command: ");
    Serial.println(cmd);
  }
}

void ParallelInterpreter::rangeEstimation(){
  Serial.println("Starting range estimation...");


  // Estimate Y-axis range (MOVE1) //

  Serial.println("Estimating Y-axis range...");

  // Move backward until sY_back is triggered
  stepperY.move(-FIELD_SIZE - 1000);
  while (digitalRead(sY_back) == LOW) {
    stepperY.run();
  }

  // Reset the position to make this sensor as the origin
  stepperY.setCurrentPosition(0);
  minY = stepperY.currentPosition();
  Serial.print("minY: ");
  Serial.println(minY);

  // Move forward until sY_front is triggered
  stepperY.move(FIELD_SIZE - 100); // Go fast close to the second sensor
  while (digitalRead(sY_front) == LOW) {
    stepperY.run();
  }
  // Then go really slowly to the sensor to define accurately how many steps
  // are between the two sensors
  while (digitalRead(sY_front) == LOW) {
    stepperY.move(1);
    stepperY.run();
  }
  maxY = stepperY.currentPosition(); // we save the position of the stepper and
                                     // it is now our limit in our movement
  Serial.print("maxY: ");
  Serial.println(maxY);

  //Center the stepper after estimation
  stepperY.moveTo((maxY + minY) / 2);
  targetY = (maxY + minY) / 2;
  stepperY.runToPosition();


  // Estimate Z-axis range (MOVE2) //

  Serial.println("Estimating Z-axis range...");

  // Move forward until sZ_front is triggered
  stepperZ.move(-FIELD_SIZE - 2000);
  while (digitalRead(sZ_back) == LOW) {
    stepperZ.run();
  }

  //Reset the position to make this sensor as the origin
  stepperZ.setCurrentPosition(0);
  minZ = stepperZ.currentPosition();
  Serial.print("minZ: ");
  Serial.println(minZ);

  // Move backward until sZ_back is triggered
  stepperZ.move(FIELD_SIZE - 100); // Go fast close to the second sensor
  while (digitalRead(sZ_front) == LOW) {
    stepperZ.run();
  }
  // Then go really slowly to the sensor to define accurately how many
  // steps are between the two sensors
  while (digitalRead(sZ_front) == LOW) {
    stepperZ.move(1);
    stepperZ.run();
  }
  maxZ = stepperZ.currentPosition(); // we save the position of the stepper and
                                     // it is now our limit in our movement
  Serial.print("maxZ: ");
  Serial.println(maxZ);

  // Center the stepper after estimation
  stepperZ.moveTo((maxZ + minZ) / 2);
  targetZ = (maxZ + minZ) / 2;
  stepperZ.runToPosition();

  Serial.println("Range estimation completed.");
}

void ParallelInterpreter::executeInterpreter(String command){
  command.trim();

  char indiv_Cmd[20];
  int value;
  int offset = 0;

  // Parse multiple commands
  while (offset < command.length()) {
    // Try to parse a command and value pair
    int parsed = sscanf(command.c_str() + offset, "%s %d", indiv_Cmd, &value);

    if (parsed == 2) {
      // Valid command and value
      setTarget(indiv_Cmd, value);

      // Update offset to skip the current command and value
      offset += strlen(indiv_Cmd) + 2 + intLength(value);
    }
  }
}

void ParallelInterpreter::moveMotorsWithSensors(){

  // MOVE1 (stepperY)
  if (stepperY.distanceToGo() != 0) {
    if (
      // Moving forward, not tripping sensor
      (targetY > stepperY.currentPosition() && digitalRead(sY_front) == LOW) ||
      // Moving backward, not tripping sensor
      (targetY < stepperY.currentPosition() && digitalRead(sY_back) == LOW)) {

      // Continue running normally
      stepperY.run();

    } else {

      stepperY.stop();  // Safety stop if sensor triggered
      Serial.println("MOVE1 stopped due to sensor.");

      // Automatically back off slightly to allow future movement
      if (digitalRead(sY_front) == HIGH) {  // Front sensor triggered

        stepperY.move(-10);                 // Back off slightly
        stepperY.runToPosition();           // Ensure the back-off completes

        Serial.println("MOVE1 backed off from front sensor.");

      } else if (digitalRead(sY_back) == HIGH) {  // Back sensor triggered

        stepperY.move(10);                        // Move forward slightly
        stepperY.runToPosition();                 // Ensure the back-off completes

        Serial.println("MOVE1 backed off from back sensor.");
      }
    }
  }

  // MOVE2 (stepperZ)
  if (stepperZ.distanceToGo() != 0) {
    if (
      (targetZ > stepperZ.currentPosition() && digitalRead(sZ_front) == LOW) ||
      (targetZ < stepperZ.currentPosition() && digitalRead(sZ_back) == LOW)) {

      stepperZ.run();  // Continue running normally

    } else {

      stepperZ.stop();  // Safety stop if sensor triggered
      Serial.println("MOVE2 stopped due to sensor.");

      // Automatically back off slightly to allow future movement
      if (digitalRead(sZ_front) == HIGH) {  // Front sensor triggered

        stepperZ.move(-10);                 // Back off slightly
        stepperZ.runToPosition();           // Ensure the back-off completes

        Serial.println("MOVE2 backed off from front sensor.");

      } else if (digitalRead(sZ_back) == HIGH) {  // Back sensor triggered

        stepperZ.move(10);                        // Move forward slightly
        stepperZ.runToPosition();                 // Ensure the back-off completes

        Serial.println("MOVE2 backed off from back sensor.");
      }
    }
  }

  // ROTATE1 (stepperX) - No sensors here
  if (stepperX.distanceToGo() != 0) {
    stepperX.run();  // Continue running normally
  }

  // ROTATE2 (stepperA) - No sensors here
  if (stepperA.distanceToGo() != 0) {
    stepperA.run();  // Continue running normally
  }
}

void ParallelInterpreter::setupSteppers(){
  Serial.begin(9600);

  // Initialize steppers
  stepperY.setMaxSpeed    (3000.0);
  stepperY.setAcceleration(3000.0);
  stepperZ.setMaxSpeed    (3000.0);
  stepperZ.setAcceleration(3000.0);
  stepperX.setMaxSpeed    (5000.0);
  stepperX.setAcceleration(5000.0);
  stepperA.setMaxSpeed    (5000.0);
  stepperA.setAcceleration(5000.0);

  // Initialize pins
  pinMode(sY_front,  INPUT);
  pinMode(sY_back ,  INPUT);
  pinMode(sZ_front,  INPUT);
  pinMode(sZ_back ,  INPUT);
  pinMode(EN      , OUTPUT);

  digitalWrite(EN, LOW);  // Enable motor driver
}
