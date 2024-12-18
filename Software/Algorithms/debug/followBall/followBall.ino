#include <stdio.h>
#include <math.h>
#include "CustomStepperControl.h"

/*************
 * Constants *
 *************/

#define CTRL_SPEED_THR 10
#define CTRL_POS_THR_X 15
#define CTRL_POS_THR_Y 15

#define  GL_ROD_POS  81.5 // Position of the goalkeeper rod
#define ATT_ROD_POS 232   // Position of the attack rod

#define SPEED_THR 25

#define FIELD_WIDTH   680
#define FIELD_HEIGHT  605

#define CAM_WIDTH  509
#define CAM_HEIGHT 455

#define         GL_OFF 20
#define CROSS_FIRE_OFF 15

// #define MIN_GOAL_CV 490 // 510
// #define MAX_GOAL_CV 845 // 865
#define MIN_GOAL_CV 170
#define MAX_GOAL_CV 350

#define MIN_ATT_CV  62
#define MAX_ATT_CV 936

// #define minGoalie 454
// #define maxGoalie 948
#define MID_ATT_LO 166
#define MID_ATT_HI 334

// Ratio of CV coordinates into the field dimension
#define SCALE_X ((double)FIELD_WIDTH / CAM_WIDTH)
#define SCALE_Y ((double)FIELD_HEIGHT/ CAM_HEIGHT)

#define FIELD_X_TO_MU 5 // 6.25

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
 * @param x         x component of ball position in CV metric (width)
 * @param y         y component of ball position in CV metric (height)
 * @param timestamp Attributed timestamp to the frame
 */
typedef struct {
  int x;
  int y;
  int timestamp;
} FrameData;

FrameData currentFrame  = {-1, -1, -1};
FrameData previousFrame = {-1, -1, -1};

bool firstFrameReceived  = false;
bool secondFrameReceived = false;

/*
 * Internal information on the ball.
 *
 * @param x         x-coordinate of ball position in millimeters
 * @param y         y-coordinate of ball position in millimeters
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

//double cur_pos = CVtoMM(FIELD_WIDTH/2, SCALE_X);

//Movement commands for players
double motorMovement[4]; // 0: Goalkeeper X,
                      // 1: Goalkeeper angle,
                      // 2: Attack rod X,
                      // 3: Attack rod angle
/*
 * Player positions in millimeters:
 *   [0: gl, 1: att1, 2: att2, 3: att3][x, angle]
 */
double playerPosition[4][2];

//hard coding initial values for xposition of players

/*
 * TODO: Doc
 */
//helper function to understand code better
double CVtoMM(int value, double scale){
  return value * scale;
}

/*
 * TODO: Doc
 */
void updateAttackPlayerX(double dist){
  for (int i = 1; i < 4; i++){
    playerPosition[i][0] += dist;
  }
}


/*
 * Retrieve ball data from CV.
 *
 * @returns frame received by CV.
 */
bool getBallData(){

    if (Serial.available() > 0) {
      Serial.readStringUntil(':');
      int x = Serial.readStringUntil(';').toInt();
      int y = Serial.readStringUntil(';').toInt();

      // Check for valid x and y
      if (x <= 0 || x > CAM_WIDTH) {
          Serial.println("Invalid data received. Ignoring frame.");
          return false; // Exit early and return false
      }

      previousFrame  = currentFrame;
      currentFrame.x = x;
      currentFrame.y = y;

      if (!firstFrameReceived) {
        firstFrameReceived = true;
      }
    }
    return firstFrameReceived;
}

/*
 * Tracks the ball with the players.
 */
void moveField(){

  // No need for that, x is already in mm
  // double target = CVtoMM(ballData.x, SCALE_X);
  double target = ballData.x / SCALE_X; // Switching back to CV


  // Moving goalie //

  double distGl = 0;
  double glPos  = playerPosition[0][0] / SCALE_X; // Switching to CV

  if (MIN_GOAL_CV < target && target < MAX_GOAL_CV) {
    // Serial.println("Moving goalie to ball");
    distGl = target - glPos;

  } else if (target < MIN_GOAL_CV) {
    // Serial.println("Moving goalie to minGoal");
    distGl = MIN_GOAL_CV - glPos;

  } else {
    // Serial.println("Moving goalie to maxGoal");
    distGl = MAX_GOAL_CV - glPos;
  }

  customStepper.executeInterpreter(
    MOVE1(distGl * FIELD_X_TO_MU)
  );
  playerPosition[0][0] += distGl;


  // Moving attack players //

  double distAtt = 0;
  if (target < MIN_ATT_CV) {
    // Serial.println("Moving attack to minAtt");
    distAtt = MIN_ATT_CV - playerPosition[1][0];

  } else if (MAX_ATT_CV < target) {
    // Serial.println("Moving attack to maxAtt");
    distAtt = MAX_ATT_CV - playerPosition[3][0];

  } else {
    // Serial.println("Moving attack to ball");
    distAtt = target - playerPosition[
      MID_ATT_LO < target ?
        target < MID_ATT_HI ?
          2 : // Middle player
        3 :   // Last player
      1       // First player
    ][0];
  }

  customStepper.executeInterpreter(
    MOVE2(distAtt * FIELD_X_TO_MU)
  );
  updateAttackPlayerX(distAtt);
}


void setup() {

  Serial.begin(9600);
  // playerPosition[0][0] = CVtoMM( 694, SCALE_X); // 258
  // playerPosition[1][0] = CVtoMM( 300, SCALE_X); // 110
  // playerPosition[2][0] = CVtoMM( 696, SCALE_X); // 258
  // playerPosition[3][0] = CVtoMM(1060, SCALE_X); // 390

  playerPosition[0][0] = CVtoMM(258, SCALE_X);
  playerPosition[1][0] = CVtoMM(110, SCALE_X);
  playerPosition[2][0] = CVtoMM(258, SCALE_X);
  playerPosition[3][0] = CVtoMM(390, SCALE_X);

  customStepper.setupSteppers();
  customStepper.executeInterpreter(BEGIN());
}

void loop() {

  if (!getBallData()){
     return;
  }

  ballData.x = CVtoMM(currentFrame.x, SCALE_X);
  ballData.y = CVtoMM(currentFrame.y, SCALE_Y);

  Serial.println(currentFrame.x);
  Serial.println(currentFrame.y);

  moveField();
}
