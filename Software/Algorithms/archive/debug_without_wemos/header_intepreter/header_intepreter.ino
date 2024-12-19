#include <stdio.h>
#include <math.h>
#include "CustomStepperControl.h"

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
#define INITIALY()       "INITIALY" 

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
CustomStepperControl customStepper(6, 3, 7, 4, 8, 11, 12, 2, 13);

//Movement commands for players
int motorMovement[4]; // 0: Goalkeeper X, 1: Goalkeeper angle, 2: Attack rod X, 3: Attack rod angle
int playerPosition[4][2]; // Player positions: [x, angle]

//retrieve ball data
bool getBallData(){

    Serial.println("hey");
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


void setup() {
  // put your setup code here, to run once:
  customStepper.setupSteppers();
  customStepper.executeInterpreter(MOVE1(-100));
  getBallData();
  ballData.x         = currentFrame.x;
  ballData.y         = currentFrame.y;
  ballData.timestamp = currentFrame.timestamp;
  Serial.println(ballData.x);
  Serial.println(ballData.y);
  Serial.println(ballData.timestamp);


}

void loop() {
  // put your main code here, to run repeatedly:

}
