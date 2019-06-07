#ifndef sevenSegmentClock_h_included
#define sevenSegmentClock_h_included

#include <Adafruit_NeoPixel.h>
#include "DjDebug.h"
#include "DjConfig.h"

// avoid flickering of the display:
#define TIME_BETWEEN_DISPLAY_UPDATES_ms 100
#define BLINK_OFF_TIME_ms 400
#define BLINK_ON_TIME_ms 400
#define defaultLedDataPin 2
class SevenSegmentClock {
public:
  SevenSegmentClock(Debug& _debug, Config& _config):debug(_debug), config(_config) { LedDataPin=defaultLedDataPin; };
  SevenSegmentClock(Debug& _debug, Config& _config, uint8_t dataPin):debug(_debug), config(_config) { LedDataPin=dataPin; };
  void begin(void);
  void displayTime(int hour, int minute);
  void displayUpdate(void);
  //void setClockSpeed(int _msPerModelSecond) { msPerModelSecond = _msPerModelSecond; setClockSpeed("x"); };
  enum BlinkMode { NoBlinking, ClockBlinking, SeperatorBlinking, DecimalPointBlinking, DecimalPointColoredBlinking };
  void setBlinkMode(BlinkMode _blinkMode) { blinkMode = _blinkMode; };
  void setClockHalted(bool halted) { clockHalted = halted; };
  enum Color { Black, Red, Green, Blue, White, Yellow, Magenta, Cyan };
  void setColor(Color color);
  void setBlinkColor(Color color);
  Color getColor(void) { return currentColorHandle; };
  void displayDigit(unsigned int digitNum, char c);
  void displaySeperator(char seperatorCharacter);
  void displaySeperator(char seperatorCharacter, uint32_t color);
  void displaySeperator(uint32_t color);
  void displayDecimalPoint(uint32_t color);
  void setBrightness(uint8_t b) { brightness=b; };
  uint8_t getBrightness(void) { return brightness; };
  int getNumberSupportedColors(void) { return numberOfSupportedColors; };
  String getColorName(int index) { return String(colorSelection[index].colorName); };
  String getColorName(Color handle);
  Color getColorHandle(int index);
  uint32_t getAdjustedStripColor(uint8_t red, uint8_t green, uint8_t blue);
  uint32 getColorByName(String name);
  uint32 getColorByHandle(Color handle);
  Color getColorHandleByName(String name);
private:
  Debug& debug;
  Config& config;
  static uint8_t LedDataPin;
  static Adafruit_NeoPixel *strip;
  static BlinkMode blinkMode;
  static uint8_t brightness;
  int clockHour;
  int clockMinute;
  bool clockHalted;
  Color currentColorHandle;
  Color blinkColorHandle;
  uint32_t currentColor;
  uint32_t blinkColor;
  uint32_t black;
  void displaySegment(unsigned int ledAddress, uint32_t color);
  static struct ColorSelection {
    Color handle;
    String colorName;
    uint8_t red, green, blue;
  } colorSelection[];
  static int numberOfSupportedColors;
};
#endif
