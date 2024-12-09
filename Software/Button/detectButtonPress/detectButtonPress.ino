const int buttonPin = 2;
const int outPin = 13;
const int holdFor = 1000; // How long should the signal be set to high in Milliseconds

int buttonState = 0;

void setup() {
  // Define Input and Output Pins
  pinMode(outPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // Attach Interrupt routine to the pin that is connected to the button
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressed, RISING);
}
void loop() {
  if (buttonState) {
    digitalWrite(outPin, HIGH);
    delay(holdFor);
    digitalWrite(outPin, LOW);
    buttonState = 0;
  }
}

// Interrupt Routine
void buttonPressed() {
  buttonState = 1;
}