#ifndef sevenSegmentClock_h_included
#define sevenSegmentClock_h_included

#include <Adafruit_NeoPixel.h>

// avoid flickering of the display:
#define TIME_BETWEEN_DISPLAY_UPDATES_ms 300
#define BLINK_ON_OFF_TIME_ms 1000
#define defaultLedDataPin 2
class SevenSegmentClock {
public:
  SevenSegmentClock() { LedDataPin=defaultLedDataPin; init(); };
  SevenSegmentClock(uint8_t dataPin) { LedDataPin=dataPin; init(); };
  void begin(void);
  void displayTime(int hour, int minute);
  //void setClockSpeed(int _msPerModelSecond) { msPerModelSecond = _msPerModelSecond; setClockSpeed("x"); };
  void setClockHalted(bool halted) { clockHalted = halted; };
  static uint32_t red, green, blue, white, black;
  enum ClockDisplayStatus { Off, Booting, Halted, StandardClock, FastClock };
  void displayDigit(unsigned int digitNum, char c);
  void displaySeperator(char seperatorCharacter);
private:
  void init(void) { displayStatus = Off; clockHour=12; clockMinute=34; setClockHalted(true); };
  static uint8_t LedDataPin;
  static Adafruit_NeoPixel *strip;
  ClockDisplayStatus displayStatus;
  int clockHour;
  int clockMinute;
  bool clockHalted;
  uint32_t currentColor;
  void displaySegment(unsigned int ledAddress, uint32_t color);
};
#endif
