#include "state.h"

String stateToString(spaceState_t s) {
  switch (s) {
  case spaceOpen:
    return spaceOpenStr;
    break;
  case spaceClosed:
    return spaceClosedStr;
    break;
  case spaceMembersOnly:
    return spaceMembersOnlyStr;
    break;
  default:
    return spaceUndefinedStr;
  }
}
