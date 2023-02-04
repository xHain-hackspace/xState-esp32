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
  digitalWrite(pin, LOW);
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
  digitalWrite(pin, LOW);
}
void playMember(uint8_t pin) {
  tone(pin, 500, beepLen * 2);
  noTone(pin);
  delay(timeBetweenBeeps * 2);
  tone(pin, 1000, beepLen);
  noTone(pin);
  delay(timeBetweenBeeps);
  tone(pin, 500, beepLen * 2);
  noTone(pin);
  digitalWrite(pin, LOW);
}
