#pragma once
#include <Arduino.h>

const uint16_t timeBetweenBeeps = 100;
const uint16_t beepLen = 100;
const uint16_t f1 = 500;
const uint16_t f2 = 800;
const uint16_t f3 = 1000;

void playClose(uint8_t pin);
void playOpen(uint8_t pin);
void playMember(uint8_t pin);
