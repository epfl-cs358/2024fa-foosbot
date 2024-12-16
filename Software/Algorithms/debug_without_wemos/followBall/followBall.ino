#include <stdio.h>
#include <math.h>
#include "CustomStepperControl.h"

/*************
 * Constants *
 *************/

#define controlSpeedThreshold 10
#define controlPositionThresholdX 15
#define controlPositionThresholdY 15
#define rod0_Y 81.5 // Position of the goalkeeper rod
#define rod1_Y 232  // Position of the attack rod
#define speedThreshold 25
#define fieldWidth   680
#define fieldHeight  605
#define cameraWidth  1280
#define cameraHeight 960
#define offsetGoalie 20
#define crossFireOffset 15
#define minGoal 240
#define maxGoal 420

// Ratio of cv coordinates into the field dimension
#define scaleX ((float)fieldWidth / cameraWidth)
#define scaleY ((float)fieldHeight/ cameraHeight)
//#define motorUnits 1100      // Unit of interpreter coordinates
//#define physicalRangeMM 220  // physical distance in mm corresponding to motor units
//#define motorUnitsPerMM motorUnits / physicalRangeMM
//#define fieldXToMM physicalRangeMM / fieldWidth
#define fieldXToMotorUnits 5
//((float)motorUnits / fieldWidth) // Ratio used to convert from field X coordinate to the units required by the motors


/**********************************************************
 * Define your interpreter-compatible commands as strings *
 **********************************************************/

// Move to an extreme and reset rotation
#define BEGIN()          "BEGIN"
// Move towards motor by +v
#define MOVE1(pos)      ("MOVE1 " + String(pos))
#define MOVE2(pos)      ("MOVE2 " + String(pos))
// Rotate by angle
#define ROTATE1(angle)  ("ROTATE1 " + String(angle))
#define ROTATE2(angle)  ("ROTATE2 " + String(angle))
// Reset Position
#define INITIALX()       "INITIALX"
// Reset Rotation
#define INITIALY()       "INITIALY"

/*
 * CV Input.
 *
 * @param x         x component of ball position (width)
 * @param y         y component of ball position (height)
 * @param timestamp Attributed timestamp to the frame
 */
typedef struct {
  int x;
  int y;
  int timestamp;
} FrameData;

FrameData currentFrame = {-1,-1,-1};
FrameData previousFrame = {-1,-1,-1};

bool firstFrameReceived  = false;
bool secondFrameReceived = false;

/*
 * Internal information on the ball.
 *
 * @param x         x-coordinate of ball position
 * @param y         y-coordinate of ball position
 * @param timestamp Metrics to determine when the frame has arrived
 * @param a         Slope of the line
 * @param b         Intercept of the line
 * @param speed     Calculated speed
 */
typedef struct {
    int x;
    int y;
    int timestamp;
    int a;
    int b;
    int speed;
} Infos;

Infos ballData;
CustomStepperControl customStepper(6, 3, 7, 4, 8, 9, 12, 2, 5);
int cur_pos = fieldWidth/2 * scaleX;

int motorMovement[4]; // 0: Goalkeeper X,
                      // 1: Goalkeeper angle,
                      // 2: Attack rod X,
                      // 3: Attack rod angle
int playerPosition[4][2]; // Player positions: [x, angle]

/*
 * Retrieve ball data from CV.
 *
 * @returns frame received by CV.
 */
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

/*
 * Tracks the ball with the player.
 */
void trackBall(){
  int target = ballData.x;

  if (minGoal < target && target < maxGoal) {
    customStepper.executeInterpreter(
        MOVE2((target-cur_pos) * fieldXToMotorUnits)
    );
    cur_pos = target;
  } else {
    cur_pos = target<minGoal ? minGoal : maxGoal;
  }
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

  trackBall(&cur_pos);
  delay(50);
}
