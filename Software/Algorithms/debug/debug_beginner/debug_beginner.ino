#include <stdio.h>
#include <math.h>


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

double scaleX = fieldWidth/ cameraWidth;    //ratio of cv coordinates into the field dimension
double scaleY = fieldHeight/ cameraHeight;

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

typedef struct {
    int x;          // x-coordinate of posB
    int y;          // y-coordinate of posB
    int timestamp;  // when the frame arrived
    double a;          // Slope of the line
    double b;          // Intercept of the line
    double speed;      // Calculated speed
} Infos;

Infos ballData;

//Movement commands for players
double motorMovement[4]; // 0: Goalkeeper X, 1: Goalkeeper angle, 2: Attack rod X, 3: Attack rod angle
double playerPosition[4][2]; // Player positions: [x, angle]

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

        Serial.print("Previous Frame:\n");
        Serial.print("x: ");
        Serial.println(previousFrame.x);
        Serial.print("y: ");
        Serial.println(previousFrame.y);
        Serial.print("t: ");
        Serial.println(previousFrame.timestamp);

        if (!firstFrameReceived) {
            firstFrameReceived = true;
        }
    }

    return firstFrameReceived;
}


void setup() {

  Serial.begin(9600); //create communication with cv

}

void loop() {

    if (!getBallData()) {
        return;
    }

    ballData.x         = currentFrame.x;
    ballData.y         = currentFrame.y;
    ballData.timestamp = currentFrame.timestamp;

    Serial.print("Ball Data:\n");
    Serial.print("x: ");
    Serial.println(ballData.x);
    Serial.print("y: ");
    Serial.println(ballData.y);
    Serial.print("t: ");
    Serial.println(ballData.timestamp);

}
