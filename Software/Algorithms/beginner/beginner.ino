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
#define motorUnits 350
#define physicalRangeMM 70

double scaleX = fieldWidth/ cameraWidth;    //ratio of cv coordinates into the field dimension
double scaleY = fieldHeight/ cameraHeight;
double motorScalingFactor = motorUnits / physicalRangeMM ;

//inputs of the cv
typedef struct {
  double x;
  double y;
  double timestamp;
} FrameData;

FrameData currentFrame = {-1,-1,-1};
FrameData previousFrame = {-1,-1,-1};

bool firstFrameReceived = false;
bool secondFrameReceived = false;

typedef struct {
    double x;          // x-coordinate of posB
    double y;          // y-coordinate of posB
    double timestamp;  // when the frame arrived
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

  //process all lines in the buffer
  while (Serial.available() > 0){

    //String fromatted as ":x;y;timestamp"
    String data = Serial.readStringUntil('\n');
    double newX, newY, newTimestamp;
    if(sscanf(data.c_str(), ":%lf;%lf;%lf", &newX, &newY, &newTimestamp) == 3){

      Serial.print(newX);
      Serial.print(newY);
      Serial.print(newTimestamp);
      //update frame data
      previousFrame = currentFrame;
      currentFrame.x = newX * scaleX;  //map camera coordinates to field coordinates
      currentFrame.y = newY * scaleY;
      currentFrame.timestamp = newTimestamp;

      // Update frame reception
      if (!firstFrameReceived) {
        firstFrameReceived = true; // The first frame has been received
      }
    }
  }

  ballData.x = currentFrame.x;
  ballData.y = currentFrame.y;
  ballData.timestamp = currentFrame.timestamp;

  return firstFrameReceived;

}

//get ball.a, ball.b, ball.speed
void calculateBallTrajectory(){

  static double previousX = 0;
  static double previousY = 0;
  static double previousTime = 0; //ms

  if (currentFrame.timestamp > -1){
    double deltaTime = ballData.timestamp - previousTime;

    if (deltaTime > 0){

      //complete linear equation ax + b
      if(ballData.x != previousX){
        ballData.a = (ballData.y - previousY) / (ballData.x - previousX); // a = y2-y1/ x2-x1
        ballData.b = ballData.y - ballData.a * ballData.x; // b = y2 - a * x2
      }
      else{
        ballData.a = 0;
        ballData.b = ballData.x; //Vertical line stays at x position
      }

    //calculate speed
    ballData.speed = sqrt(pow((ballData.x - previousX), 2) + pow((ballData.y - previousY), 2)) / deltaTime; // v = d/t
    }
  }
  else{
    ballData.a = 0; //first time function is called
    ballData.b = currentFrame.y;
    ballData.speed = 0;
  }

  //update variables
  previousX = ballData.x;
  previousY = ballData.y;
  previousTime = ballData.timestamp;

}

//helper function
int getClosestAttackPlayerIndex(double x, double y){

  int closestIndex = -1;
  double minDistance = 1e5;

  //Calulate the euclidian distance to the ball for each player
  for (int i = 1; i < 4; i++) { //only attack players (index 1, 2, 3)
    double playerX = playerPosition[i][0];
    double playerY = rod1_Y;

    // Calculate the Euclidean distance to the ball
    double distance = sqrt(pow((x - playerX), 2) + pow((y - playerY), 2));

     // Update the closest player if this distance is smaller
    if (distance < minDistance) {
      minDistance = distance;
      closestIndex = i;
    }
  }

  return closestIndex;

}

bool isBallControlled(){

  int closestAttackerToBall = getClosestAttackPlayerIndex(ballData.x, ballData.y);

  //check if ball is controlled by the goalkeeper
  bool isNearRod0 = (abs(ballData.x - playerPosition[0][0]) <= controlPositionThresholdX) &&
                      (abs(ballData.y - rod0_Y) <= controlPositionThresholdY);

  //check if ball is controlled by a player of the attack rod
  bool isNearRod1 = (abs(ballData.x - playerPosition[closestAttackerToBall][0]) <= controlPositionThresholdX) &&
                      (abs(ballData.y - rod1_Y) <= controlPositionThresholdY);

  //check that the ball is slow enough
  bool isSpeedLow = ballData.speed <= controlSpeedThreshold;

  return (isNearRod0 || isNearRod1) && isSpeedLow;

}

