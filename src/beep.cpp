#include "beep.h"

void playClose(uint8_t pin) {
  tone(pin, 1000, beepLen);
  noTone(pin);
  delay(timeBetweenBeeps);
  tone(pin, 800, beepLen);
  noTone(pin);
  delay(timeBetweenBeeps);
  tone(pin, 500, beepLen);
  noTone(pin);
  delay(timeBetweenBeeps);
}
void playOpen(uint8_t pin) {
  tone(pin, 500, beepLen);
  noTone(pin);
  delay(timeBetweenBeeps);
  tone(pin, 800, beepLen);
  noTone(pin);
  delay(timeBetweenBeeps);
  tone(pin, 1000, beepLen);
  noTone(pin);
  delay(timeBetweenBeeps);
}
void playMember(uint8_t pin) {
  tone(pin, 500, beepLen);
  noTone(pin);
  delay(timeBetweenBeeps);
  tone(pin, 1000, beepLen);
  noTone(pin);
  delay(timeBetweenBeeps);
  tone(pin, 500, beepLen);
  noTone(pin);
  delay(timeBetweenBeeps);
}
