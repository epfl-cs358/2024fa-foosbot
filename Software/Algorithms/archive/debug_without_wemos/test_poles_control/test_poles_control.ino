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
#define cameraHeight 480                      // Height of received Frame
#define scaleX ((float)fieldWidth / cameraWidth)       //ratio of cv coordinates into the field dimension
#define scaleY ((float)fieldHeight/ cameraHeight)
#define motorUnits 1100                        // Unit of interpreter coordinates
#define physicalRangeMM 220                    // physical distance in mm corresponding to motor units
//#define motorUnitsPerMM motorUnits / physicalRangeMM 
//#define fieldXToMM physicalRangeMM / fieldWidth
#define fieldXToMotorUnits motorUnits / fieldWidth  // Ratio used to convert from field X coordinate to the units required by the motors

// Define your interpreter-compatible commands as strings
#define BEGIN()          "BEGIN"                             // Move to an extreme and reset rotation
#define MOVE1(pos)      ("MOVE1" + String(pos))              // Move towards motor by +v
#define MOVE2(pos)      ("MOVE2" + String(pos))   
#define ROTATE1(angle)  ("ROTATE1" + String(angle))          // Rotate by angle
#define ROTATE2(angle)  ("ROTATE2" + String(angle))   
#define INITIALX()       "INITIALX"                          // Reset Position
#define INITIALY()       "INITIALY"                          // Reset Rotation

// Moves the pole according to the given target & current X coordinates.
// Note that this function takes arguments in field coordinates (used by the algorithm).
void moveField(int poleId, int target_x, int* curr_x) {
  int diff = target_x - *curr_x;
  *curr_x += diff; // MIGHT NEED TO CLAMP // CHECK WHETHER IN RANGE
  if (poleId == 0){
    executeInterpreter(MOVE1(fieldXToMotorUnits * diff));
  }
  else if (poleId == 1){
    executeInterpreter(MOVE2(fieldXToMotorUnits * diff));
  }
  else{
    println("please enter valid poleId, 0 or 1");
  }
}

// Moves the pole back to the middle (initial X coordinate)
// and updates the current X coordinate
void centerPole(int poleId, int* curr_x) {
  executeInterpreter(INITIALX());                                 //Not sure how the interpreter was defined in this case 
  *curr_x = fieldWidth / 2;
}

//chose based on pole ID which pole to move
void rotate(int poleId, int angle){
  if (poleId == 0){
    executeInterpreter(ROTATE1(angle));
  }
  else if (poleId == 1){
    executeInterpreter(ROTATE2(angle));
  }
  else{
    println("please enter valid poleId, 0 or 1");
  }
}

void setup() {
  Serial.begin(9600);
  wemosSerial1.begin(9600);
  wemosSerial2.begin(9600);
  stepperY.setMaxSpeed(5000.0); // set max speed of the stepper , slower to get better accuracy
  stepperY.setAcceleration(5000.0); //set acceleration of the stepper 
  stepperX.setMaxSpeed(5000.0); // set max speed of the stepper , slower to get better accuracy
  stepperX.setAcceleration(5000.0); //set acceleration of the stepper 
  pinMode(EN, OUTPUT); 
  digitalWrite(EN, LOW); // Enable motor driver
  //stepperY.setCurrentPosition(0);  // initialize the current position im at to be 0 

  //sensors for the pole 1  single player
  pinMode(11, INPUT); // sensor far from the  sideway motor, responsible for the pos unit move control 
  pinMode(12, INPUT); // sensor close to the sideway motor, responsible for the pos unit move control  
  // sensor for the pole 2 double player
  pinMode(2, INPUT); // TODO not set up yet
  pinMode(13, INPUT);
}

void loop() {

  //poleID == 0 moves goalkeeper, poleID == 1 moves attack 
  int poleId = 0; // Change this to decide what motor to control
  delay(2000);

  // INITIAL STEPS

  // Set the Pole to the start position (in the middle of the field)
  executeInterpreter(BEGIN());  
  int curr_x_algo = fieldWidth / 2;
  delay(2000);
  // TEST ROTATION
  executeInterpreter(INITIALY()); // Should bring rotation
  delay(1000);

  // SIMPLE ROTATIONS
  rotate(poleId, 30); // Should move back (shooting position)
  delay(1000);
  rotate(poleId, -30); // Should get back to initial rotation
  delay(1000);
  rotate(poleId, -30); // Gets into position for receiving a pass
  delay(1000);
  executeInterpreter(INITIALY());
  delay(1000);

  // Do a full Revolution
  rotate(poleId, 360); // In one direction
  delay(2000);
  rotate(poleId, -360);  // In the other direction
  delay(2000);
 
  // Simulate a shot
  rotate(poleId, 30); // Should move back (shooting position)
  delay(1000);
  rotate(poleId, -60); // Should go from 30° to .-30°
  delay(1000);
  executeInterpreter(INITIALY());


  // LINEAR MOTION

  // Example of conversion from camera coordinates to field coordinates
  int target_x_algo = (scaleX * cameraWidth);

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