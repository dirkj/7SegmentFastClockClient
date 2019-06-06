/* DjFastclockScanner -- collect fastclock server names
 */

#ifndef _DjFastclockScannerIncluded
#define _DjFastclockScannerIncluded true

#include <Arduino.h>
#include "DjDebug.h"

#define MAX_CLOCKS 10

class FastclockScanner {
	public:
		FastclockScanner(Debug& _debug):debug(_debug) {};
		void addClock(String clockName);
		void addClock(const char * clockName);
		String *getKnownClocks() { return knownClocks; }
		int getNumberOfKnownClocks() { return numberOfKnownClocks; }
	private:
		Debug& debug;
		static String knownClocks[];
		static int numberOfKnownClocks;
};

#endif
