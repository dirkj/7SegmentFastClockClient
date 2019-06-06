#ifndef _webui_included
#define _webui_included

#include "app.h"
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include "DjDebug.h"
#include "DjConfig.h"
#include "SevenSegmentClock.h"
#include "ClockClient.h"

class WebUI {
public:
  WebUI(Debug& _debug, Config& _config, NTPClient& _timeClient, ClockClient& _fastclock, SevenSegmentClock& _sevenSegmentClock):
    debug(_debug), config(_config), timeClient(_timeClient), fastclock(_fastclock), sevenSegmentClock(_sevenSegmentClock) {
    debug.outln(F("WebUI constructor called"));
  };
  void begin(void) {
    server = new ESP8266WebServer(80);
    server->on("/config", std::bind(&WebUI::appConfig, this));
    server->on("/configSave", std::bind(&WebUI::appConfigSave, this));
    server->on("/configSavePermanent", std::bind(&WebUI::appConfigSavePermanent, this)); 
    //server->on("/config", HTTP_GET, appConfig);
    //server->on("/configSave", HTTP_GET, appConfigSave);
    //server->on("/configSavePermanent", HTTP_GET, appConfigSavePermanent);

    server->begin();
  };
  void loop(void) {
    server->handleClient();
  };
  void appConfig();
  void appConfigSave();
  void appConfigSavePermanent();
private:
  Debug& debug;
  Config& config;
  NTPClient& timeClient;
  ClockClient& fastclock;
  SevenSegmentClock& sevenSegmentClock;
  ESP8266WebServer *server;
};

#endif
