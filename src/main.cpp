#include <Arduino.h>
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <NTPClient.h>        //https://github.com/esp8266/Arduino
#include <WiFiUdp.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <ArduinoJson.h>
#include <SPI.h>
#include "DjDebug.h"
#include "DjConfig.h"
#include "SevenSegmentClock.h"
#include "DjDebug.h"
#include "ClockClient.h"

// NTP
WiFiUDP ntpUDP;
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 60000);

#define MODE_DEMO "Demo"
#define MODE_REALCLOCK "Realclock"
#define MODE_FASTCLOCK "Fastclock"

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
ESP8266WebServer *server;

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

const char _HEAD[] PROGMEM              = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char _STYLE[] PROGMEM             = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} input.r{width:20%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char _SCRIPT[] PROGMEM            = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char _HEAD_END[] PROGMEM          = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char _PORTAL_OPTIONS[] PROGMEM    = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
const char _ITEM[] PROGMEM              = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char _FORM_START[] PROGMEM        = "<form method='get' action='configSave'>";
const char _FORM_CLOCKNAME[] PROGMEM    = "<label for='n'>Fastclock name</label><input id='n' name='n' length=32 placeholder='clock name' value='{n}'><br/>";
const char _FORM_CLOCKSFOUND_START[] PROGMEM = "Fastclocks seen:<br/><ul>";
const char _FORM_CLOCKSFOUND_ITEM[] PROGMEM = "<li>{fc}</li>";
const char _FORM_CLOCKSFOUND_END[] PROGMEM = "</ul><br/>";
const char _FORM_CLOCKMODE_HEADLINE[] PROGMEM = "<br/>Clock mode:<br/>";
const char _FORM_CLOCKMODE_DEMO[] PROGMEM = "<input class='r' id='md' name='m' type='radio' value='demo' {check}><label for='md'>Demo</label><br/>";
const char _FORM_CLOCKMODE_REAL[] PROGMEM = "<input class='r' id='mr' name='m' type='radio' value='real' {check}><label for='md'>Real Clock</label><br/>";
const char _FORM_CLOCKMODE_FAST[] PROGMEM = "<input class='r' id='mf' name='m' type='radio' value='fast' {check}><label for='md'>Fast Clock</label><br/>";
const char _FORM_UTC_OFFSET[] PROGMEM   = "<label for='utc'>UTC offset (minutes)</label><input id='utc' name='utc' length=4 placeholder='120'><br/>";
const char _FORM_PARAM[] PROGMEM        = "<br/><input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
const char _FORM_COLOR_HEADLINE[] PROGMEM = "<br/>Display color:<br/>";
const char _FORM_COLOR_template[] PROGMEM = "<input class='r' id='{cid}' name='c' type='radio' value='{cname}' {check}><label for='{cid}'>{cname}</label><br/>";
const char _FORM_BRIGHTNESS[] PROGMEM   = "<label for='b'>Brightness:</label><input id='b' name='b' type='range' min='10' max='255' value='{bright}'><br/>";
const char _FORM_FASTCLOCK_INFO[] PROGMEM = "<div>Number of fastclocks found: {nfc}</div><br/>";
const char _FORM_END[] PROGMEM          = "<br/><button type='submit'>apply</button></form><br/>";
const char _SAVE_PERM_BUTTON[] PROGMEM  = "<br/><form action=\"/configSavePermanent\" method=\"get\"><button>Save permanently</button></form><br/>";
const char _CONFIG_BUTTON[] PROGMEM     = "<br/><form action=\"/config\" method=\"get\"><button>Configure</button></form><br/>";
const char _VSPACE[] PROGMEM            = "<br/><br/>";
const char _SAVED[] PROGMEM             = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
const char _END[] PROGMEM               = "</div></body></html>";

void appConfig() {
  String page = FPSTR(_HEAD);
  String input;
  String value;
  int ivalue;

  page.replace("{v}", "7Seg Config");
  page += FPSTR(_SCRIPT);
  page += FPSTR(_STYLE);
  //page += _customHeadElement;
  page += FPSTR(_HEAD_END);
  page += String(F("<h1>"));
  page += appName;
  page += String(F("</h1>"));
  page += String(F("<h3>Clock Options</h3>"));
  //page += FPSTR(_PORTAL_OPTIONS);
  page += FPSTR(_FORM_START);
  page += FPSTR(_FORM_CLOCKMODE_HEADLINE);
  input = FPSTR(_FORM_CLOCKMODE_DEMO);
  input.replace("{check}", (config.getString("appMode").equals("Demo")) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_CLOCKMODE_REAL);
  input.replace("{check}", (config.getString("appMode").equals("Realclock")) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_CLOCKMODE_FAST);
  input.replace("{check}", (config.getString("appMode").equals("Fastclock")) ? "checked" : "");
  page += input;
  page += FPSTR(_VSPACE);
  page += FPSTR(_FORM_UTC_OFFSET);
  page += FPSTR(_VSPACE);
  input = FPSTR(_FORM_CLOCKNAME);
  value = config.getString("listenToClock");
  input.replace("{n}", value);
  page += input;
  page += FPSTR(_FORM_CLOCKSFOUND_START);
  String *knownClocks = fastclock.getKnownClocks();
  for (int i=0; i<fastclock.getNumberOfKnownClocks(); ++i) {
    input = FPSTR(_FORM_CLOCKSFOUND_ITEM);
    input.replace("{fc}", knownClocks[i]);
    page += input;
  }
  page += FPSTR(_FORM_CLOCKSFOUND_END);

  page += FPSTR(_VSPACE);
  page += FPSTR(_FORM_COLOR_HEADLINE);
  for (int i=0; i<sevenSegmentClock.getNumberSupportedColors(); ++i) {
    input = FPSTR(_FORM_COLOR_template);
    input.replace("{cid}", sevenSegmentClock.getColorName(i));
    input.replace("{cname}", sevenSegmentClock.getColorName(i));
    input.replace("{check}", (config.getString("clockColor").equals(sevenSegmentClock.getColorName(i))) ? "checked" : "");
    page += input;
  }

  page += FPSTR(_VSPACE);
  input = FPSTR(_FORM_BRIGHTNESS);
  value = String(sevenSegmentClock.getBrightness());
  input.replace("{bright}", value);
  page += input;

  page += FPSTR(_VSPACE);
  input = FPSTR(_FORM_FASTCLOCK_INFO);
  ivalue = fastclock.getNumberOfKnownClocks();
  value = String(ivalue);
  //value = String("unknown");
  input.replace("{nfc}", value);
  page += input;

  page += FPSTR(_FORM_END);
  page += FPSTR(_END);

  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);
}

