#include <stdio.h>
#include <math.h>
#include "ParallelInterpreter.h"
//#include "ConstantsForMac.h"
#include "ConstantsForLinux.h"

/*************
 * Constants *
 *************/

// Dimensions //

#define FIELD_WIDTH   680
#define FIELD_HEIGHT  605

#define FIELD_SIZE 1300

// Scaling factors //

#define SCALE_X ((double)FIELD_WIDTH / CAM_WIDTH)
#define SCALE_Y ((double)FIELD_HEIGHT/ CAM_HEIGHT)

#define FIELD_X_TO_MU 5 // Converts from millimeters to motor units on the X
                        // axis

#define CV_TO_MM(VALUE, SCALE) (VALUE * SCALE)

// Control factors //

#define CTRL_SPEED_THR 10

// Rod positions //

#define  GL_ROD_Y 81.5 // Position of the goalkeeper rod
#define ATT_ROD_Y 232   // Position of the attack rod

// MM Constants //

#define MOVE_THR 10

#define MIN_GOAL_X_MM MIN_GOAL_X_CV * SCALE_X
#define MAX_GOAL_X_MM MAX_GOAL_X_CV * SCALE_X

#define MIN_ATT_X_MM MIN_ATT_X_CV * SCALE_X
#define MAX_ATT_X_MM MAX_ATT_X_CV * SCALE_X

#define MID_ATT_LO_MM MID_ATT_LO_CV * SCALE_X
#define MID_ATT_HI_MM MID_ATT_HI_CV * SCALE_X

#define MIN_GOAL_Y_MM MIN_GOAL_Y_CV * SCALE_Y
#define MAX_GOAL_Y_MM MAX_GOAL_Y_CV * SCALE_Y

#define MIN_ATT_Y_MM MIN_ATT_Y_CV * SCALE_Y
#define MAX_ATT_Y_MM MAX_ATT_Y_CV * SCALE_Y

#define MAX_ATT1_X_MM MAX_ATT1_X_CV * SCALE_X

#define MIN_ATT2_X_MM MIN_ATT2_X_CV * SCALE_X
#define MAX_ATT2_X_MM MAX_ATT2_X_CV * SCALE_X

#define MIN_ATT3_X_MM MIN_ATT3_X_CV * SCALE_X

#define MAX_HIT_RANGE_MM 10

// Misc //

#define SPEED_THR 25

#define         GL_OFF 20
#define CROSS_FIRE_OFF 15

/**********************************************************
 * Define your interpreter-compatible commands as strings *
 **********************************************************/

// Move to an extreme and reset rotation
#define BEGIN()          "BEGIN 100"
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
ParallelInterpreter interpret(
        6, 3,  7,  4, 13, 12, 5,
        2, 8, 10, 11, A3, A0, FIELD_SIZE
);

// TODO: Remove
//Movement commands for players
//double motorMovement[4]; // 0: Goalkeeper X,
//                      // 1: Goalkeeper angle,
//                      // 2: Attack rod X,
//                      // 3: Attack rod angle
/*
 * Player positions: MM
 *   [0: gl, 1: att1, 2: att2, 3: att3][x, angle]
 */
double playerPosition[4][2];

double pos_yz[2];

/*
 * Scales by the given factor to get millimeters from CV input.
 *
 * @param value Value to be scaled
 * @param scale Scaling to be applied, should be either `SCALE_X` or `SCALE_Y`
 */
// double CVtoMM(int value, double scale){
//   return value * scale;
// }

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
      // Flush incoming serial
      Serial.end();
      Serial.begin(9600);

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
 * Returns the closest player to the ball according to given x.
 *
 * @param plyr1 First player to compare
 * @param plyr2 Second player to compare
 * @param x     x-coordinate of ball position
 *
 * @returns the closest player to ball.
 */
int defendingAttPlayer(int p1, int p2, double x)
{
  return
    abs(playerPosition[p1][0] - x) < abs(playerPosition[p2][0] - x) ? p1 : p2;
}

/*
 * Tracks the ball with the players.
 */
