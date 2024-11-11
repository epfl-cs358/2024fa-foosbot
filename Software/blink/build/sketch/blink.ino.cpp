#include <Arduino.h>
#line 1 "/home/tough/Documents/EPFL/BA5/MIT/Group Project/2024fa-foosbot/Software/blink/blink.ino"
int flashPin = 4;

void setup()
{
    pinMode(flashPin, OUTPUT);
}

void loop()
{
    digitalWrite(flashPin, HIGH);
    delay(1000);
    digitalWrite(flashPin, LOW);
    delay(1000);
}

