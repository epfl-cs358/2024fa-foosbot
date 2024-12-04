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


void setup() {

  Serial.begin(9600); //create communication with cv 

}

void loop() {

    // Retrieve ball data from computer vision system
    if(!getBallData()){
      return;
    }
}
