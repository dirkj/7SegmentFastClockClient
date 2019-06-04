#ifndef sevenSegmentClock_h_included
#define sevenSegmentClock_h_included

#include <Adafruit_NeoPixel.h>

// avoid flickering of the display:
#define TIME_BETWEEN_DISPLAY_UPDATES_ms 100
#define BLINK_OFF_TIME_ms 200
#define BLINK_ON_TIME_ms 200
#define defaultLedDataPin 2
class SevenSegmentClock {
public:
  SevenSegmentClock() { LedDataPin=defaultLedDataPin; init(); };
  SevenSegmentClock(uint8_t dataPin) { LedDataPin=dataPin; init(); };
  void begin(void);
  void displayTime(int hour, int minute);
  void displayUpdate(void);
  //void setClockSpeed(int _msPerModelSecond) { msPerModelSecond = _msPerModelSecond; setClockSpeed("x"); };
  enum BlinkMode { NoBlinking, ClockBlinking, SeperatorBlinking, DecimalPointBlinking };
  void setBlinkMode(BlinkMode _blinkMode) { blinkMode = _blinkMode; };
  void setClockHalted(bool halted) { clockHalted = halted; };
  enum Color { Black, Red, Green, Blue, White };
  void setColor(Color color);
  Color getColor(void) { return currentColorHandle; };
  static uint32_t red, green, blue, white, black;
  enum ClockDisplayStatus { Off, Booting, Halted, StandardClock, FastClock };
  void displayDigit(unsigned int digitNum, char c);
  void displaySeperator(char seperatorCharacter);
  void setBrightness(uint8_t b) { brightness=b; initColors(b); };
  uint8_t getBrightness(void) { return brightness; };
private:
  void init(void) { displayStatus = Off; clockHour=12; clockMinute=34; setClockHalted(true); currentColorHandle = Blue; currentColor = blue; };
  static uint8_t LedDataPin;
  static Adafruit_NeoPixel *strip;
  static BlinkMode blinkMode;
  static uint8_t brightness;
  ClockDisplayStatus displayStatus;
  int clockHour;
  int clockMinute;
  bool clockHalted;
  Color currentColorHandle;
  uint32_t currentColor;
  void displaySegment(unsigned int ledAddress, uint32_t color);
  void initColors(uint8_t _brightness);
};
#endif
