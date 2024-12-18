#include "AccelStepper.h"
#include <SoftwareSerial.h>

// Pin Definitions
#define EN 8
#define Y_DIR 6
#define Y_STP 3
#define Z_DIR 7
#define Z_STP 4
#define A_STP 12
#define A_DIR 13
#define X_STP 2
#define X_DIR 5

// Sensor Pins
#define sY_front 10
#define sY_back 11
#define sZ_front A3
#define sZ_back A0
#define fieldSize 1300
// Stepper Motor Initialization
AccelStepper stepperY(1, Y_STP, Y_DIR);  // MOVE1
AccelStepper stepperZ(1, Z_STP, Z_DIR);  // MOVE2
AccelStepper stepperX(1, X_STP, X_DIR);  // ROTATE1
AccelStepper stepperA(1, A_STP, A_DIR);  // ROTATE2

// Sensor-controlled Stepper Movement Targets
int targetY = 0, targetZ = 0, targetX = 0, targetA = 0;
int maxY = 0, minY = 0, maxZ = 0, minZ = 0;

int intLength(int value) {
  if (value == 0) return 1;            // Special case for 0
  int length = (value < 0) ? 2 : 1;    // Start with 2 if negative (1 for '-' and 1 for digits), or 1 for positive
  length += floor(log10(abs(value)));  // Count the number of digits
  return length;
}


// Function to set stepper target positions with safety checks
void setTarget(const char* cmd, int value) {
  if (strcmp(cmd, "MOVE1") == 0) {
    Serial.println("1");
    targetY += value;
    targetY = targetY > maxY ? maxY : targetY < minY ? minY
                                                     : targetY;
    stepperY.moveTo(targetY);
  } else if (strcmp(cmd, "MOVE2") == 0) {
    Serial.println("2");
    targetZ += value;
    targetZ = targetZ > maxZ ? maxZ : targetZ < minZ ? minZ
                                                     : targetZ;
    stepperZ.moveTo(targetZ);
  } else if (strcmp(cmd, "ROTATE1") == 0) {
    Serial.println("3");
    targetX += value;
    stepperX.moveTo(targetX);
  } else if (strcmp(cmd, "ROTATE2") == 0) {
    Serial.println("4");
    targetA += value;
    stepperA.moveTo(targetA);
  } else if (strcmp(cmd, "BEGIN") == 0) {
    Serial.println("5");
    rangeEstimation();
  } else {
    Serial.println("Invalid Command!");
    Serial.println(cmd);
  }
}

void rangeEstimation() {
  Serial.println("Starting range estimation...");

  // Estimate Y-axis range (MOVE1)
  Serial.println("Estimating Y-axis range...");
  // Move backward until sY_back is triggered
  stepperY.move(-fieldSize - 1000);
  while (digitalRead(sY_back) == LOW) {
    stepperY.run();
  }
  //Reset the position to make this sensor as the origin
  stepperY.setCurrentPosition(0);
  minY = stepperY.currentPosition();
  Serial.print("minY: ");
  Serial.println(minY);

  // Move forward until sY_front is triggered
  stepperY.move(fieldSize - 100);  //go fast close to the second sensor
  while (digitalRead(sY_front) == LOW) {
    stepperY.run();
  }
  //and then goes really slowly to the sensor to define accurately how many steps are between the two sensors
  while (digitalRead(sY_front) == LOW) {
    stepperY.move(1);
    stepperY.run();
  }
  maxY = stepperY.currentPosition();  //we save the position of the stepper and it is now our limit in our movement
  Serial.print("maxY: ");
  Serial.println(maxY);
  //Center the stepper after estimation
  stepperY.moveTo((maxY + minY) / 2);
  stepperY.runToPosition();

  // Estimate Z-axis range (MOVE2)
  Serial.println("Estimating Z-axis range...");
  // Move forward until sZ_front is triggered
  stepperZ.move(-fieldSize - 2000);
  while (digitalRead(sZ_back) == LOW) {
    stepperZ.run();
  }
  //Reset the position to make this sensor as the origin
  stepperZ.setCurrentPosition(0);
  minZ = stepperZ.currentPosition();
  Serial.print("minZ: ");
  Serial.println(minZ);

  // Move backward until sZ_back is triggered
  stepperZ.move(fieldSize - 100);  //go fast close to the second sensor
  while (digitalRead(sZ_front) == LOW) {
    stepperZ.run();
  }
  //and then goes really slowly to the sensor to define accurately how many steps are between the two sensors
  while (digitalRead(sZ_front) == LOW) {
    stepperZ.move(1);
    stepperZ.run();
  }
  maxZ = stepperZ.currentPosition();  //we save the position of the stepper and it is now our limit in our movement
  Serial.print("maxZ: ");
  Serial.println(maxZ);
  // Center the stepper after estimation
  stepperZ.moveTo((maxZ + minZ) / 2);
  stepperZ.runToPosition();
  Serial.println("Range estimation completed.");
}
// Command Parsing Function
void executeInterpreter(String command) {
  char indiv_Cmd[20];
  int value;
  int offset = 0;
  const char* cmd_Line = command.c_str();
  //Parse multiple commands
  while (sscanf(cmd_Line + offset, "%s %d", indiv_Cmd, &value) == 2) {
    setTarget(indiv_Cmd, value);
    //set offset to next command log10(value)+1 allow to know a many numbers there is inside value
    offset += strlen(indiv_Cmd) + 1 + intLength(value);
  }
}

// Non-blocking Motor Control with Sensor Safety
void moveMotorsWithSensors() {
  // MOVE1 (stepperY)
  if (stepperY.distanceToGo() != 0) {
    if ((targetY > stepperY.currentPosition() && digitalRead(sY_front) == LOW) ||  // Moving forward, not tripping sensor
        (targetY < stepperY.currentPosition() && digitalRead(sY_back) == LOW)) {   // Moving backward, not tripping sensor
      stepperY.run();                                                              // Continue running normally
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
    if ((targetZ > stepperZ.currentPosition() && digitalRead(sZ_front) == LOW) || (targetZ < stepperZ.currentPosition() && digitalRead(sZ_back) == LOW)) {
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


void setup() {
  Serial.begin(9600);

  // Initialize steppers
  stepperY.setMaxSpeed(5000.0);
  stepperY.setAcceleration(5000.0);
  stepperZ.setMaxSpeed(5000.0);
  stepperZ.setAcceleration(5000.0);
  stepperX.setMaxSpeed(5000.0);
  stepperX.setAcceleration(5000.0);
  stepperA.setMaxSpeed(5000.0);
  stepperA.setAcceleration(5000.0);

  // Initialize pins
  pinMode(sY_front, INPUT);
  pinMode(sY_back, INPUT);
  pinMode(sZ_front, INPUT);
  pinMode(sZ_back, INPUT);
  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);  // Enable motor driver
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');  // Read and trim command
    command.trim();
    executeInterpreter(command);
  }

  moveMotorsWithSensors();  // Continuously monitor and move motors safely
}