#ifndef sevenSegmentClock_h_included
#define sevenSegmentClock_h_included

#include <Adafruit_NeoPixel.h>
#include "DjDebug.h"
#include "DjConfig.h"

// avoid flickering of the display:
#define TIME_BETWEEN_DISPLAY_UPDATES_ms 100
#define BLINK_OFF_TIME_ms 200
#define BLINK_ON_TIME_ms 200
#define defaultLedDataPin 2
class SevenSegmentClock {
public:
  SevenSegmentClock(Debug& _debug, Config& _config):debug(_debug), config(_config) { LedDataPin=defaultLedDataPin; init(); };
  SevenSegmentClock(Debug& _debug, Config& _config, uint8_t dataPin):debug(_debug), config(_config) { LedDataPin=dataPin; init(); };
  void begin(void);
  void displayTime(int hour, int minute);
  void displayUpdate(void);
  //void setClockSpeed(int _msPerModelSecond) { msPerModelSecond = _msPerModelSecond; setClockSpeed("x"); };
  enum BlinkMode { NoBlinking, ClockBlinking, SeperatorBlinking, DecimalPointBlinking };
  void setBlinkMode(BlinkMode _blinkMode) { blinkMode = _blinkMode; };
  void setClockHalted(bool halted) { clockHalted = halted; };
  enum Color { Black, Red, Green, Blue, White, Yellow };
  void setColor(Color color);
  Color getColor(void) { return currentColorHandle; };
  enum ClockDisplayStatus { Off, Booting, Halted, StandardClock, FastClock };
  void displayDigit(unsigned int digitNum, char c);
  void displaySeperator(char seperatorCharacter);
  void setBrightness(uint8_t b) { brightness=b; initColors(b); };
  uint8_t getBrightness(void) { return brightness; };
  int getNumberSupportedColors(void) { return numberOfSupportedColors; };
  String getColorName(int index) { return String(colorSelection[index].colorName); };
  String getColorName(Color handle) {
    for (int i=0; i<numberOfSupportedColors; ++i) {
      if (colorSelection[i].handle == handle)
        return colorSelection[i].colorName;
    }
    debug.outln(F("ERROR: Unknown color / handle not known"), DEBUG_ERROR);
    return String(F("ERROR: Unknown color handle"));
  };
  Color getColorHandle(int index) { return colorSelection[index].handle; };
  uint32_t getAdjustedStripColor(uint8_t red, uint8_t green, uint8_t blue) {
    return strip->Color((red * brightness) / 255, (green * brightness) / 255, (blue * brightness) / 255);
  }
  uint32 getColorByName(String name) {
    for (int i=0; i<numberOfSupportedColors; ++i) {
      if (colorSelection[i].colorName.equals(name)) {
        return getAdjustedStripColor(colorSelection[i].red, colorSelection[i].green, colorSelection[i].blue);
      }
    }
    debug.out(F("ERROR: Unknown color name "), DEBUG_ERROR);
    debug.outln(name, DEBUG_ERROR);
    return 0xffffffff;
  };
  uint32 getColorByHandle(Color handle) {
    for (int i=0; i<numberOfSupportedColors; ++i) {
      if (colorSelection[i].handle == handle) {
        return getAdjustedStripColor(colorSelection[i].red, colorSelection[i].green, colorSelection[i].blue);
      }
    }
    debug.outln(F("ERROR: Unknown color handle"), DEBUG_ERROR);
    return 0xffffffff;
  };
  Color getColorHandleByName(String name) {
    for (int i=0; i<numberOfSupportedColors; ++i) {
      if (colorSelection[i].colorName.equals(name)) {
        return colorSelection[i].handle;
      }
    }
    debug.out(F("ERROR: Unknown color name "), DEBUG_ERROR);
    debug.outln(name, DEBUG_ERROR);
    return Green; // default
  };
private:
  Debug& debug;
  Config& config;
  void init(void) { displayStatus = Off; clockHour=12; clockMinute=34; setClockHalted(true); currentColorHandle = Green; currentColor = green; };
  static uint8_t LedDataPin;
  static Adafruit_NeoPixel *strip;
  static BlinkMode blinkMode;
  static uint8_t brightness;
  static uint32_t red, green, blue, white, black, yellow;
  ClockDisplayStatus displayStatus;
  int clockHour;
  int clockMinute;
  bool clockHalted;
  Color currentColorHandle;
  uint32_t currentColor;
  void displaySegment(unsigned int ledAddress, uint32_t color);
  void initColors(uint8_t _brightness);
  static struct ColorSelection {
    Color handle;
    String colorName;
    uint8_t red, green, blue;
  } colorSelection[];
  static int numberOfSupportedColors;
};
#endif
