#define EN 8

#define Y_DIR 6

#define Y_STP 3

void step(boolean dir, byte dirPin, byte stepperPin, int steps)
{
  digitalWrite(dirPin, dir);
  delay(100);
  for (int i = 0; i < steps; i++)
  {
    digitalWrite(stepperPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepperPin, LOW);
    delayMicroseconds(500);
  }
}

void setup()
{
  pinMode(Y_DIR, OUTPUT);
  pinMode(Y_STP, OUTPUT);
  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);
}

void loop()
{
  step(false, Y_DIR, Y_STP, 6400);
  delay(1000);
  step(true, Y_DIR, Y_STP, 6400);
  delay(1000);
}
