#ifndef ATTACK_H
#define ATTACK_H

#include "CustomStepperControl.h"
#include <SoftwareSerial.h>


class Attack
{
public:
  /*
   * @param c  Initialised CustomStepperControl instance to be used
   * @param sp Serial port to be used
   */
  Attack (CustomStepperControl c, SoftwareSerial sp);
  /*
   * Shoots straight.
   */
  void shoot();

private:
  CustomStepperControl control;
  SoftwareSerial       serialPort;
};

#endif /* ifndef ATTACK_H */
