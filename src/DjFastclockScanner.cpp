//
//    FILE: DjFastclockScanner.h
// VERSION: 1.0
// PURPOSE: Scans the broadcasts for clocks and returns the clocknames found
//
//

#include <DjFastclockScanner.h>


String FastclockScanner::knownClocks[MAX_CLOCKS];
int FastclockScanner::numberOfKnownClocks = 0;

void FastclockScanner::addClock(String clockName) {
  for (int i=0; i<numberOfKnownClocks; ++i) {
    if (clockName.equals(knownClocks[i])) return;
  }
  if (numberOfKnownClocks < MAX_CLOCKS) {
    knownClocks[numberOfKnownClocks] = String(clockName);
    ++numberOfKnownClocks;
    debug.out(F("Added new clock with name=")); debug.outln(clockName);
  }
  logHeap();
}

void FastclockScanner::addClock(const char * clockName) {
  addClock(String(clockName));
}



