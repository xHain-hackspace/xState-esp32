#include "util.h"
#include "Arduino.h"

void blinkLED(uint8_t pin, uint16_t d) {
  digitalWrite(pin, HIGH);
  delay(d);
  digitalWrite(pin, LOW);
  delay(d);
}

void displayErrorLoop(uint8_t pin) {
  while (true) {
    blinkLED(pin, 200);
  }
}