//helper function
void rotateAndUpdatePlayers(double y){
  motorMovement[3] = y;
  ROTATE(y);
  for (int i = 1; i < 4; i++){
      playerPosition[i][1] = y;
  }
}

//helper function
void translateAndUpdatePlayers(double x){
  motorMovement[2] = x * motorScalingFactor;
  MOVE(motorMovement[2]);
  for (int i = 1; i < 4; i++){
      playerPosition[i][0] += x;
  }
}

void takeDefensePosition(){

  //find player closest to new pos of the ball and calculate the distance
  double predictedX = (rod1_Y - ballData.b)/ ballData.a;
  double defenderIndex = getClosestPlayerIndexAttack(predictedX, rod1_Y);
  double defenderX = playerPosition[defenderIndex][0];
  int rodTranslation = predictedX - defenderX;

  //rotate player accordingly to defense positions
  if (abs(ballData.speed) <= speedThreshold){
    ballData.speed >= 0 ? rotateAndUpdatePlayers(-30) : rotateAndUpdatePlayers(30);
  }
  else{
    rotateAndUpdatePlayers(-30);
  }

  //move rod to intercept ball
  translateAndUpdatePlayers(rodTranslation);

  //offset gardian depending on the half of the field the ball comes from
  if(ballData.x <= fieldWidth/2){
    motorMovement[0] = rodTranslation - offsetGoalie;  //offset the goalkeeper to the left of the attacker
    MOVE(motorMovement[0])
    playerPosition[0][0] += rodTranslation - offsetGoalie;
  }else{
    motorMovement[0] = (rodTranslation + offsetGoalie) * motorScalingFactor;
    MOVE(motorMovement[0])                            //offset the goalkeeper to the right of the attacker
    playerPosition[0][0] += rodTranslation + offsetGoalie;
  }

}

//helper function
void alignMiddlePlayerWithGoalkeeper(){

  double alignement = playerPosition[0][0] - playerPosition[2][0];
  double receivePass = 30;

  translateAndUpdatePlayers(alignement);
  rotateAndUpdatePlayers(receivePass);

}

//helper function
void passBallToMiddlePlayer(){

  while(rod0_Y + controlPositionThresholdY / 2){
    motorMovement[1] -= 5;
    ROTATE(-5);
    playerPosition[0][1] -5= ;
  }

}

//helper function
void hitBallFront(){

  motorMovement[3] = -360;
  ROTATE(-360);
  INITALY;

}

//helper function
void hitBallBack(){

  motorMovement[3] = -60;
  ROTATE(-60);
  INITALY;

}

//helper function
void liftAttackerAndShoot(double x){

  motorMovement[3] -= 10; //lift player up to the front
  ROTATE(motorMovement[3]);
  motorMovement[2] -= x * motorScalingFactor;  //offset player
  MOVE(motorMovement[2]);
  hitBallFront();         //hit ball

}

//helper function
void liftAttackerAndPush(double x){

  motorMovement[3] += 30;  //lift player up to the back
  ROTATE(motorMovement[3]);
  motorMovement[2] += x * motorScalingFactor;   //offset player
  MOVE(motorMovement[2]);
  motorMovement[3] -= 30;  //push the ball
  ROTATE(motorMovement[3]);
  INITALY;

}

void takeAttackPosition(){

  //if the goalkeeper has the ball pass it to the attack
  if(ballData.y <= rod0_Y + controlPositionThresholdY/2){
    alignMiddlePlayerWithBall();
    passBallToMiddlePlayer();
  }
  else{

    // Decide on a direct attack or crossed attack based on ball position (interval checks)
    if (ballData.x >= minGoal && ballData.x <= maxGoal) {
      (motorMovement[3] < 0 )? hitBallFront() : hitBallBack(); // Ball is in goal range, direct attack
    } else if (ballData.x < minGoal) {
      (motorMovement[3] < 0 )? liftAttackerAndShoot(crossFireOffset) : liftLeftAttackerAndPush(crossFireOffset);
    } else {
      (motorMovement[3] < 0 )? liftAttackerAndShoot(-crossFireOffset) : liftLeftAttackerAndPush(-crossFireOffset);
    }
  }

}

void setup() {

  Serial.begin(9600); //create communication with cv

}

void loop() {

    // Retrieve ball data from computer vision system
    if(!getBallData()){
      return;
    }

    calculateBallTrajectory();

    // Defensive or offensive positioning based on ball control
    if (!isBallControlled()) {
        takeDefensePosition();
    } else {
        takeAttackPosition();
    }
}