void moveField(){

  double target = ballData.x;


  // Attack players position //

  double distAtt = 0;
  if (target < MIN_ATT_X_MM) {
    // Serial.println("Moving attack to minAtt");
    distAtt = MIN_ATT_X_MM - playerPosition[1][0];

  } else if (MAX_ATT_X_MM < target) {
    // Serial.println("Moving attack to maxAtt");
    distAtt = MAX_ATT_X_MM - playerPosition[3][0];

  } else if (MIN_ATT_X_MM < target && target < MIN_ATT2_X_MM){
    distAtt = target - playerPosition[1][0];

  } else if (MIN_ATT2_X_MM < target && target < MAX_ATT1_X_MM){
    distAtt = target - playerPosition[defendingAttPlayer(1,2, target)][0];

  } else if (MAX_ATT1_X_MM < target && target < MIN_ATT3_X_MM){
    distAtt = target - playerPosition[2][0];

  } else if (MIN_ATT3_X_MM < target && target  < MAX_ATT2_X_MM){
    distAtt = target - playerPosition[defendingAttPlayer(2,3, target)][0];

  } else {
    distAtt = target - playerPosition[3][0];

  }


  // Goalie position //

  double distGl = 0;
  double glPos  = playerPosition[0][0];

  if (MIN_GOAL_X_MM < target && target < MAX_GOAL_X_MM) {
    // Serial.println("Moving goalie to ball");
    distGl = target - glPos;

  } else if (target < MIN_GOAL_X_MM) {
    // Serial.println("Moving goalie to minGoal");
    distGl = MIN_GOAL_X_MM - glPos;

  } else {
    // Serial.println("Moving goalie to maxGoal");
    distGl = MAX_GOAL_X_MM - glPos;
  }


  // Move //

  if (abs(distAtt) >= MOVE_THR) {
    if  (abs(distGl) >= MOVE_THR) {

      interpret.executeInterpreter(
        MOVE2(distAtt * FIELD_X_TO_MU) +
        " " +
        MOVE1(distGl * FIELD_X_TO_MU)
      );

      playerPosition[0][0] += distGl;

    } else {

      interpret.executeInterpreter(
        MOVE2(distAtt * FIELD_X_TO_MU)
      );
    }

    updateAttackPlayerX(distAtt);

  } else if (abs(distGl) >= MOVE_THR) {
    interpret.executeInterpreter(
      MOVE1(distGl * FIELD_X_TO_MU)
    );

    playerPosition[0][0] += distGl;

  }
}

//helper function
int getClosestPlayer(){

  // Calulate the euclidian distance for the goalkeeper
  double distance = sqrt(pow(playerPosition[0][0] - ballData.x, 2) +
                         pow(GL_ROD_Y- ballData.y, 2));
  double minDistance = distance;
  double closestIndex = 0;

  //Calulate the euclidian distance to the ball for each player
  for(int i = 1; i < 4; i++) {

    // Calculate the Euclidean distance to the ball
    double distance = sqrt(pow(playerPosition[i][0] - ballData.x, 2) + pow(ATT_ROD_Y - ballData.y, 2));

    // Update the closest player if this distance is smaller
    if (distance < minDistance) {
      minDistance = distance;
      closestIndex = i;
    }
  }

  return closestIndex;

}

/*
 * Checks if shoot available and shoots the ball according to the given
 * range if it is.
 *
 * @param rangeLo The smallest accessible location to the player
 * @param rangeHi The largest accessible location to the player
 */
void checkAndShoot()
{

  //check player that attacks and add treshold
  int index = getClosestPlayer();

  if(index == 0) {
    int x_min = playerPosition[0][0] - MAX_HIT_RANGE_MM;
    int x_max = playerPosition[0][0] + MAX_HIT_RANGE_MM;

    if (MIN_GOAL_Y_MM < ballData.y && ballData.y < MAX_GOAL_Y_MM
    &&  x_min < ballData.x && ballData.x < x_max) {
    Serial.println("Shooting with Goalie !");
    interpret.executeInterpreter(
      ROTATE1(-200)
    );
    }
  }

  if (MIN_GOAL_Y_MM < ballData.y &&
      ballData.y < MAX_GOAL_Y_MM) {
    Serial.println("Shooting with Goalie !");
    interpret.executeInterpreter(
      ROTATE1(-200)
    );
  }

  if (MIN_ATT_Y_MM < ballData.y &&
      ballData.y < MAX_ATT_Y_MM) {
    Serial.println("Shooting with Attacker !");
    interpret.executeInterpreter(
      ROTATE2(-200)
    );
  }

  delay(500);
}


void setup() {

  Serial.begin(9600);
  playerPosition[0][0] = CV_TO_MM(PLAYER_0_X, SCALE_X);
  playerPosition[1][0] = CV_TO_MM(PLAYER_1_X, SCALE_X);
  playerPosition[2][0] = CV_TO_MM(PLAYER_2_X, SCALE_X);
  playerPosition[3][0] = CV_TO_MM(PLAYER_3_X, SCALE_X);

  interpret.setupSteppers();
  interpret.executeInterpreter(BEGIN());

}

void loop() {

  if (!getBallData()){
     return;
  }

  Serial.println(currentFrame.y);


  ballData.x = CV_TO_MM(currentFrame.x, SCALE_X);
  ballData.y = CV_TO_MM(currentFrame.y, SCALE_Y);

  Serial.println(currentFrame.y);

  moveField();
  checkAndShoot();

  interpret.moveMotorsWithSensors();
}
