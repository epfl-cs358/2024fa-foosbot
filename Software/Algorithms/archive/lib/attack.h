#ifndef ATTACK_H
#define ATTACK_H

#include "CustomStepperControl.h"

class Attack
{
public:
  /*
   * @param c Initialised CustomStepperControl instance to be used
   */
  Attack (CustomStepperControl c);
  /*
   * Shoots straight.
   */
  void shoot();

private:
  CustomStepperControl control;
  SoftwareSerial       serialPort;
};

#endif /* ifndef ATTACK_H */
