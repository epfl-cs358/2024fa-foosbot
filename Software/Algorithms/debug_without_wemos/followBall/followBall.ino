#include <stdio.h>
#include <math.h>
#include "CustomStepperControl.h"

//constants
#define controlSpeedThreshold 10
#define controlPositionThresholdX 15
#define controlPositionThresholdY 15
#define rod0_Y 81.5 //position of the goalkeeper rod
#define rod1_Y 232  //position of the attack rod
#define speedThreshold 25
#define fieldWidth   680
#define fieldHeight  605
#define cameraWidth  1280
#define cameraHeight 960
#define offsetGoalie 20
#define crossFireOffset 15
#define minGoal 240
#define maxGoal 420

#define scaleX ((float)fieldWidth / cameraWidth)      //ratio of cv coordinates into the field dimension
#define scaleY ((float)fieldHeight/ cameraHeight)
//#define motorUnits 1100                        // Unit of interpreter coordinates
//#define physicalRangeMM 220                    // physical distance in mm corresponding to motor units
//#define motorUnitsPerMM motorUnits / physicalRangeMM
//#define fieldXToMM physicalRangeMM / fieldWidth
#define fieldXToMotorUnits 5
//((float)motorUnits / fieldWidth) // Ratio used to convert from field X coordinate to the units required by the motors

// Define your interpreter-compatible commands as strings
#define BEGIN()          "BEGIN"                     // Move to an extreme and reset rotation
#define MOVE1(pos)      ("MOVE1 " + String(pos))     // Move towards motor by +v
#define MOVE2(pos)      ("MOVE2 " + String(pos))
#define ROTATE1(angle)  ("ROTATE1 " + String(angle)) // Rotate by angle
#define ROTATE2(angle)  ("ROTATE2 " + String(angle))
#define INITIALX()       "INITIALX"                  // Reset Position
#define INITIALY()       "INITIALY"                  // Reset Rotation

//inputs of the cv
typedef struct {
  int x;
  int y;
  int timestamp;
} FrameData;

FrameData currentFrame = {-1,-1,-1};
FrameData previousFrame = {-1,-1,-1};

bool firstFrameReceived  = false;
bool secondFrameReceived = false;
//int curr_x_algo = 0;

typedef struct {
    int x;          // x-coordinate of posB
    int y;          // y-coordinate of posB
    int timestamp;  // when the frame arrived
    int a;          // Slope of the line
    int b;          // Intercept of the line
    int speed;      // Calculated speed
} Infos;

Infos ballData;
CustomStepperControl customStepper(6, 3, 7, 4, 8, 9, 12, 2, 5);
int cur_pos = fieldWidth/2 * scaleX;

// Movement commands for players
int motorMovement[4];     // 0: Goalkeeper X,
                          // 1: Goalkeeper angle,
                          // 2: Attack rod X,
                          // 3: Attack rod angle
int playerPosition[4][2]; // Player positions: [x, angle]

// Retrieve ball data
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

void trackX(int target_pos, int* cur_pos){
  int diff = (target_pos - *cur_pos);
  customStepper.executeInterpreter(MOVE2(diff * fieldXToMotorUnits));

  int newPos = cur_pos+diff;
  *cur_pos =
    newPos < maxGoal ? newPos > minGoal ? newPos : minGoal : maxGoal;
}


void setup() {
  customStepper.setupSteppers();
  // customStepper.executeInterpreter(BEGIN());
}

void loop() {
  if (!getBallData()){
     return;
  }

  ballData.x = currentFrame.x * scaleX;
  ballData.y = currentFrame.y * scaleY;

  Serial.println(ballData.x);
  Serial.println(ballData.y);

  int target_pos = ballData.x;
  trackX(target_pos, &cur_pos);
  delay(50);
}
