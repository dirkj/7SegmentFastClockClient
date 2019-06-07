#include "app.h"
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <NTPClient.h>        //https://github.com/esp8266/Arduino
#include <WiFiUdp.h>

//needed for library
#include <DNSServer.h>
//#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <ArduinoJson.h>
#include <SPI.h>
#include "DjDebug.h"
#include "DjConfig.h"
#include "SevenSegmentClock.h"
#include "DjDebug.h"
#include "ClockClient.h"
#include "WebUI.h"

// NTP
WiFiUDP ntpUDP;
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 60000);


static const char *appName = "FastclockClient7Seg";
const char * const PROGMEM mainConfig[] = {
  "appMode:string:" MODE_DEMO,
  "utcTimeOffsetMinutes:int:120",
  "listenPort:int:2000",
  "clockColor:string:green",
  "brightness:int:20"
};

Debug debug;
Config config(debug);
ClockClient fastclock(debug, config);
SevenSegmentClock sevenSegmentClock(debug, config);
ClockClient fastclockClient(debug, config);
//ESP8266WebServer *server;
WebUI *webUI;

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  debug.outln("Should save config", DEBUG_MAX_INFO);
  shouldSaveConfig = true;
}


void setupWifiConnection() {
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(300);

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality(15);

  debug.outln("Starting autoConnect ...", DEBUG_MAX_INFO);
  //if (!wifiManager.autoConnect("FastclockClient7Seg", "password")) {
  //if (!wifiManager.autoConnect("fc7seg", "password")) {
  if (!wifiManager.autoConnect("fc7seg")) {
    debug.outln("failed to connect and hit timeout", DEBUG_WARNING);
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  debug.outln("connected...yeey :)", DEBUG_MAX_INFO);

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    debug.outln("saving config ... NYI", DEBUG_MED_INFO);
  }

  debug.out("local ip: ", DEBUG_MAX_INFO); debug.outln(WiFi.localIP(), DEBUG_MAX_INFO);
  debug.out("gateway: ", DEBUG_MAX_INFO); debug.outln(WiFi.gatewayIP(), DEBUG_MAX_INFO);
  debug.out("subnet: ", DEBUG_MAX_INFO); debug.outln(WiFi.subnetMask(), DEBUG_MAX_INFO);
}



void setup() {
  debug.out(F("Starting *** "), DEBUG_MAX_INFO); debug.outln(appName, DEBUG_MAX_INFO);
  debug.out(F("Reset reason: "), DEBUG_MIN_INFO);
  debug.outln(ESP.getResetReason(), DEBUG_MIN_INFO);

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  debug.outln(F("mounting FS..."), DEBUG_MAX_INFO);

  if (SPIFFS.begin()) {
    debug.outln(F("mounted file system"), DEBUG_MAX_INFO);
    config.begin("main.cfg", mainConfig, sizeof(mainConfig)/sizeof(mainConfig[0]));
  } else {
    debug.outln(F("failed to mount FS"), DEBUG_ERROR);
    config.begin("main.cfg", mainConfig, sizeof(mainConfig)/sizeof(mainConfig[0]));
  }
  setupWifiConnection();
  debug.outln(F("Starting NTP Client"), DEBUG_MAX_INFO);
  timeClient.begin();

  debug.outln(F("Have following configuration:"), DEBUG_MAX_INFO);
  debug.out(F("   App Mode: "), DEBUG_MAX_INFO); debug.outln(config.getString("appMode"), DEBUG_MAX_INFO);
  debug.out(F("   Clock color: "), DEBUG_MAX_INFO); debug.outln(config.getString("clockColor"), DEBUG_MAX_INFO);
  debug.out(F("   Brightness: "), DEBUG_MAX_INFO); debug.outln(config.getString("brightness"), DEBUG_MAX_INFO);
  debug.out(F("   Clock listen port: "), DEBUG_MAX_INFO); debug.outln(config.getString("listenPort"), DEBUG_MAX_INFO);
  debug.out(F("   Real time UTC offset: "), DEBUG_MAX_INFO); debug.outln(config.getString("utcTimeOffsetMinutes"), DEBUG_MAX_INFO);

  timeClient.setTimeOffset(config.getInt("utcTimeOffsetMinutes") * 60);
  debug.outln(F("Starting fastclock ..."), DEBUG_MAX_INFO);
  fastclock.begin();

  debug.outln(F("Starting 7-segment clock display ..."), DEBUG_MAX_INFO);
  sevenSegmentClock.begin();
  sevenSegmentClock.setBrightness(config.getInt("brightness"));
  sevenSegmentClock.setColor(sevenSegmentClock.getColorHandleByName(config.getString("clockColor")));

  // setting up web server for clock configuration
  //server = new ESP8266WebServer(80);
  webUI = new WebUI(debug, config, timeClient, fastclock, sevenSegmentClock);
  webUI->begin();
}

int hours = 0, minutes = 0;
uint32_t nextUpdate_ms = 0;

void loop() {
  timeClient.update();
  fastclock.loop();

  if (config.getString("appMode").equals(MODE_DEMO)) {
    if (millis() > nextUpdate_ms) {
      nextUpdate_ms = millis() + 1000;
      minutes++;
      if (minutes > 99) { minutes = 0; }
      if (minutes % 5 == 0) hours++;
      if (hours > 99) hours = 0;
      sevenSegmentClock.displayTime(hours, minutes);
      if (hours % 4 == 0) sevenSegmentClock.setBlinkMode(SevenSegmentClock::SeperatorBlinking); else sevenSegmentClock.setBlinkMode(SevenSegmentClock::NoBlinking);
    }
  } else if (config.getString("appMode").equals(MODE_REALCLOCK)) {
    sevenSegmentClock.setClockHalted(!fastclock.isActive());
    sevenSegmentClock.setBlinkMode(SevenSegmentClock::NoBlinking);
    sevenSegmentClock.displayTime(timeClient.getHours(), timeClient.getMinutes());
  } else if (config.getString("appMode").equals(MODE_FASTCLOCK)) {
    sevenSegmentClock.setClockHalted(!fastclock.isActive());
    if (fastclock.isActive()) {
      sevenSegmentClock.setBlinkMode(SevenSegmentClock::NoBlinking);
    } else {
      sevenSegmentClock.setBlinkMode(SevenSegmentClock::DecimalPointColoredBlinking);
    }
    sevenSegmentClock.displayTime(fastclock.getClockHours(), fastclock.getClockMinutes());
  } else { debug.outln(F("ERROR: Unknown appMode found."), DEBUG_ERROR); }

  sevenSegmentClock.displayUpdate();
  webUI->loop();
}
