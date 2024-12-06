// Inspired by https://gist.github.com/edgar-bonet/607b387260388be77e96
#include <stdio.h>
#include <math.h>

//constants
#define controlSpeedThreshold 10
#define controlPositionThresholdX 15
#define controlPositionThresholdY 15
#define rod0_Y 81.5                   //position of the goalkeeper rod in coordinates of game logic
#define rod1_Y 232                            //position of the attack rod in coordinates of game logic
#define speedThreshold 25
#define fieldWidth   680                      // Width in coordinate system of game logic
#define fieldHeight  605                      // Height in coordinate system of game logic
#define cameraWidth  640                      // Width of received Frame
#define cameraHeight 605                      // Height of received Frame
#define scaleX = fieldWidth/ cameraWidth;     //ratio of cv coordinates into the field dimension
#define scaleY = fieldHeight/ cameraHeight;
#define motorUnits 350                        // Unit of interpreter coordinates
#define physicalRangeMM 70                    // physical distance in mm corresponding to motor units
#define motorUnitsPerMM motorUnits / physicalRangeMM
#define angleUnits 12                         // Value that interpreter expects for 180°
#define angleStep 180                         // 180°
#define unitsPerDeg double(angleUnits) / double(angleStep)

#define angleToUnits(angle) round(angle * unitsPerDeg)

#define BEGIN()       // Move to an extreme and reset rotation
#define MOVE(v)       // Move towards motor by +v
#define ROTATE(angle) // Rotate by angle
#define INITIALX()    // Reset Rotation
#define INITIALY()    // Reset Position


void setup() {
  Serial.begin(9600);
}

void loop() {

  delay(2000);
  Serial.println(motorUnitsPerMM);
  // INITIAL STEPS

  // Set both Poles to the start position
  BEGIN(); // Pole 1
  //BEGIN();
  delay(2000);
  // TEST ROTATION
  INITIALY(); // Test whether it is zero
  delay(1000);

  // SIMPLE ROTATIONS
  ROTATE(angleToUnits(-30)); // Should move back (shooting position)
  delay(1000);
  ROTATE(angleToUnits(30)); // Should get back to initial rotation
  delay(1000);
  ROTATE(angleToUnits(30)); // Gets into position for receiving a pass
  delay(1000);
  INITIALY();
  delay(1000);

  // Do a full Revolution
  ROTATE(angleToUnits(360)); // In one direction
  delay(2000);
  ROTATE(angleToUnits(-360)); // In the other direction
  delay(2000);

  // Simulate a shot
  ROTATE(angleToUnits(-30)); // Should move back (shooting position)
  delay(1000);
  ROTATE(angleToUnits(60)); // Should go from -30° to +30°

  /*MOVE()
  ROTATE()
  INITIALY()
  INITIALX()
  BEGIN()*/
}