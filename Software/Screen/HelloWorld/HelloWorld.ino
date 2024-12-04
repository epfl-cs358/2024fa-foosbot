// Original Code from : https://www.instructables.com/Arduino-Interfacing-With-LCD-Without-Potentiometer/
#include <LiquidCrystal.h>

const int contrastPin = 10;

const int contrast = 70;

LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

void setup() {
  //Initialize LCD Screen
  analogWrite(contrastPin, contrast);
  lcd.begin(16, 2);
}
void loop() {
  // Display Hello on the first line
  lcd.setCursor(0, 0);
  lcd.print("Hello");

  // Display World ! on the second line
  lcd.setCursor(0, 1);
  lcd.print("World !");
}