#include <stdio.h>
#include <math.h>
#include "interpreter.h"

#define EN 8
#define Y_DIR 6
#define Y_STP 3
#define X_DIR 7
#define X_STP 4

//constants
#define controlSpeedThreshold 10
#define controlPositionThresholdX 15
#define controlPositionThresholdY 15
#define rod0_Y 81.5                   //position of the goalkeeper rod
#define rod1_Y 232                    //position of the attack rod
#define speedThreshold 25
#define fieldWidth   680
#define fieldHeight  605
#define cameraWidth  640
#define cameraHeight 605
#define offsetGoalie 20
#define crossFireOffset 15
#define minGoal 250
#define maxGoal 430

#define scaleX ((float)fieldWidth / cameraWidth)       //ratio of cv coordinates into the field dimension
#define scaleY ((float)fieldHeight/ cameraHeight)
#define motorUnits 1100                        // Unit of interpreter coordinates
#define physicalRangeMM 220                    // physical distance in mm corresponding to motor units
//#define motorUnitsPerMM motorUnits / physicalRangeMM
//#define fieldXToMM physicalRangeMM / fieldWidth
#define fieldXToMotorUnits motorUnits / fieldWidth  // Ratio used to convert from field X coordinate to the units required by the motors

// Define your interpreter-compatible commands as strings
#define BEGIN()          "BEGIN"                             // Move to an extreme and reset rotation
#define MOVE1(pos)      ("MOVE1 " + String(pos))              // Move towards motor by +v
#define MOVE2(pos)      ("MOVE2 " + String(pos))
#define ROTATE1(angle)  ("ROTATE1 " + String(angle))          // Rotate by angle
#define ROTATE2(angle)  ("ROTATE2 " + String(angle))
#define INITIALX()       "INITIALX"                          // Reset Position
#define INITIALY()       "INITIALY"                          // Reset Rotation

#define STEPS_PER_WM   400.0
#define STEPS_PER_REV 3200.0
#define ACCELERATION     5

Interpreter interpret(
  STEPS_PER_WM,
  &SoftwareSerial(-1, 10),
  &SoftwareSerial(-1, 2),
  &AccelStepper(1, Y_STP, Y_DIR),
  &AccelStepper(1, X_STP, X_DIR),
  STEPS_PER_REV,
  360.0 / STEPS_PER_REV,
  ACCELERATION
);

//inputs of the cv
typedef struct {
  int x;
  int y;
  int timestamp;
} FrameData;

FrameData currentFrame = {-1,-1,-1};
FrameData previousFrame = {-1,-1,-1};

bool firstFrameReceived = false;
bool secondFrameReceived = false;
int curr_x_algo = 0;

typedef struct {
    int x;          // x-coordinate of posB
    int y;          // y-coordinate of posB
    int timestamp;  // when the frame arrived
    int a;          // Slope of the line
    int b;          // Intercept of the line
    int speed;      // Calculated speed
} Infos;

Infos ballData;

//Movement commands for players
int motorMovement[4]; // 0: Goalkeeper X, 1: Goalkeeper angle, 2: Attack rod X, 3: Attack rod angle
int playerPosition[4][2]; // Player positions: [x, angle]

//retrieve ball data
bool getBallData(){
    if (Serial.available() > 0) {
        Serial.readStringUntil(':');
        int x         = Serial.readStringUntil(';' ).toInt();
        int y         = Serial.readStringUntil(';' ).toInt();
        int timestamp = Serial.readStringUntil('\n').toInt();

        previousFrame = currentFrame;
        currentFrame.x         = x;
        currentFrame.y         = y;
        currentFrame.timestamp = timestamp;

        if (!firstFrameReceived) {
            firstFrameReceived = true;
        }
    }

    return firstFrameReceived;
}

void moveField(int target_x, int* curr_x) {
  int diff = target_x - *curr_x;
  *curr_x += diff; // MIGHT NEED TO CLAMP // CHECK WHETHER IN RANGE
  interpret.executeInterpreter(MOVE1(fieldXToMotorUnits * diff));
}

void setup() {
  Serial.begin(9600); //create communication with cv

  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW); // Enable motor driver

  // Sensors for the pole 1  single player
  // sensor far from the  sideway motor, responsible for the pos unit move
  // control
  pinMode(11, INPUT);
  // sensor close to the sideway motor, responsible for the pos unit move
  // control
  pinMode(12, INPUT);

  // Sensor for the pole 2 double player
  pinMode(2, INPUT); // TODO not set up yet
  pinMode(13, INPUT);

  curr_x_algo = fieldWidth / 2;

  Serial.println("Setting up");

  interpret.executeInterpreter(MOVE1(100));
}

void loop() {

  //interpret.executeInterpreter(BEGIN());

  if (!getBallData()) {
      return;
  }

  ballData.x         = currentFrame.x;
  ballData.y         = currentFrame.y;
  ballData.timestamp = currentFrame.timestamp;

  int target_x_algo = (scaleX * ballData.x);
  moveField(curr_x_algo, target_x_algo);
  delay(4000);
}
