#include "attack.h"

Attack::Attack(CustomStepperControl c, SoftwareSerial sp)
  : control(c)
  , serialPort(sp) {}

void Attack::shoot() {
  control.rotateByAngle(serialPort, -45);
  control.rotateByAngle(serialPort,  90);
  control.rotateByAngle(serialPort,  45);
}
