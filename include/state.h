#pragma once
#include <Arduino.h>

const char spaceOpenStr[] = "open";
const char spaceClosedStr[] = "closed";
const char spaceMembersOnlyStr[] = "membersOnly";
const char spaceUndefinedStr[] = "undefined";

enum spaceState_t {
  spaceOpen = 0,
  spaceMembersOnly = 1,
  spaceClosed = 2,
  spaceUndefined = -1,
};

String stateToString(spaceState_t s);