void setRealClockTimeOffset(int offsetInMinutes) {
  timeClient.setTimeOffset(offsetInMinutes * 60);
}

void appConfigSave() {
  String page = FPSTR(_HEAD);

  page.replace("{v}", "7Seg Config");
  page += FPSTR(_SCRIPT);
  page += FPSTR(_STYLE);
  page += FPSTR(_HEAD_END);
  page += String(F("<h1>"));
  page += appName;
  page += String(F("</h1>"));

  debug.out(server->args(), DEBUG_MED_INFO); debug.outln(" arguments", DEBUG_MED_INFO);
  for (int i=0; i<server->args(); ++i) {
    debug.out(server->argName(i), DEBUG_MAX_INFO);
    debug.out(": ", DEBUG_MAX_INFO);
    debug.outln(server->arg(i), DEBUG_MAX_INFO);
  }
  if (server->hasArg("n")) {
    String clockName = server->arg("n");
    config.setString("listenToClock", clockName);
    fastclock.setListenToClock(clockName);
    page += F("<div>Set fastclock to listen to clock with name ");
    page += clockName;
    page += F(".</div>");
  }
  if (server->hasArg("b")) {
    int brightness = server->arg("b").toInt();
    sevenSegmentClock.setBrightness(brightness);
    config.setInt("brightness", brightness);
    page += F("<div>Set brightness to ");
    page += server->arg("b");
    page += F(".</div>");
  }
  if (server->hasArg("c")) {
    String colorName = server->arg("c");
    SevenSegmentClock::Color colorHandle = sevenSegmentClock.getColorHandleByName(server->arg("c"));
    sevenSegmentClock.setColor(colorHandle);
    config.setString("clockColor", colorName);
    page += F("<div>Set color to ");
    page += server->arg("c");
    page += F(".</div>");
  }
  if (server->hasArg("m")) {
    debug.out("setting clock mode to ", DEBUG_MAX_INFO); debug.outln(server->arg("m"), DEBUG_MAX_INFO);
    page += F("<div>Set clock mode to ");
    page += server->arg("m");
    page += F(".</div>");
    if (server->arg("m").equals("real")) config.setString("appMode", MODE_REALCLOCK);
    else if (server->arg("m").equals("fast")) config.setString("appMode", MODE_FASTCLOCK);
    else if (server->arg("m").equals("demo")) config.setString("appMode", MODE_DEMO);
    else {
      debug.outln("ERROR: Unknown application mode, going into demo mode", DEBUG_ERROR);
      config.setString("appMode", MODE_DEMO);
      page += F("<div>ERROR: Unknown clockmode, using default: demo.</div>");
    }
  }
  if (server->hasArg("utc")) {
    page += F("<div>Set real clock offset to ");
    int timeOffset;
    if (server->arg("utc").equals("")) {
      page += "120";
      timeOffset = 120;
    } else {
      page += server->arg("utc");
      timeOffset = server->arg("utc").toInt();
    }
    config.setInt("utcTimeOffsetMinutes", timeOffset);
    setRealClockTimeOffset(timeOffset);
    page += F(" minutes.</div>");
  }
  page += String(F("<div>Configuration updated.</div>"));
  page += FPSTR(_CONFIG_BUTTON);
  page += FPSTR(_SAVE_PERM_BUTTON);
  page += FPSTR(_END);
  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);
}

