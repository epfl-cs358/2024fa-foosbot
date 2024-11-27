#include <stdio.h>
#include <math.h>

#define rode0_y 0 // 1 player
#define rode1_y 0 // 3 players
#define mm_to_step_ratio 1
#define fps 25
#define graphDist_to_mmDist 0
#define speedTreshold 10// mm/s
#define minGoal 300 //mm
#define maxGoal 480 //mm
#define offsetGoalie 15 //mm
#define staticTreshold 1 //mm/s
#define reachingRange 20 //mm
#define fieldLength 605 //Field length in mm
#define zoneDefense1 202 //range in mm 
#define zoneDefense2 404 //mm
#define zoneDefense3 605 //mm


typedef struct {
    double x;       // x-coordinate of posB
    double y;       // y-coordinate of posB
    double a;       // Slope of the line
    double b;       // Intercept of the line
    double speed;   // Calculated speed
} Infos;

Infos ballData;

//double translationGradian[0]
//double rotationGradian[1]
//double translationAttack[2]
//double rotationAttack[3]
double motorMovement[4]


//double player0 goalKeeper (x0, theta0)
//double player1 attack (x1, theta1)
//double player2 attack (x2, theta2)
//double player3 attack (x3, theta3)
double playerPosition[4][2]

void traitementInput(double posA[2], double posB[2],Infos ballData){
  ballData.x = posB[0];
  ballData.y = posB[1];
  ballData.a = (posB[1] - posA[1]) / (posB[0] - posA[0]);
  ballData.b = posA[1] - result[2]* posA[0];
  // distance calculation
  double dist = sqrt(pow(posB[0] - posA[0], 2) + pow(posB[1] - posA[1], 2)) * graphDist_to_mmDist;
  // speed calculation (distance / time)
  ballData.speed = dist / fps;
}

//funcion that defines the defensive zones
int getDefensivePlayer(Infos ballData){
  int NewBallX = (rode1_y - ballData.b)/ ballData.a
  if (NewBallX <= ZONE_DEFENSE_P1) return 1; // Player 1
  else if (NewBallX <= ZONE_DEFENSE_P2) return 2; // Player 2
  else return 3; // Player 3
}

void updatePlayersRodX(double p){
  for (int i = 1; i < 4; i++){      
      playerPosition[i][0] += x;
  }
}

void updatePlayersRodAngle(double t){
  for (int i = 1; i < 4; i++){
      playerPosition[i][1] += t;
      
  }
}

//Update the players positions accordingly 
void takeDefensePosition(Infos ballData){
  int defender = getDefensivePlayer(ballData)
  int defenderX = playerPosition[defender][0]
  int NewBallX = (rode1_y - ballData.b)/ ballData.a
  int rodTranslation =  NewBallX - defenderX ;


  //adjust the right angle of the player and translate to interecept the ball 
  if (math.abs(ballData.speed) <= speedTreshold){
    if (ballData.speed >= 0){
      motorMovement[3] = 30; //unité des angles pour Nur?? + en avant qu'en +
      updatePlayersRodAngle(30);
    } else {
      motorMovement[3] = -30;
      updatePlayersRodAngle(-30);
    }
  }
  else{
    motorMovement[3] = 30; //unité des angles pour Nur??
    updatePlayersRodAngle(30);
  }
  motorMovement[2] = rodTranslation;      //move rod to intercept the ball 
  updatePlayersRodX(rodTranslation);

  //offset the gardian in comparison to the attacker that defends 
  if(ballData.x < FIELD_LENGTH/2){
    motorMovement[0] = rodTranslation - offsetGoalie;
    playerPosition[0][0] = NewBallX - offsetGoalie;
  }
  else{
    motorMovement[0] = rodTranslation + offsetGoalie;
    playerPosition[0][0] = NewBallX + offsetGoalie;
  }
  }
}

// implementer plus tard le gardian qui rotate pour chopper la ball 
void hitBallInFront(){
  motorMovement[3] += 360;
}
void hitBallInBack(){
  motorMovement[3] += 60;
  motorMovement[3] -= 60;
}

//par du principe que nouvelle capture avec ball statique 
voidAttack(Infos ballData){
  double alignementTranslation = playerPosition[2][0] - playerPosition[0][0] //translate of pos middle - pos goal
  double receivePass = 30 
  double crossfireOffset = 15;

  //très preu probable cette condition comme pas de controle gardian en ce moment
  if ((ballData.speed < staticTreshold) && (ballData.y == rode0_y)){
    //passe au joueur en face
    motorMovement[2] = alignementTranslation;
    updatePlayersRodX(alignementTranslation);

    motorMovement[3] = receivePass;
    updatePlayersRodAngle(receivePass);

    while (ballData.y < rode0_y + treshold){
    motorMovement[1] = 5;
    playerPosition[0][1]+= 5;
    }
  }
  if ((ballData.speed == 0) && (ballData.y == rode1_y)){
    if(ballData.x >= minGoal && ballData.x <= maxGoal){
      if (motorMovement[3] > 0){
        hitBallFront();  //controlle de nur comment ?
      }
      else{
        hitBallBack()
      }
    else if(ballData.x < minGoal){
      if (motorMovement[3] > 0){
        motorMovement[3] += 10;  //monter pour décaler 
        motorMovement[2] -= crossfireOffset;
        hitBallFront(); 
        motorMovement[3] -= 10; //retrouver angle initiale
      }
      else{
        motorMovement[3] -= 30;  //
        motorMovement[2] -= crossfireOffset;
        motorMovement[3] += 30; 
       }
    }
   else{
    if (motorMovement[3] > 0){
        motorMovement[3] += 10;  //monter pour décaler 
        motorMovement[2] += crossfireOffset;
        hitBallFront(); 
        motorMovement[3] -= 10; //retrouver angle initiale
      }
      else{
        motorMovement[3] -= 30;  //
        motorMovement[2] += crossfireOffset;
        motorMovement[3] += 30; 
       }
    }
      

    }
  }
}


void main() {




}


void loop() {
  // put your main code here, to run repeatedly:

}