#pragma once
#include <Arduino.h>

const uint8_t timeBetweenBeeps = 100;
const uint8_t beepLen = 100;
const uint8_t f1 = 500;
const uint8_t f2 = 800;
const uint8_t f3 = 1000;

void playClose();
void playOpen();
void playMember();
