#ifndef _config_h_included
#define _config_h_included

#define MAX_CLOCK_NAME_LEN 16
#define MAX_CLOCK_CHANNEL_STRING_LEN 3
#define MAX_CLOCK_COLOR_LEN 16
#define DEFAULT_CLOCK_NAME "fastclk"
#define DEFAULT_CLOCK_CHANNEL_STRING "1"
#define DEFAULT_CLOCK_CHANNEL 1
#define DEFAULT_CLOCK_COLOR "green"
#define DEFAULT_BRIGHTNESS 31
//#define DEFAULT_COLOR SevenSegmentClock::Green
#define DEFAULT_COLOR "Green"

class Config {
public:
  Config() {
    strncpy(clockName, DEFAULT_CLOCK_NAME, MAX_CLOCK_NAME_LEN);
    strncpy(clockChannelString, DEFAULT_CLOCK_CHANNEL_STRING, MAX_CLOCK_CHANNEL_STRING_LEN);
    clockChannel = DEFAULT_CLOCK_CHANNEL;
    appMode = MODE_REALCLOCK;
    utcTimeOffsetMinutes = 120;
    brightness = DEFAULT_BRIGHTNESS;
    clockColorName = DEFAULT_COLOR;
  };
  enum AppMode { MODE_DEMO, MODE_REALCLOCK, MODE_FASTCLOCK };
private:
  char clockName[MAX_CLOCK_NAME_LEN+1];
  char clockChannelString[MAX_CLOCK_CHANNEL_STRING_LEN+1];
  uint8_t clockChannel;
  AppMode appMode;
  int utcTimeOffsetMinutes;
  uint8_t brightness;
  String clockColorName;
};

//SevenSegmentClock::Color clockColor = DEFAULT_COLOR;
//uint8_t brightness = DEFAULT_BRIGHTNESS;

#endif
