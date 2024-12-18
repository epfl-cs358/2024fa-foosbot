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

// Stepper Motor Initialization
AccelStepper stepperY(1, Y_STP, Y_DIR);  // MOVE1
AccelStepper stepperZ(1, Z_STP, Z_DIR);  // MOVE2
AccelStepper stepperX(1, X_STP, X_DIR);  // ROTATE1
AccelStepper stepperA(1, A_STP, A_DIR);  // ROTATE2

// Sensor-controlled Stepper Movement Targets
int targetY = 0, targetZ = 0, targetX = 0, targetA = 0;

// Function to set stepper target positions with safety checks
void setTarget(const char *cmd, int value) {
  if (strcmp(cmd, "MOVE1") == 0) {
    targetY += value;
    stepperY.moveTo(targetY);
  } else if (strcmp(cmd, "MOVE2") == 0) {
    targetZ += value;
    stepperZ.moveTo(targetZ);
  } else if (strcmp(cmd, "ROTATE1") == 0) {
    targetX += value;
    stepperX.moveTo(targetX);
  } else if (strcmp(cmd, "ROTATE2") == 0) {
    targetA += value;
    stepperA.moveTo(targetA);
  } else {
    Serial.println("Invalid Command!");
  }
}

// Command Parsing Function
void executeInterpreter(String command) {
  char indiv_Cmd[20];
  int value;
  int offset = 0;
  const char* commd =command.c_str();

    //Parse multiple commands
  while (sscanf(commd + offset, "%s %d", indiv_Cmd, &value) == 2) {
    setTarget(indiv_Cmd, value);
    offset += strlen(indiv_Cmd) + 4; // Move offset to next command
  }
  // while (sscanf(commd + offset, "%s %d", indiv_Cmd, &value) == 2) {
  //   Serial.print("Command: ");
  //   Serial.print(indiv_Cmd);
  //   Serial.print(", Value: ");
  //   Serial.println(value);

  //   offset += strlen(indiv_Cmd) + 3;  // Advance to next command
  // }
}

// Non-blocking Motor Control with Sensor Safety
void moveMotorsWithSensors() {
  // MOVE1 (stepperY)
  if (stepperY.distanceToGo() != 0) {
    if ((targetY > stepperY.currentPosition() && digitalRead(sY_front) == LOW) ||  // Moving forward, not tripping sensor
        (targetY < stepperY.currentPosition() && digitalRead(sY_back) == LOW)) {   // Moving backward, not tripping sensor
      stepperY.run();
    } else {
      stepperY.stop();  // Safety stop if sensor triggered
      Serial.println("MOVE1 stopped due to sensor.");
    }
  }

  // MOVE2 (stepperZ)
  if (stepperZ.distanceToGo() != 0) {
    if ((targetZ > stepperZ.currentPosition() && digitalRead(sZ_front) == LOW) || 
        (targetZ < stepperZ.currentPosition() && digitalRead(sZ_back) == LOW)) {
      stepperZ.run();
    } else {
      stepperZ.stop();
      Serial.println("MOVE2 stopped due to sensor.");
    }
  }

  // ROTATE1 (stepperX) - No sensors here
  if (stepperX.distanceToGo() != 0) {
    stepperX.run();
  }

  // ROTATE2 (stepperA) - No sensors here
  if (stepperA.distanceToGo() != 0) {
    stepperA.run();
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
  digitalWrite(EN, LOW); // Enable motor driver
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // Read and trim command
    command.trim();
    executeInterpreter(command);
  }

  moveMotorsWithSensors(); // Continuously monitor and move motors safely
}