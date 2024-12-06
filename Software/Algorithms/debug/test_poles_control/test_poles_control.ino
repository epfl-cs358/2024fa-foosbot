// Inspired by https://gist.github.com/edgar-bonet/607b387260388be77e96
#include <stdio.h>
#include <math.h>

//constants
#define controlSpeedThreshold 10
#define controlPositionThresholdX 15
#define controlPositionThresholdY 15
#define rod0_Y 81.5                           //position of the goalkeeper rod in   coordinates of game logic
#define rod1_Y 232                            //position of the attack rod in coordinates of game logic
#define speedThreshold 25
#define fieldWidth   680                      // Width in coordinate system of game logic
#define fieldHeight  605                      // Height in coordinate system of game logic
#define cameraWidth  640                      // Width of received Frame
#define cameraHeight 605                      // Height of received Frame
#define scaleX fieldWidth / cameraWidth       //ratio of cv coordinates into the field dimension
#define scaleY fieldHeight/ cameraHeight
#define motorUnits 1100                        // Unit of interpreter coordinates
#define physicalRangeMM 220                    // physical distance in mm corresponding to motor units
//#define motorUnitsPerMM motorUnits / physicalRangeMM 
//#define fieldXToMM physicalRangeMM / fieldWidth
#define fieldXToMotorUnits motorUnits / fieldWidth  // Ratio used to convert from field X coordinate to the units required by the motors

#define BEGIN(p)          // Move to an extreme and reset rotation
#define MOVE(p, v)        // Move towards motor by +v
#define ROTATE(p, angle)  // Rotate by angle
#define INITIALX(p)       // Reset Position
#define INITIALY(p)       // Reset Rotation


void setup() {
  Serial.begin(9600);
}

void loop() {

  int poleId = 0; // Change this to decide what motor to control
  delay(2000);

  // INITIAL STEPS

  // Set the Pole to the start position (in the middle of the field)
  BEGIN(motorId);
  int curr_x_algo = fieldWidth / 2;
  delay(2000);
  // TEST ROTATION
  INITIALY(motorId); // Should bring rotation
  delay(1000);

  // SIMPLE ROTATIONS
  ROTATE(motorId, -30); // Should move back (shooting position)
  delay(1000);
  ROTATE(motorId, 30); // Should get back to initial rotation
  delay(1000);
  ROTATE(motorId, 30); // Gets into position for receiving a pass
  delay(1000);
  INITIALY(motorId);
  delay(1000);

  // Do a full Revolution
  ROTATE(motorId, 360); // In one direction
  delay(2000);
  ROTATE(motorId, -360); // In the other direction
  delay(2000);

  // Simulate a shot
  ROTATE(motorId, -30); // Should move back (shooting position)
  delay(1000);
  ROTATE(motorId, 60); // Should go from -30° to +30°
  delay(1000);
  INITIALY(motorId);


  // LINEAR MOTION

  // Example of conversion from camera coordinates to field coordinates
  int target_x_cam = cameraHeight;
  int target_x_algo = (scaleX * target_x_cam);

  // Move to the extreme on the side towards the motor
  moveField(poleId, target_x_algo, &curr_x_algo);
  delay(4000);
  // Move the pole to the extreme opposite of the motor
  target_x_algo = 0;
  moveField(poleId, target_x_algo, &curr_x_algo);
  delay(4000);

  // Move back to the middle (& update current_pos)
  centerPole(poleId, &curr_x_algo);
  delay(2000);
}

// Moves the pole according to the given target & current X coordinates.
// Note that this function takes arguments in field coordinates (used by the algorithm).
void moveField(int pole, int target_x, int* curr_x) {
  int diff = target_x - *curr_x;
  *curr_x += diff; // MIGHT NEED TO CLAMP / CHECK WHETHER IN RANGE
  MOVE(pole, fieldXToMotorUnits * diff);
}

// Moves the pole back to the middle (initial X coordinate)
// and updates the current X coordinate
void centerPole(int pole, int* curr_x) {
  INITIALX(pole);
  *curr_x = fieldWidth / 2;
}