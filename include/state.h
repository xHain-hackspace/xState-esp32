#pragma once
#include <Arduino.h>

const char spaceOpenStr[] = "open";
const char spaceClosedStr[] = "closed";
const char spaceMembersOnlyStr[] = "membersOnly";
const char spaceUndefinedStr[] = "undefined";

typedef enum {
  spaceUndefined,
  spaceOpen,
  spaceClosed,
  spaceMembersOnly
} spaceState_t;

String stateToString(spaceState_t s);
