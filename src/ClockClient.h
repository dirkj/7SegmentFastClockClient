//
//    FILE: ClockClient.h
// VERSION: 0.1
// PURPOSE: FREMO Clock Client
//
//

#ifndef _clockClientLoaded
#define _clockClientLoaded

#include <Arduino.h>
#include "DjDebug.h"
#include "DjFastclockScanner.h"
#include "DjConfig.h"
//#include <Ticker.h>
#include <WiFiUdp.h>

#define MAX_CLOCK_CHANGE_CALLBACKS 5

typedef void (*ClockChangeCallback)(int h, int m, int s);

class ClockClient
{
  public:
    ClockClient(Debug& _debug, Config& _config):debug(_debug), config(_config), fastclockScanner(_debug) {};
    void begin();
    void loop();
    static void setListenToClock(const char *_name) { name = String(_name); }
    static void setListenToClock(String _name) { name = String(_name); }
    static const char * const getLastMessage();
    static String const getText() { return text; }
    static String const getClock() { return clock; }
    static String const getName() { return name; }
    static boolean const isActive() { return active; }
    static float const getSpeed() { return speed; }
    static int const getClockHours() { return clockHours; }
    static int const getClockMinutes() { return clockMinutes; }
    static int const getClockSeconds() { return clockSeconds; }
    static void addClockChangeCallback(ClockChangeCallback callback);
    int getNumberOfKnownClocks() { return fastclockScanner.getNumberOfKnownClocks(); };
    String *getKnownClocks() { return fastclockScanner.getKnownClocks(); };
    int getListenPort();
    IPAddress getMulticastIP();
    static String const getClockString() {
      String output = String(clockHours) + ":" + String(clockMinutes) + ":" + String(clockSeconds);
      return output;
    }


  private:
    Debug& debug;
    Config& config;
    FastclockScanner fastclockScanner;
    //Ticker clockTrigger;
    static int numClockChangeCallbacks;
    static ClockChangeCallback clockChangeCallback[MAX_CLOCK_CHANGE_CALLBACKS];
    static String name;
    static String text;
    static String clocktype;
    static boolean active;
    static float speed;
    static int interval;
    static String clock;
    static int clockHours;
    static int clockMinutes;
    static int clockSeconds;
    static String weekday;
    void interpretClockMessage(const char *msg);
    void interpretClockMessageLine(const char *msg);
    //void interpretClockMessageLine(String msg);
    void addClock(const char * clockName);
    void addClock(String clockName);
    void outLine(uint8_t *buff, int len);
    void dumpReceivedMessage(String msg);
    // temporary message storage:
    static String checkName, _text, _clocktype, _clock, _weekday;
    static float _speed;
    static boolean _active;
    static int _clockHours, _clockMinutes, _clockSeconds, _interval;

};

#endif