void appConfigSavePermanent() {
  String page = FPSTR(_HEAD);

  page.replace("{v}", "7Seg Config");
  page += FPSTR(_SCRIPT);
  page += FPSTR(_STYLE);
  page += FPSTR(_HEAD_END);
  page += String(F("<h1>"));
  page += appName;
  page += String(F("</h1>"));

  debug.outln("Writing configs to save them permanently", DEBUG_MAX_INFO);
  config.writeAllConfigs();
  page += String(F("<div>Configuration permanently saved.</div>"));
  page += FPSTR(_CONFIG_BUTTON);
  page += FPSTR(_END);
  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);
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

  setRealClockTimeOffset(config.getInt("utcTimeOffsetMinutes"));

  debug.outln(F("Starting fastclock ..."), DEBUG_MAX_INFO);
  fastclock.begin();

  debug.outln(F("Starting 7-segment clock display ..."), DEBUG_MAX_INFO);
  sevenSegmentClock.begin();
  sevenSegmentClock.setBrightness(config.getInt("brightness"));
  sevenSegmentClock.setColor(sevenSegmentClock.getColorHandleByName(config.getString("clockColor")));

  // setting up web server for clock configuration
  server = new ESP8266WebServer(80);
  server->on("/config", HTTP_GET, appConfig);
  server->on("/configSave", HTTP_GET, appConfigSave);
  server->on("/configSavePermanent", HTTP_GET, appConfigSavePermanent);

  server->begin();
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
    sevenSegmentClock.displayTime(timeClient.getHours(), timeClient.getMinutes());
  } else if (config.getString("appMode").equals(MODE_FASTCLOCK)) {
    sevenSegmentClock.displayTime(fastclock.getClockHours(), fastclock.getClockMinutes());
  } else { debug.outln("ERROR: Unknown appMode found.", DEBUG_ERROR); }

  sevenSegmentClock.displayUpdate();
  server->handleClient();
}
