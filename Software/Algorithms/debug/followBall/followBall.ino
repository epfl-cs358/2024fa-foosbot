#include <stdio.h>
#include <math.h>
#include "CustomStepperControl.h"

/*************
 * Constants *
 *************/

#define controlSpeedThreshold 10
#define controlPositionThresholdX 15
#define controlPositionThresholdY 15
#define rod0_Y 81.5 //position of the goalkeeper rod
#define rod1_Y 232  //position of the attack rod
#define speedThreshold 25
#define fieldWidth   680
#define fieldHeight  605
#define cameraWidth  509
#define cameraHeight 455
#define offsetGoalie 20
#define crossFireOffset 15
#define minGoal 490 //510
#define maxGoal 845 //865
#define minGoalie 454
#define maxGoalie 948

// Ratio of CV coordinates into the field dimension
#define scaleX ((double)fieldWidth / cameraWidth)
#define scaleY ((double)fieldHeight/ cameraHeight)
#define fieldXToMotorUnits 5 //6.25

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
#define INITIALX()       "INITIX"


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

bool firstFrameReceived = false;
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
CustomStepperControl customStepper(
        6, 3, 7, 4, 9, 13, 5,
        2, 8, 10, 11, A3, A0
);

//double cur_pos = CVtoMM(fieldWidth/2, scaleX);

//Movement commands for players
double motorMovement[4]; // 0: Goalkeeper X,
                      // 1: Goalkeeper angle,
                      // 2: Attack rod X,
                      // 3: Attack rod angle
double playerPosition[4][2]; // Player positions: [x, angle]

//hard coding initial values for xposition of players 

//helper function to understand code better
double CVtoMM(int value, double scale){
  return value * scale;
}

void updateAttackPlayerX(double newPos){
  for (int i = 1; i < 4; i++){
    playerPosition[i][0] += newPos;
  }

}


/*
 * Retrieve ball data from CV.
 *
 * @returns frame received by CV.
 */
bool getBallData(){

    if (Serial.available() > 0) {
      Serial.println("Serial available");
      Serial.readStringUntil(':');
      int x         = Serial.readStringUntil(';' ).toInt();
      int y         = Serial.readStringUntil(';' ).toInt();

      // Check for valid x and y
      if (x <= 0 || x > cameraWidth) {
          Serial.println("Invalid data received. Ignoring frame.");
          return false; // Exit early and return false
      }

      previousFrame = currentFrame;
      currentFrame.x         = x;
      currentFrame.y         = y;

      if (!firstFrameReceived) {
        firstFrameReceived = true;
      }
    }
    return firstFrameReceived;
}

/*
 * Tracks the ball with the player.
 */
void moveField(){

  double target = CVtoMM(ballData.x, scaleX);

  if (minGoal < target && target < maxGoal) {
    //Serial.println("Moving to ball");
    double newPos0 = target-playerPosition[0][0];
    customStepper.executeInterpreter(
        MOVE1(newPos0 * fieldXToMotorUnits)
    );
    Serial.println(playerPosition[0][0]);
     double newPos1 = target - playerPosition[2][0];
     customStepper.executeInterpreter(
         MOVE2(newPos1 * fieldXToMotorUnits)
     ) ;
    playerPosition[0][0] += newPos0;
    Serial.println(playerPosition[0][0]);
    updateAttackPlayerX(newPos1);
    
  } else if (target < minGoal){
     //Serial.println("Moving to minGoal");
    double newPos = minGoal-playerPosition[0][0];
    customStepper.executeInterpreter(
         MOVE1(newPos * fieldXToMotorUnits)
    );
  //   double newPos = target - playerPosition[1][0];
  //   customStepper.executeInterpreter(
  //      MOVE2(newPos * fieldXToMotorUnits)
  //   );
    playerPosition[0][0] += newPos;
  //   updateAttackPlayerX(newPos);
    } else {
     //Serial.println("Moving to maxGoal");
    double newPos = maxGoal-playerPosition[0][0];
    customStepper.executeInterpreter(
        MOVE1(newPos * fieldXToMotorUnits)
     );
  //   double newPos = target - playerPosition[3][0];
  //   customStepper.executeInterpreter(
  //      MOVE2(newPos * fieldXToMotorUnits)
  //   );
    playerPosition[0][0] += newPos;
  //   updateAttackPlayerX(newPos);
  }
}

// /*
//  * Shoots the ball.
//  */
// void shootBall()
// {
  
// }


void setup() {
  
  Serial.begin(9600);
  playerPosition[0][0] = CVtoMM(694, scaleX);
  playerPosition[1][0] = CVtoMM(300, scaleX);
  playerPosition[2][0] = CVtoMM(696, scaleX);
  playerPosition[3][0] = CVtoMM(1060, scaleX);
  customStepper.setupSteppers();
  customStepper.executeInterpreter(BEGIN());

}

void loop() {

  if (!getBallData()){
     return;
  }

  ballData.x = CVtoMM(currentFrame.x, scaleX);
  ballData.y = CVtoMM(currentFrame.y, scaleY);

  Serial.println(ballData.x);
  Serial.println(ballData.y);

  //int target = ballData.x;

  moveField();
  //delay(50);
}
