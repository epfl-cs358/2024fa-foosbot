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

int scaleX = fieldWidth/ cameraWidth;    //ratio of cv coordinates into the field dimension
int scaleY = fieldHeight/ cameraHeight;

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
        int x         = Serial.readStringUntil(';').toInt();
        int y         = Serial.readStringUntil(';').toInt();
        int timestamp = millis();

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

        Serial.print("Current Frame:\n");
        Serial.print("x: ");
        Serial.println(currentFrame.x);
        Serial.print("y: ");
        Serial.println(currentFrame.y);
        Serial.print("t: ");
        Serial.println(currentFrame.timestamp);

        if (!firstFrameReceived) {
            firstFrameReceived = true;
        }
    }

    return firstFrameReceived;
}

//get ball.a, ball.b, ball.speed
void calculateBallTrajectory(){

  if (ballData.timestamp > -1){
    int deltaTime = ballData.timestamp - previousFrame.timestamp;

    if (deltaTime > 0){

      //complete linear equation ax + b
      if(ballData.x != previousFrame.x){
        ballData.a = (ballData.y - previousFrame.y) / (ballData.x - previousFrame.x); // a = y2-y1/ x2-x1
        ballData.b = ballData.y - ballData.a * ballData.x; // b = y2 - a * x2
      }
      else{
        ballData.a = 0;
        ballData.b = ballData.x; //Vertical line stays at x position
      }

    //calculate speed
    ballData.speed = sqrt(pow((ballData.x - previousFrame.x), 2) + pow((ballData.y - previousFrame.y), 2)) / deltaTime; // v = d/t
    }
  }
  else{
    ballData.a     = 0; //first time function is called
    ballData.b     = ballData.y;
    ballData.speed = 0;
  }

  Serial.print("Speed: \n");
  Serial.println(ballData.speed);
  Serial.print("a: ");
  Serial.println(ballData.a);
  Serial.print("b: ");
  Serial.println(ballData.b);
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

    calculateBallTrajectory();
}
