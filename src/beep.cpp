#include "beep.h"

void playClose() {
  tone(GPIO_NUM_2, 1000, beepLen);
  noTone(GPIO_NUM_2);
  delay(timeBetweenBeeps);
  tone(GPIO_NUM_2, 800, beepLen);
  noTone(GPIO_NUM_2);
  delay(timeBetweenBeeps);
  tone(GPIO_NUM_2, 500, beepLen);
  noTone(GPIO_NUM_2);
  delay(timeBetweenBeeps);
}
void playOpen() {
  tone(GPIO_NUM_2, 500, beepLen);
  noTone(GPIO_NUM_2);
  delay(timeBetweenBeeps);
  tone(GPIO_NUM_2, 800, beepLen);
  noTone(GPIO_NUM_2);
  delay(timeBetweenBeeps);
  tone(GPIO_NUM_2, 1000, beepLen);
  noTone(GPIO_NUM_2);
  delay(timeBetweenBeeps);
}
void playMember() {
  tone(GPIO_NUM_2, 500, beepLen);
  noTone(GPIO_NUM_2);
  delay(timeBetweenBeeps);
  tone(GPIO_NUM_2, 1000, beepLen);
  noTone(GPIO_NUM_2);
  delay(timeBetweenBeeps);
  tone(GPIO_NUM_2, 500, beepLen);
  noTone(GPIO_NUM_2);
  delay(timeBetweenBeeps);
}
