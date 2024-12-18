#include <stdio.h>
#include <math.h>
#include "CustomStepperControl.h"

/*************
 * Constants *
 *************/

// Dimensions //

#define FIELD_WIDTH   680
#define FIELD_HEIGHT  605

#define CAM_WIDTH  509
#define CAM_HEIGHT 455

// Scaling factors //

#define SCALE_X ((double)FIELD_WIDTH / CAM_WIDTH)
#define SCALE_Y ((double)FIELD_HEIGHT/ CAM_HEIGHT)

#define FIELD_X_TO_MU 5 // Converts from millimeters to motor units on the X
                        // axis

// Control factors // TODO: Remove

#define CTRL_SPEED_THR 10
#define CTRL_POS_THR_X 15
#define CTRL_POS_THR_Y 15

// Rod positions //

#define  GL_ROD_POS  81.5 // Position of the goalkeeper rod
#define ATT_ROD_POS 232   // Position of the attack rod


// CV Constants //

#define MIN_GOAL_CV 170
#define MAX_GOAL_CV 350

#define MIN_ATT_CV  62
#define MAX_ATT_CV 936

#define MID_ATT_LO_CV 166
#define MID_ATT_HI_CV 334

// MM Constants //

#define MOVE_THR 10

#define MIN_GOAL_MM MIN_GOAL_CV * SCALE_X
#define MAX_GOAL_MM MAX_GOAL_CV * SCALE_X

#define MIN_ATT_MM MIN_ATT_CV * SCALE_X
#define MAX_ATT_MM MAX_ATT_CV * SCALE_X

#define MID_ATT_LO_MM MID_ATT_LO_CV * SCALE_X
#define MID_ATT_HI_MM MID_ATT_HI_CV * SCALE_X

// Misc // TODO: Remove

#define SPEED_THR 25

#define         GL_OFF 20
#define CROSS_FIRE_OFF 15

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
 * @param x         x component of ball position (width ): CV
 * @param y         y component of ball position (height): CV
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
 * @param x         x-coordinate of ball position: MM
 * @param y         y-coordinate of ball position: MM
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
} Info;

Info ballData;
CustomStepperControl customStepper(
        6, 3, 7, 4, 9, 13, 5,
        2, 8, 10, 11, A3, A0
);

// TODO: Remove
//Movement commands for players
double motorMovement[4]; // 0: Goalkeeper X,
                      // 1: Goalkeeper angle,
                      // 2: Attack rod X,
                      // 3: Attack rod angle
/*
 * Player positions: MM
 *   [0: gl, 1: att1, 2: att2, 3: att3][x, angle]
 */
double playerPosition[4][2];

/*
 * Scales by the given factor to get millimeters from CV input.
 *
 * @param value Value to be scaled
 * @param scale Scaling to be applied, should be either `SCALE_X` or `SCALE_Y`
 */
double CVtoMM(int value, double scale){
  return value * scale;
}

/*
 * Updates the position of the attack players.
 *
 * @param dist Distance that has been covered by the players
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
      Serial.read(); // Flush the incoming serial to have last frame
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

  double target = ballData.x;


  // Moving attack players //

  double distAtt = 0;
  if (target < MIN_ATT_MM) {
    // Serial.println("Moving attack to minAtt");
    distAtt = MIN_ATT_MM - playerPosition[1][0];

  } else if (MAX_ATT_MM < target) {
    // Serial.println("Moving attack to maxAtt");
    distAtt = MAX_ATT_MM - playerPosition[3][0];

  } else {
    // Serial.println("Moving attack to ball");
    distAtt = target - playerPosition[
      MID_ATT_LO_MM < target ?
        target < MID_ATT_HI_MM ?
          2 : // Middle player
        3 :   // Last player
      1       // First player
    ][0];
  }

  if (abs(distAtt) >= MOVE_THR) {
    customStepper.executeInterpreter(
      MOVE2(distAtt * FIELD_X_TO_MU)
    );
    updateAttackPlayerX(distAtt);
  }


  // Moving goalie //

  double distGl = 0;
  double glPos  = playerPosition[0][0];

  if (MIN_GOAL_MM < target && target < MAX_GOAL_MM) {
    // Serial.println("Moving goalie to ball");
    distGl = target - glPos;

  } else if (target < MIN_GOAL_MM) {
    // Serial.println("Moving goalie to minGoal");
    distGl = MIN_GOAL_MM - glPos;

  } else {
    // Serial.println("Moving goalie to maxGoal");
    distGl = MAX_GOAL_MM - glPos;
  }

  if (abs(distGl) >= MOVE_THR) {
    customStepper.executeInterpreter(
      MOVE1(distGl * FIELD_X_TO_MU)
    );
    playerPosition[0][0] += distGl;
  }
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

  Serial.println(ballData.x);
  Serial.println(ballData.y);

  moveField();
}
